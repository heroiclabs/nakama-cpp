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

#include "nakama-cpp/realtime/NDefaultWebsocket.h"
#include "nakama-cpp/log/NLogger.h"

#ifdef BUILD_DEFAULT_WEBSOCKETS

#include "realtime/NDefaultWebsocket.h"

namespace Nakama {

NRtTransportPtr createDefaultWebsocket()
{
    return NRtTransportPtr(new NDefaultWebsocket());
}

NDefaultWebsocket::NDefaultWebsocket()
{
    _wsClient.init_asio();
}

void NDefaultWebsocket::tick()
{
    _wsClient.poll();
}

void NDefaultWebsocket::connect(const std::string & url, NRtTransportType type)
{
    try {
        websocketpp::lib::error_code ec;

        // set logging to only fails
        _wsClient.clear_access_channels(websocketpp::log::alevel::all);
        _wsClient.set_access_channels(websocketpp::log::alevel::fail);
        //_wsClient.set_error_channels(websocketpp::log::elevel::warn);

        WsClient::connection_ptr con = _wsClient.get_connection(url, ec);

        con->set_open_handler([this](websocketpp::connection_hdl hdl)
        {
            NLOG_DEBUG("socket connected");
            _con_hdl = hdl;
            onConnected();
        });

        con->set_message_handler([this](websocketpp::connection_hdl, WsClient::message_ptr msg)
        {
            NLOG(NLogLevel::Debug, "socket message received %d bytes", msg->get_payload().size());

            NBytes bytes;

            bytes.assign(msg->get_payload().begin(), msg->get_payload().end());

            onMessage(bytes);
        });

        con->set_fail_handler([this](websocketpp::connection_hdl hdl)
        {
            NLOG_ERROR("socket fail");
            onError("fail");
        });

        con->set_close_handler([this](websocketpp::connection_hdl hdl)
        {
            NLOG_DEBUG("socket closed");
            onDisconnected();
        });

        if (type == NRtTransportType::Binary)
            _op_code = websocketpp::frame::opcode::binary;
        else
            _op_code = websocketpp::frame::opcode::text;

        NLOG_DEBUG("socket connecting...");
        _wsClient.connect(con);
    }
    catch (websocketpp::exception const & e)
    {
        onError("connect failed: " + std::string(e.what()));
        NLOG_ERROR(e.what());
    }
}

void NDefaultWebsocket::disconnect()
{
    NLOG_DEBUG("...");

    _wsClient.close(_con_hdl, websocketpp::close::status::normal, "");
}

void NDefaultWebsocket::send(const NBytes & data)
{
    NLOG(NLogLevel::Debug, "sending %d bytes...", data.size());

    _wsClient.send(_con_hdl, data.data(), data.size(), _op_code);
}

} // namespace Nakama

#else

namespace Nakama {

NRtTransportPtr createDefaultWebsocket()
{
    NLOG_ERROR("Default websocket is not available for this platform.");
    return nullptr;
}

} // namespace Nakama

#endif // BUILD_DEFAULT_WEBSOCKETS
