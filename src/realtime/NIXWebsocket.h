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
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketMessageQueue.h"

namespace Nakama {

    class NIXWebsocket : public NRtTransportInterface
    {
    public:
        NIXWebsocket();
        ~NIXWebsocket();
        
        void setPingSettings(const NRtPingSettings& settings) override;
        NRtPingSettings getPingSettings() const override;

        void tick() override;
        void connect(const std::string& url, NRtTransportType type) override;
        void disconnect() override;
        bool send(const NBytes& data) override;

    protected:
        void onSocketMessage(
            ix::WebSocketMessageType messageType,
            const std::string& str,
            size_t wireSize,
            const ix::WebSocketErrorInfo& error,
            const ix::WebSocketOpenInfo& openInfo,
            const ix::WebSocketCloseInfo& closeInfo);
        
    private:
        NRtTransportType _type = NRtTransportType::Binary;
        ix::WebSocketMessageQueue _wsMessageQueue;
        ix::WebSocket _ixWebSocket;
    };

}
