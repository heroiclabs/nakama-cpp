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
    _ixWebSocketPoll.bindWebsocket(&_ixWebSocket);

    // Setup a callback to be fired when a message or an event (open, close, error) is received
    _ixWebSocketPoll.setOnMessageCallback(
        bind(
            &NIXWebsocket::onSocketMessage,
            this,
            placeholders::_1,
            placeholders::_2,
            placeholders::_3,
            placeholders::_4,
            placeholders::_5,
            placeholders::_6
        )
    );

    _ixWebSocket.setHeartBeatPeriod(30);
    _ixWebSocket.disableAutomaticReconnection();
}

NIXWebsocket::~NIXWebsocket()
{
    disconnect();
    _ixWebSocketPoll.bindWebsocket(nullptr);
}

void NIXWebsocket::tick()
{
    _ixWebSocketPoll.poll();
}

void NIXWebsocket::connect(const string& url, NRtTransportType type)
{
    _type = type;

    NLOG_DEBUG("...");
    _ixWebSocket.setUrl(url);
    _ixWebSocket.start();
}

void NIXWebsocket::onSocketMessage(
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
        if (closeInfo.remote)
        {
            NLOG(NLogLevel::Debug, "disconnected. code: %d", closeInfo.code);
        }
        else
        {
            NLOG_DEBUG("closed");
        }

        NRtTransportInterface::onDisconnected();
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
    }
}

bool NIXWebsocket::send(const NBytes & data)
{
    NLOG(NLogLevel::Debug, "sending %d bytes...", data.size());

    ix::WebSocketSendInfo info;

    if (_type == NRtTransportType::Binary)
    {
        info = _ixWebSocket.send(data);
    }
    else
    {
        info = _ixWebSocket.sendText(data);
    }

    if (!info.success)
    {
        NLOG_ERROR("send failed");
    }

    return info.success;
}

void NIXWebsocket::disconnect()
{
    NLOG_DEBUG("...");

    _ixWebSocket.stop();
}

} // Nakama

#endif // BUILD_IXWEBSOCKET
