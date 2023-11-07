/*
 * Copyright 2021 The Nakama Authors
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

#include <string>
#include <memory>
#include <wslay/wslay.h>
#include "nakama-cpp/realtime/NRtTransportInterface.h"
#include <nakama-cpp/realtime/wslay/WslayIOInterface.h>

namespace Nakama {

enum class State {
    RemoteDisconnect,
    Disconnected,
    Connecting,
    Handshake_Sending,
    Handshake_Receiving,
    Connected
};
class NWebsocketWslay : public NRtTransportInterface
{
public:
    NWebsocketWslay(std::unique_ptr<WslayIOInterface> io);
    ~NWebsocketWslay() override;

    void setActivityTimeout(uint32_t timeout) override;
    uint32_t getActivityTimeout() const override;

    void tick() override;
    void connect(const std::string& url, NRtTransportType type) override;
    void disconnect() override;
    bool send(const NBytes& data) override;

protected:
    bool isConnecting() override;

private:
    static ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t* data, size_t len, int flags, void* user_data);
    static ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t* data, size_t len, int flags, void* user_data);
    static void on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg* arg, void* user_data);

    NetIOAsyncResult http_handshake_init();
    NetIOAsyncResult http_handshake_send();
    NetIOAsyncResult http_handshake_receive();
    void disconnect(bool remote, opt::optional<uint16_t> code);

    std::unique_ptr<WslayIOInterface> _io;
    struct wslay_event_callbacks _callbacks;
    std::unique_ptr<std::remove_pointer<wslay_event_context_ptr>::type, decltype(&wslay_event_context_free)> _ctx;
    uint32_t _timeout = 0;
    uint8_t _opcode = 0xFF; // invalid opcode by default

    URLParts _url;
    std::string _client_key;
    State _state = State::Disconnected;

    //Http send state
    std::string _buf;
    std::string::iterator _buf_iter;
};

}