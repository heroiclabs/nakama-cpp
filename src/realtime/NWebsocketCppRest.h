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
#include "cpprest/ws_client.h"
#include <list>
#include <mutex>

namespace Nakama {

    /**
     * Websocket transport using C++ REST SDK (https://github.com/microsoft/cpprestsdk)
     */
    class NWebsocketCppRest : public NRtTransportInterface
    {
    public:
        NWebsocketCppRest();
        ~NWebsocketCppRest();

        void setActivityTimeout(uint16_t timeout) override;
        uint16_t getActivityTimeout() const override;

        void tick() override;

        void connect(const std::string& url, NRtTransportType type) override;

        void disconnect() override;

        bool send(const NBytes& data) override;

    protected:
        void onOpened();
        void onClosed(web::websockets::client::websocket_close_status close_status,
            const utility::string_t& reason,
            const std::error_code& error);
        void onSocketMessage(const web::websockets::client::websocket_incoming_message& msg);

        void addErrorEvent(std::string&& err);

        void disconnect(web::websockets::client::websocket_close_status status, const std::string& reason);

    protected:
        using WsClient = web::websockets::client::websocket_callback_client;
        std::unique_ptr<WsClient> _wsClient;
        NRtTransportType _type = NRtTransportType::Binary;
        bool _disconnectInitiated = false;
        std::mutex _mutex;
        std::unique_ptr<NRtClientDisconnectInfo> _disconnectEvent;
        std::list<std::string> _errorEvents;
        std::list<NBytes> _messageEvents;
        bool _connectedEvent = false;
        bool _connected = false;
        uint32_t _activityTimeoutSec;
        std::atomic<uint64_t> _lastReceivedMessageTimeMs;
    };

}
