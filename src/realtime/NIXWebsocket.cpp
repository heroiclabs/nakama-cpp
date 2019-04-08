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

#ifdef BUILD_IXWEBSOCKET

#include "NIXWebsocket.h"
#include "nakama-cpp/log/NLogger.h"

#undef NMODULE_NAME
#define NMODULE_NAME "NIXWebsocket"

namespace Nakama {

using namespace std;

NIXWebsocket::NIXWebsocket()
{
}

NIXWebsocket::~NIXWebsocket()
{
    disconnect();
}

void NIXWebsocket::connect(const string& url, NRtTransportType type)
{
    _type = type;
    
    if (_type == NRtTransportType::Text)
    {
        NLOG_WARN("Text type is not supported");
    }
    
    _ixWebSocket.setUrl(url);

    // Setup a callback to be fired when a message or an event (open, close, error) is received
    _ixWebSocket.setOnMessageCallback(
        bind(
            &NIXWebsocket::setOnMessageCallback,
            this,
            placeholders::_1,
            placeholders::_2,
            placeholders::_3,
            placeholders::_4,
            placeholders::_5,
            placeholders::_6
        )
    );

    _ixWebSocket.start();
}

void NIXWebsocket::setOnMessageCallback(
    ix::WebSocketMessageType messageType,
    const string& str,
    size_t wireSize,
    const ix::WebSocketErrorInfo& error,
    const ix::WebSocketOpenInfo& openInfo,
    const ix::WebSocketCloseInfo& closeInfo)
{
    if (messageType == ix::WebSocket_MessageType_Message)
    {
        NRtTransportInterface::onMessage(str);
    }
    else if (messageType == ix::WebSocket_MessageType_Open)
    {
        NLOG_DEBUG("connected");
        NRtTransportInterface::onConnected();
    }
    else if (messageType == ix::WebSocket_MessageType_Close)
    {
        NLOG_DEBUG("closed");
        // std::cout << "close with code: " << closeInfo.code << " reason: " << closeInfo.reason << std::endl;
        NRtTransportInterface::onDisconnected();

        // prevent auto reconnect
        //_ixWebSocket.disableAutomaticReconnection();
    }
    else if (messageType == ix::WebSocket_MessageType_Error)
    {
        NLOG(NLogLevel::Error, "error: %s", error.reason.c_str());
        NRtTransportInterface::onError(error.reason);
    }
    else if (messageType == ix::WebSocket_MessageType_Ping)
    {
    }
    else if (messageType == ix::WebSocket_MessageType_Pong)
    {
    }
    else if (messageType == ix::WebSocket_MessageType_Fragment)
    {
        NLOG_DEBUG("fragment");
    }
}

void NIXWebsocket::send(const NBytes & data)
{
    //if (_type == NRtTransportType::Binary)
    {
        _ixWebSocket.send(data);
    }
    //else
    //{
    //    _ixWebSocket.sendText(data);
    //}
}

void NIXWebsocket::disconnect()
{
    _ixWebSocket.stop();
}

} // Nakama

#endif // BUILD_IXWEBSOCKET
