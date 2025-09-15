/*
 * Copyright 2022 The Nakama Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "NWebsocketLibHC.h"
#include <chrono>
#include <httpClient/httpClient.h>
#include <mutex>
#include <nakama-cpp/log/NLogger.h>

namespace Nakama {

// libhttpclient has a bug where it can call callback even after we closed websocket, which
// leads to hard to reproduce and debug use-after-free errors  (https://github.com/microsoft/libHttpClient/issues/698)
// Workaround is to track all allocated instances in global registry and check for existence in the websocket callback
static struct {
  void pop(NWebsocketLibHC* key) {
    std::unique_lock lock(_mutex);
    auto it = std::find(_instances.begin(), _instances.end(), key);
    if (it != std::end(_instances)) {
      _instances.erase(it);
    }
  }

  void push(NWebsocketLibHC* key) {
    std::unique_lock lock(_mutex);
    auto it = std::find(_instances.begin(), _instances.end(), key);
    if (it == std::end(_instances)) {
      _instances.push_back(key);
    }
  }

  // returns locked unique_lock if key is found, unlocked otherwise
  std::unique_lock<std::mutex> lockKey(NWebsocketLibHC* key) {
    std::unique_lock lock(_mutex);
    auto it = std::find(_instances.begin(), _instances.end(), key);
    if (it == std::end(_instances)) {
      lock.unlock();
    }
    return lock;
  }

private:
  std::mutex _mutex;
  std::vector<NWebsocketLibHC*> _instances;
} g_registry;

HC_DEFINE_TRACE_AREA(wsTransportLibHC, HCTraceLevel::Verbose);

// We do not call HCCleanup() on purpose to prevent indefinite hang.
// Because of that, lets not re-init every instantiation , even though it is safe, it still
// piling up reference count. It is better to have exactly one HCInitialize unpaired
// with HCCleanup, than unspecified number of them.
static bool g_libhc_initialized = false;

static NTimestamp getUnixTimestampMs() {
  using namespace std::chrono;
  auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

  return ms.count();
}

// Converts our loglevel to HCTracelevel
static HCTraceLevel HCLevelForNLevel(NLogLevel lvl) {
  switch (lvl) {
    case NLogLevel::Debug:
      return HCTraceLevel::Verbose;
    case NLogLevel::Info:
      return HCTraceLevel::Information;
    case NLogLevel::Warn:
      return HCTraceLevel::Warning;
    case NLogLevel::Error:
      return HCTraceLevel::Error;
    case NLogLevel::Fatal:
      return HCTraceLevel::Error;
  }

  return HCTraceLevel::Off;
}

static void configureNLogger() {
  static bool configured = false;
  auto l = NLogger::getSink();
  if (!l) {
    return;
  }

  if (configured)
    return;
  configured = true;

  HCSettingsSetTraceLevel(HCLevelForNLevel(l->getLevel()));
  HCTraceSetClientCallback(
      [](const char* areaName, HCTraceLevel level, uint64_t /*threadId*/, uint64_t /*timestamp*/, const char* message) {
        switch (level) {
          case HCTraceLevel::Verbose:
            return NLogger::Debug(message, areaName);
          case HCTraceLevel::Information:
            return NLogger::Info(message, areaName);
          case HCTraceLevel::Important:
          case HCTraceLevel::Warning:
            return NLogger::Warn(message, areaName);
          case HCTraceLevel::Error:
            return NLogger::Error(message, areaName);
          case HCTraceLevel::Off:
            return;
        }
      });
}

std::unique_ptr<NWebsocketLibHC> NWebsocketLibHC::New(const NPlatformParameters& platformParams) {
  std::unique_ptr<NWebsocketLibHC> obj = nullptr;

  configureNLogger();

#ifdef __ANDROID__
  HCInitArgs initArgs{.javaVM = platformParams.javaVM, .applicationContext = platformParams.applicationContext};
  HCInitArgs* initArgsParam = &initArgs;
#else
  (void)platformParams;
  HCInitArgs* initArgsParam = nullptr;
#endif
  if (!g_libhc_initialized) {
    HRESULT hr = HCInitialize(initArgsParam);
    if (FAILED(hr)) {
      HC_TRACE_ERROR_HR(wsTransportLibHC, hr, "HCInitialize failed");
      return nullptr;
    }
    g_libhc_initialized = true;
  }
  XTaskQueueHandle q = nullptr;
  HRESULT hr = XTaskQueueCreate(
      XTaskQueueDispatchMode::ThreadPool,
      XTaskQueueDispatchMode::Manual, // callbacks from ticks
      &q);

  if (FAILED(hr)) {
    HC_TRACE_ERROR(wsTransportLibHC, "Error XTaskQueueCreate(): hr=%d", hr);
    return nullptr;
  }
  return std::unique_ptr<NWebsocketLibHC>(new NWebsocketLibHC(q));
}

NWebsocketLibHC::NWebsocketLibHC(XTaskQueueHandle q)
    : m_queue(q, &XTaskQueueCloseHandle), m_ws(nullptr, &HCWebSocketCloseHandle), m_is_binary(false),
      _activityTimeoutMs(0), _lastReceivedMessageTimeMs(0) {
  g_registry.push(this);
}

