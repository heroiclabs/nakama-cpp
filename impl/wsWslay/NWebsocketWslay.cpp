#include <cassert>
#include <chrono>
#include <cstring>
#include <memory>
#include <random>
#include <string>

#include "NWebsocketWslay.h"
#include "StrUtil.h"
#include "sha1.h"
#include <nakama-cpp/log/NLogger.h>
#include <wslay/wslay.h>

namespace Nakama {

void NWebsocketWslay::on_msg_recv_callback(
    wslay_event_context_ptr /*ctx*/,
    const struct wslay_event_on_msg_recv_arg* arg,
    void* user_data) {
  auto ws = static_cast<NWebsocketWslay*>(user_data);

  NLOG(NLogLevel::Debug, "Received WS message. Opcode: %d Length: %d", arg->opcode, arg->msg_length);

  if (wslay_is_ctrl_frame(arg->opcode)) {
    if (arg->opcode == WSLAY_CONNECTION_CLOSE) {
      NLOG(NLogLevel::Info, "Remote server closed connection with status code %d", arg->status_code);
      ws->_state.store(State::RemoteDisconnect);
    }
  } else {
    // Copy message data — arg memory is only valid during this callback
    NBytes data(reinterpret_cast<const char*>(arg->msg), arg->msg_length);
    ws->enqueueCallback([ws, data = std::move(data)]() { ws->fireOnMessage(data); });
  }
}

static int genmask_callback(wslay_event_context_ptr /*ctx*/, uint8_t* buf, const size_t len, void* /*user_data*/) {
  static std::mt19937 rng(std::random_device{}());
  for (size_t offset = 0; offset < len; offset += sizeof(decltype(rng)::result_type)) {
    auto rnd = rng();
    std::memcpy(buf + offset, &rnd, std::min(len - offset, sizeof(rnd)));
  }
  return 0;
}

ssize_t
NWebsocketWslay::recv_callback(wslay_event_context_ptr ctx, uint8_t* data, size_t len, int /*flags*/, void* user_data) {
  auto ws = static_cast<NWebsocketWslay*>(user_data);

  if (!ws->_buf.empty()) { // together with handshake response server sent us message, "read" it
    auto n = std::min(len, ws->_buf.size());
    std::memcpy(data, ws->_buf.data(), n);
    ws->_buf.erase(ws->_buf.begin(), std::next(ws->_buf.begin(), n));
    return n;
  }

  int would_block = 0;
  ssize_t ret = ws->_io->recv(data, len, &would_block);
  if (would_block) {
    wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    return -1;
  } else if (ret < 0) {
    wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    return -1;
  }
  return ret;
}

static std::string create_acceptkey(const std::string& clientkey) {
  auto s = SHA1();
  s.update(clientkey);
  s.update("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
  auto digest = s.final();

  std::string result(digest.begin(), digest.end());
  return Nakama::base64Encode(result);
}

ssize_t NWebsocketWslay::send_callback(
    wslay_event_context_ptr ctx,
    const uint8_t* data,
    size_t len,
    int /*flags*/,
    void* user_data) {
  auto ws = static_cast<NWebsocketWslay*>(user_data);
  int would_block = 0;
  ssize_t ret = ws->_io->send(data, len, &would_block);
  if (ret < 0) {
    wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    return -1;
  } else if (would_block) {
    wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    return 0;
  }
  return ret;
}

static std::string get_random16() {
  std::mt19937_64 rng(std::random_device{}());
  uint64_t rnd[2] = {rng(), rng()};
  return {reinterpret_cast<char*>(&rnd[0]), sizeof(rnd)};
}

NetIOAsyncResult NWebsocketWslay::http_handshake_init() {
  _client_key = Nakama::base64Encode(get_random16());

  bool useSsl = _url.scheme == "wss";
  uint16_t default_port = useSsl ? 443 : 80;
  uint16_t port = _url.port.value_or(default_port);

  std::ostringstream reqb;
  reqb << "GET /" << _url.pathAndArgs << " HTTP/1.1" << "\r\n"
       << "Host: " << _url.host << ":" << port << "\r\n"
       << "Upgrade: websocket\r\n"
       << "Connection: Upgrade\r\n"
       << "Sec-WebSocket-Version: 13\r\n"
       << "Sec-WebSocket-Key: " << _client_key << "\r\n"
       << "\r\n";
  _buf = reqb.str();
  _buf_iter = _buf.begin();
  return NetIOAsyncResult::DONE;
}

NetIOAsyncResult NWebsocketWslay::http_handshake_send() {
  int would_block = 0;
  while (_buf_iter != _buf.end() || would_block != 0) {
    ssize_t sent = this->_io->send(&*_buf_iter, _buf.end() - _buf_iter, &would_block);
    if (sent < 0) {
      return NetIOAsyncResult::ERR;
    }
    std::advance(_buf_iter, sent);
  }

  if (_buf_iter == _buf.end()) {
    _buf.resize(0);
    return NetIOAsyncResult::DONE;
  }

  return NetIOAsyncResult::AGAIN;
}

NetIOAsyncResult NWebsocketWslay::http_handshake_receive() {
  int would_block = 0;
  do {
    char rbuf[1024]{};
    ssize_t read = this->_io->recv(&rbuf[0], sizeof(rbuf), &would_block);
    if (would_block != 0) {
      return NetIOAsyncResult::AGAIN;
    } else if (read < 0) {
      return NetIOAsyncResult::ERR;
    } else if (read == 0) {
      NLOG_ERROR("http_upgrade: disconnected during http handshake");
      return NetIOAsyncResult::ERR;
    }
    _buf.append(&rbuf[0], read);
  } while (_buf.find("\r\n\r\n") == std::string::npos);

  // received full response
  const char* hdr = "Sec-WebSocket-Accept: ";
  auto s = _buf.find(hdr);
  if (s == std::string::npos) {
    NLOG_ERROR("http_upgrade: missing required headers");
    return NetIOAsyncResult::ERR;
  }

  auto accept_key = std::string_view(_buf);
  accept_key.remove_prefix(s + strlen(hdr));
  accept_key.remove_suffix(accept_key.size() - accept_key.find("\r\n", 2));

  if (accept_key != create_acceptkey(_client_key)) {
    NLOG(Nakama::NLogLevel::Error, "Websocket server returned unexpected Sec-WebSocket-Accept key: %s", accept_key);
    return NetIOAsyncResult::ERR;
  }

  auto rnrn_pos = _buf.find("\r\n\r\n");
  // clear handshake response from buffer. if server sent us message immediately after HTTP upgrade, buffer will still
  // contain it.
  _buf.erase(_buf.begin(), std::next(_buf.begin(), rnrn_pos + 4));

  return NetIOAsyncResult::DONE;
}

NWebsocketWslay::NWebsocketWslay(std::unique_ptr<WslayIOInterface> io)
    : _io(std::move(io)),
      _callbacks{recv_callback, send_callback, genmask_callback, nullptr, nullptr, nullptr, on_msg_recv_callback},
      _ctx(nullptr, wslay_event_context_free) {}

NWebsocketWslay::~NWebsocketWslay() {
  _ioRunning.store(false);
  if (_ioThread.joinable())
    _ioThread.join();
  cleanupConnection();
}

void NWebsocketWslay::enqueueCallback(std::function<void()> cb) {
  std::lock_guard<std::mutex> lock(_callbackMutex);
  _callbackQueue.push_back(std::move(cb));
}

void NWebsocketWslay::cleanupConnection() {
  if (_state.load() == State::Connected && _ctx) {
    // Best-effort close frame
    wslay_event_queue_close(_ctx.get(), 0, nullptr, 0);
    wslay_event_send(_ctx.get());
  }

  _io->close();
  _state.store(State::Disconnected);
  _connected = false;
  _ctx.reset(nullptr);

  {
    std::lock_guard<std::mutex> lock(_sendMutex);
    std::queue<NBytes> empty;
    _outgoingQueue.swap(empty);
  }
}

void NWebsocketWslay::connect(const std::string& url, NRtTransportType transportType) {
  assert(!_ctx);
  {
    wslay_event_context_ptr p;
    wslay_event_context_client_init(&p, &this->_callbacks, this);
    _ctx.reset(p);
  }

  if (transportType == NRtTransportType::Binary) {
    _opcode = WSLAY_BINARY_FRAME;
  } else {
    _opcode = WSLAY_TEXT_FRAME;
  }

  auto urlOpt = ParseURL(url);
  if (!urlOpt) {
    enqueueCallback([this]() { fireOnError("Malformed URL"); });
    _ctx.reset(nullptr);
    return;
  }
  _url = urlOpt.value();

  if (this->_io->connect_init(_url) == NetIOAsyncResult::ERR) {
    enqueueCallback([this]() { fireOnError("Failed connect"); });
    _ctx.reset(nullptr);
    return;
  }

  _state.store(State::Connecting);
  _ioRunning.store(true);
  _ioThread = std::thread(&NWebsocketWslay::ioThreadFunc, this);
}

void NWebsocketWslay::disconnect() {
  if (!_ioRunning.load() && _state.load() == State::Disconnected)
    return;

  _ioRunning.store(false);
  if (_ioThread.joinable())
    _ioThread.join();

  // Discard pending callbacks — NRtClient handles its own disconnect notification.
  // This also satisfies the contract: "late messages won't trigger messageCallback"
  {
    std::lock_guard<std::mutex> lock(_callbackMutex);
    _callbackQueue.clear();
  }

  cleanupConnection();
}

uint32_t NWebsocketWslay::getActivityTimeout() const { return this->_timeout; }

bool NWebsocketWslay::send(const NBytes& data) {
  if (_state.load() != State::Connected)
    return false;

  std::lock_guard<std::mutex> lock(_sendMutex);
  _outgoingQueue.push(data);
  return true;
}

void NWebsocketWslay::setActivityTimeout(uint32_t timeout) { this->_timeout = timeout; }

void NWebsocketWslay::tick() {
  std::list<std::function<void()>> callbacks;
  {
    std::lock_guard<std::mutex> lock(_callbackMutex);
    callbacks.swap(_callbackQueue);
  }
  for (auto& cb : callbacks) {
    cb();
  }
}

void NWebsocketWslay::ioThreadFunc() {
  while (_ioRunning.load()) {
    State currentState = _state.load();

    if (currentState == State::Connected) {
      // 1. Drain outgoing queue into wslay
      {
        std::lock_guard<std::mutex> lock(_sendMutex);
        while (!_outgoingQueue.empty()) {
          auto& data = _outgoingQueue.front();
          struct wslay_event_msg msg {
            _opcode, reinterpret_cast<const uint8_t*>(data.data()), data.size()
          };
          int ret = wslay_event_queue_msg(_ctx.get(), &msg);
          if (ret != 0) {
            NLOG(NLogLevel::Error, "[wslay] unable to queue egress message: %d", ret);
          }
          _outgoingQueue.pop();
        }
      }

      // 2. Receive
      int ret = wslay_event_recv(_ctx.get());
      if (ret != 0 && ret != WSLAY_ERR_WOULDBLOCK) {
        NLOG(NLogLevel::Error, "[wslay] unable to receive message from peer: %d", ret);
        _io->close();
        _state.store(State::Disconnected);
        _ctx.reset(nullptr);
        enqueueCallback([this]() {
          NRtClientDisconnectInfo info;
          info.code = NRtClientDisconnectInfo::Code::TRANSPORT_ERROR;
          info.remote = false;
          fireOnDisconnected(info);
        });
        break;
      }

      // 3. Check for remote disconnect (set by on_msg_recv_callback for WSLAY_CONNECTION_CLOSE)
      if (_state.load() == State::RemoteDisconnect) {
        uint16_t code = wslay_event_get_status_code_received(_ctx.get());
        _io->close();
        _state.store(State::Disconnected);
        _ctx.reset(nullptr);
        enqueueCallback([this, code]() {
          NRtClientDisconnectInfo info;
          info.code = code;
          info.remote = true;
          fireOnDisconnected(info);
        });
        break;
      }

      // 4. Send
      ret = wslay_event_send(_ctx.get());
      if (ret != 0) {
        NLOG(NLogLevel::Error, "[wslay] unable to send message to peer: %d", ret);
        _io->close();
        _state.store(State::Disconnected);
        _ctx.reset(nullptr);
        enqueueCallback([this]() {
          NRtClientDisconnectInfo info;
          info.code = NRtClientDisconnectInfo::Code::TRANSPORT_ERROR;
          info.remote = false;
          fireOnDisconnected(info);
        });
        break;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } else {
      // Drive connection state machine: Connecting → Handshake_Sending → Handshake_Receiving → Connected
      NetIOAsyncResult res = NetIOAsyncResult::AGAIN;
      do {
        currentState = _state.load();
        if (currentState == State::Connecting) {
          NLOG_DEBUG("Wslay state: Connecting");
          res = _io->connect_tick();
          if (res == NetIOAsyncResult::DONE) {
            _state.store(State::Handshake_Sending);
            res = http_handshake_init();
          }
        } else if (currentState == State::Handshake_Sending) {
          NLOG_DEBUG("Wslay state: Handshake sending");
          res = http_handshake_send();
          if (res == NetIOAsyncResult::DONE) {
            _state.store(State::Handshake_Receiving);
          }
        } else if (currentState == State::Handshake_Receiving) {
          NLOG_DEBUG("Wslay state: Handshake receiving");
          res = http_handshake_receive();
          if (res == NetIOAsyncResult::DONE) {
            _state.store(State::Connected);
            enqueueCallback([this]() { fireOnConnected(); });
            break;
          }
        } else {
          break;
        }
      } while (res == NetIOAsyncResult::DONE);

      if (res == NetIOAsyncResult::ERR) {
        std::string errMessage;
        currentState = _state.load();
        switch (currentState) {
          case State::Connecting:
            errMessage = "Failed connect";
            break;
          case State::Handshake_Receiving:
          case State::Handshake_Sending:
            errMessage = "Failed HTTP handshake";
            break;
          default:
            break;
        }
        NLOG(NLogLevel::Debug, "Wslay result: ERROR %s", errMessage.c_str());
        _state.store(State::Disconnected);
        _io->close();
        _ctx.reset(nullptr);
        enqueueCallback([this, errMessage]() { fireOnError(errMessage); });
        break;
      }

      if (res == NetIOAsyncResult::AGAIN) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }
}

bool NWebsocketWslay::isConnecting() const {
  State s = _state.load();
  return s == State::Connecting || s == State::Handshake_Receiving || s == State::Handshake_Sending;
}
} // namespace Nakama
