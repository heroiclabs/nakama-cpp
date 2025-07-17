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

#pragma once

#include <httpClient/httpClient.h>
#include <memory>
#include <nakama-cpp/NPlatformParams.h>
#include <nakama-cpp/realtime/NRtTransportInterface.h>

namespace Nakama {
class NWebsocketLibHC final : public NRtTransportInterface {
public:
  static std::unique_ptr<NWebsocketLibHC> New(const NPlatformParameters& platformParams);
  ~NWebsocketLibHC() noexcept override;

  void setActivityTimeout(uint32_t timeoutMs) override;
  uint32_t getActivityTimeout() const override;
  void tick() override;

  void connect(const std::string& url, NRtTransportType type) override;

  void disconnect() override;

  bool send(const NBytes& data) override;
  bool isConnecting() const override { return false; };

private:
  explicit NWebsocketLibHC(XTaskQueueHandle q);
  //        void submit_cb(const NHttpResponseCallback &cb, int statusCode, std::string body, std::string err = "")
  //        noexcept; HRESULT NWebsocketLibHC::prep_hc_call(const NHttpRequest& req, std::unique_ptr<HC_CALL,
  //        decltype(&HCHttpCallCloseHandle)>& call);
  //
  std::unique_ptr<std::remove_pointer<XTaskQueueHandle>::type, decltype(&XTaskQueueCloseHandle)> m_queue;
  std::unique_ptr<std::remove_pointer<HCWebsocketHandle>::type, decltype(&HCWebSocketCloseHandle)> m_ws;

  bool m_is_binary;
  uint32_t _activityTimeoutMs = 0;
  uint64_t _lastReceivedMessageTimeMs;

  static void __stdcall ws_on_text_msg(HCWebsocketHandle ws, const char* str, void* self);
  static void __stdcall ws_on_binary_msg(HCWebsocketHandle ws, const uint8_t* bytes, uint32_t size, void* self);
  static void __stdcall ws_on_close(HCWebsocketHandle ws, HCWebSocketCloseStatus status, void* self);
  void on_msg(HCWebsocketHandle ws, std::string&& s);
};
} // namespace Nakama