NWebsocketLibHC::~NWebsocketLibHC() noexcept {
  disconnect();

  g_registry.pop(this);

  // We can proceed with destroying this instance if even in the presence
  // of outstanding connections, like those which hang, because closing
  // HCWebsocketHandle in m_ws seem to prevent callbacks passed to
  // `HCWebSocketCreate` from firing even for hanging connections left
  // "live" in the guts of Libhttpclient.
  m_ws.reset(nullptr);
  m_queue.reset(nullptr);

  // Don't cleanup on purpose, because of https://github.com/microsoft/libHttpClient/issues/696
  // HCCleanup();
  HC_TRACE_INFORMATION(wsTransportLibHC, "Destroying instance %p", this);
}

void NWebsocketLibHC::connect(const std::string& url, NRtTransportType type) {
  {
    HCWebsocketHandle ws = nullptr;
    auto hr = HCWebSocketCreate(
        &ws, reinterpret_cast<HCWebSocketMessageFunction>(&ws_on_text_msg),
        reinterpret_cast<HCWebSocketBinaryMessageFunction>(&ws_on_binary_msg),
        reinterpret_cast<HCWebSocketCloseEventFunction>(&ws_on_close), this);
    if (FAILED(hr)) {
      HC_TRACE_ERROR(wsTransportLibHC, "Error HCWebSocketCreate(): hr=%d", hr);
      this->fireOnError("Websocket connection error");
      return;
    }
#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_GDK
    HCWebSocketSetMaxReceiveBufferSize(ws, 1024 * 1024);
#endif
    m_ws.reset(ws);
  }

  auto* asyncBlock = new XAsyncBlock{};
  asyncBlock->context = this;
  asyncBlock->queue = m_queue.get(); // unique_ptr m_queue will outlive asyncBlock, so thats fine
  asyncBlock->callback = [](XAsyncBlock* ab) {
    const std::unique_ptr<XAsyncBlock> asyncBlock(ab);
    auto self = static_cast<NWebsocketLibHC*>(asyncBlock->context);

    WebSocketCompletionResult result{};
    HRESULT hr = HCGetWebSocketConnectResult(asyncBlock.get(), &result);
    if (SUCCEEDED(hr)) {
      // Check that m_ws is still present, meaning no disconnects have been called
      // actual socket handle comparison is commented due to https://github.com/microsoft/libHttpClient/issues/697
      // because of that, we won't be able to process connect - disconnect - connect sequence correctly
      if (self->m_ws /*&& self->m_ws.get() == result.websocket*/) {
        if (SUCCEEDED(result.errorCode)) {
          self->_lastReceivedMessageTimeMs = getUnixTimestampMs();
          self->fireOnConnected();
        } else {
          self->fireOnError("Websocket connection error");
        }
      } else {
        // While we were connecting, user called disconnect() and closed websocket, making
        // this connection notification useless, so we ignore it and don't trigger any callbacks
        HC_TRACE_WARNING(
            wsTransportLibHC,
            "User requested disconnect, while connection was in progress. Ignoring connection result.");
      }
    } else {
      self->fireOnError("Websocket connection error: HCGetWebsocketConnectResult failed");
    }
  };
  m_is_binary = (NRtTransportType::Binary == type);
  HCWebSocketConnectAsync(url.c_str(), "", m_ws.get(), asyncBlock);
}

void NWebsocketLibHC::disconnect() {
  if (m_ws) {
    HCWebSocketDisconnect(m_ws.get());

    // Prevent any late responses from being processed
    m_ws.reset(nullptr);

    _connected = false; // white lie, we didn't receive close confirmation from server yet
  }
}

void NWebsocketLibHC::setActivityTimeout(uint32_t timeoutMs) { _activityTimeoutMs = timeoutMs; }

uint32_t NWebsocketLibHC::getActivityTimeout() const { return _activityTimeoutMs; }

void NWebsocketLibHC::tick() {
  while (XTaskQueueDispatch(m_queue.get(), XTaskQueuePort::Completion, 0)) {
  }

  if (isConnected() && _activityTimeoutMs > 0 &&
      getUnixTimestampMs() - _lastReceivedMessageTimeMs >= _activityTimeoutMs) {
    disconnect();
  }
}

