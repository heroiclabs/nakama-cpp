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

#ifdef BUILD_WEBSOCKETPP

#include "NWebsocketpp.h"
#include "nakama-cpp/log/NLogger.h"

#undef NMODULE_NAME
#define NMODULE_NAME "NWebsocketpp"

namespace Nakama {

NWebsocketpp::NWebsocketpp()
{
    _wsClient.init_asio();

#ifdef NAKAMA_SSL_ENABLED
    _wsClient.set_tls_init_handler([this](websocketpp::connection_hdl) {
        return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv1);
    });
#endif
}

void NWebsocketpp::tick()
{
    _wsClient.poll();
}

void NWebsocketpp::connect(const std::string & url, NRtTransportType type)
{
    try {
        websocketpp::lib::error_code ec;

        // set logging to only fails
        _wsClient.clear_access_channels(websocketpp::log::alevel::all);
        _wsClient.set_access_channels(websocketpp::log::alevel::fail);
        //_wsClient.set_error_channels(websocketpp::log::elevel::warn);

        WsClient::connection_ptr con = _wsClient.get_connection(url, ec);

        if (ec)
        {
            onError("initialize connect failed: " + ec.message());
            return;
        }

        con->set_open_handler([this](websocketpp::connection_hdl hdl)
        {
            NLOG_DEBUG("socket connected");
            _con_hdl = hdl;
            onConnected();
        });

        con->set_message_handler([this](websocketpp::connection_hdl, WsClient::message_ptr msg)
        {
            auto& payload = msg->get_payload();

            NLOG(NLogLevel::Debug, "socket message received %d bytes", payload.size());

            //NBytes bytes;

            //bytes.assign(payload.begin(), payload.end());

            //onMessage(bytes);

            onMessage(payload);
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

        NLOG_DEBUG("...");
        (void) _wsClient.connect(con);
    }
    catch (websocketpp::exception const & e)
    {
        NLOG_ERROR(e.what());
        onError("connect failed: " + std::string(e.what()));
    }
}

void NWebsocketpp::disconnect()
{
    NLOG_DEBUG("...");

    _wsClient.close(_con_hdl, websocketpp::close::status::normal, "");
}

bool NWebsocketpp::send(const NBytes & data)
{
    NLOG(NLogLevel::Debug, "sending %d bytes...", data.size());

    websocketpp::lib::error_code ec;

    _wsClient.send(_con_hdl, data.data(), data.size(), _op_code, ec);

    if (ec)
    {
        NLOG(NLogLevel::Error, "error: %d, %s", ec.value(), ec.message().c_str());
    }

    return !ec;
}

} // namespace Nakama

#endif // BUILD_WEBSOCKETPP
