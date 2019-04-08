/*
 * Copyright 2019 The Nakama Authors
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

#include "nakama-cpp/realtime/NRtTransportInterface.h"

#define _WEBSOCKETPP_CPP11_STL_
#include "websocketpp/config/asio_no_tls_client.hpp"
#include "websocketpp/client.hpp"

namespace Nakama {

    /**
     * Websocketpp transport
     */
    class NWebsocketpp : public NRtTransportInterface
    {
    public:
        NWebsocketpp();

        void tick() override;

        void connect(const std::string& url, NRtTransportType type) override;

        void disconnect() override;

        void send(const NBytes& data) override;

    protected:
        using WsClient = websocketpp::client<websocketpp::config::asio_client>;

        websocketpp::frame::opcode::value _op_code = websocketpp::frame::opcode::binary;
        WsClient _wsClient;
        websocketpp::connection_hdl _con_hdl;
    };

}