bool NWebsocketLibHC::send(const NBytes& data) {
  if (!isConnected()) {
    return false;
  }

  HC_TRACE_INFORMATION(wsTransportLibHC, "Sending bytes");

  auto* asyncBlock = new XAsyncBlock{};

  asyncBlock->context = this;
  asyncBlock->queue = m_queue.get(); // unique_ptr m_queue will outlive asyncBlock, so thats fine
  asyncBlock->callback = [](XAsyncBlock* ab) {
    std::unique_ptr<XAsyncBlock> asyncBlock(ab);
    WebSocketCompletionResult result;
    HRESULT hr = HCGetWebSocketSendMessageResult(asyncBlock.get(), &result);
    if (FAILED(result.errorCode) || FAILED(hr)) {
      HC_TRACE_ERROR(
          wsTransportLibHC, "Send error: hr=0x%08x ws.errorCode=0x%08x platformErr=0x%08x", hr, result.errorCode,
          result.platformErrorCode);
      static_cast<NWebsocketLibHC*>(asyncBlock->context)->fireOnError("Error sending msg");
    }
  };
  HRESULT hr;
  if (m_is_binary) {
    hr = HCWebSocketSendBinaryMessageAsync(
        m_ws.get(), reinterpret_cast<const uint8_t*>(data.data()), uint32_t(data.size()), asyncBlock);
  } else {
    hr = HCWebSocketSendMessageAsync(m_ws.get(), data.c_str(), asyncBlock);
  }
  if (FAILED(hr)) {
    HC_TRACE_ERROR(wsTransportLibHC, "Send error: hr=0x%08x", hr);
  }
  return SUCCEEDED(hr);
}

void __stdcall NWebsocketLibHC::ws_on_text_msg(HCWebsocketHandle ws, const char* s, void* context) {
  HC_TRACE_INFORMATION(wsTransportLibHC, "ws_on_text_msg ws=%p", ws);
  auto self = static_cast<NWebsocketLibHC*>(context);
  if (auto lock = g_registry.lockKey(self)) {
    self->on_msg(ws, std::string(s));
  }
}

void __stdcall NWebsocketLibHC::ws_on_binary_msg(
    HCWebsocketHandle ws,
    const uint8_t* bytes,
    uint32_t size,
    void* context) {
  HC_TRACE_INFORMATION(wsTransportLibHC, "ws_on_binary_msg ws=%p", ws);
  auto self = static_cast<NWebsocketLibHC*>(context);
  if (auto lock = g_registry.lockKey(self)) {
    self->on_msg(ws, std::string(reinterpret_cast<const char*>(bytes), size));
  }
}

// Callback is called on an unspecified thread when socket is closed.
// Arrange fireOnDisconnected() to be called from the tick()
void __stdcall NWebsocketLibHC::ws_on_close(HCWebsocketHandle ws, HCWebSocketCloseStatus status, void* context) {
  HC_TRACE_INFORMATION(wsTransportLibHC, "ws_on_close ws=%p", ws);
  auto self = static_cast<NWebsocketLibHC*>(context);

  if (auto lock = g_registry.lockKey(self)) {
    using ctx_t = std::tuple<NWebsocketLibHC*, uint16_t, HCWebsocketHandle>;
    auto ctx = new ctx_t(self, static_cast<uint16_t>(status), ws);
    HC_TRACE_INFORMATION(wsTransportLibHC, "ws_on_close: accessing instance %p", self);

    XTaskQueueSubmitCallback(self->m_queue.get(), XTaskQueuePort::Completion, ctx, [](void* context, bool) {
      std::unique_ptr<ctx_t> ctx(static_cast<ctx_t*>(context));
      auto self = std::get<0>(*ctx);
      auto code = std::get<1>(*ctx);
      auto ws = std::get<2>(*ctx);

      // m_ws is closed on disconnects, but already scheduled notifications are still processed.
      // Here we check that notification is for the "active" socket, not a previously closed one to avoid race
      if (self->m_ws && self->m_ws.get() == ws) {
        self->m_ws.reset(nullptr);
        NRtClientDisconnectInfo info;
        info.code = code;
        info.remote = true;
        self->fireOnDisconnected(info);
      } else {
        // if we are here, then user initiated disconnect and callback was already fired
        HC_TRACE_INFORMATION(
            wsTransportLibHC, "ws_on_close->tick: ignoring close for use-initiated disconnect on %p", ws);
      }
    });
  } else {
    HC_TRACE_INFORMATION(wsTransportLibHC, "ws_on_close: ignoring close for already deleted instance %p", self);
  }
}

// Called by libhttpClient when message arrives. Schedule fireOnMessage to be called on the
// tick() thread
// NOTE: If message exceeds internal buffer (1MiB), partial message will be passed here with no indication
// that it was truncated.
void NWebsocketLibHC::on_msg(HCWebsocketHandle ws, std::string&& s) {
  using ctx_t = std::tuple<NWebsocketLibHC*, std::string, HCWebsocketHandle>;
  auto ctx = new ctx_t(this, s, ws);

  XTaskQueueSubmitCallback(this->m_queue.get(), XTaskQueuePort::Completion, ctx, [](void* context, bool) {
    std::unique_ptr<ctx_t> ctx(static_cast<ctx_t*>(context));
    auto self = std::get<0>(*ctx);
    auto& buf = std::get<1>(*ctx);
    auto ws = std::get<2>(*ctx);

    // m_ws is closed on disconnects, but already scheduled notifications are still processed.
    // Here we check that notification is for the "active" socket, not a previously closed one to avoid race
    if (self->m_ws && self->m_ws.get() == ws) {
      self->fireOnMessage(buf);
      self->_lastReceivedMessageTimeMs = getUnixTimestampMs();
    }
  });
}
} // namespace Nakama
