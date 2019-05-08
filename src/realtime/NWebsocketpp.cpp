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
#include "nakama-cpp/StrUtil.h"

#undef NMODULE_NAME
#define NMODULE_NAME "NWebsocketpp"

namespace Nakama {

NWebsocketpp::NWebsocketpp()
{
#ifdef NAKAMA_SSL_ENABLED
    _wssClient.set_tls_init_handler([this](websocketpp::connection_hdl) {
        return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv1);
    });
#endif
}

void NWebsocketpp::tick()
{
#ifdef NAKAMA_SSL_ENABLED
    if (_ssl)
    {
        if (_wssInitialized)
            _wssClient.poll();
    }
    else
#endif
    {
        if (_wsInitialized)
            _wsClient.poll();
    }
}

void NWebsocketpp::connect(const std::string & url, NRtTransportType type)
{
    if (type == NRtTransportType::Binary)
        _op_code = websocketpp::frame::opcode::binary;
    else
        _op_code = websocketpp::frame::opcode::text;

    try {
        websocketpp::lib::error_code ec;

        _ssl = isStringStartsWith(url, "wss://");

        if (_ssl)
        {
#ifdef NAKAMA_SSL_ENABLED
            if (!_wssInitialized)
            {
                _wssClient.init_asio();
                _wssInitialized = true;
            }

            // set logging to only fails
            _wssClient.clear_access_channels(websocketpp::log::alevel::all);
            _wssClient.set_access_channels(websocketpp::log::alevel::fail);
            //_wssClient.set_error_channels(websocketpp::log::elevel::warn);

            WssClient::connection_ptr con = _wssClient.get_connection(url, ec);

            if (ec)
            {
                onError("initialize connect failed: " + ec.message());
                return;
            }

            con->set_open_handler   (std::bind(&NWebsocketpp::onOpened, this, std::placeholders::_1));
            con->set_message_handler(std::bind(&NWebsocketpp::onSocketMessage, this, std::placeholders::_1, std::placeholders::_2));
            con->set_fail_handler   (std::bind(&NWebsocketpp::onFailed, this, std::placeholders::_1));
            con->set_close_handler  (std::bind(&NWebsocketpp::onClosed, this, std::placeholders::_1));

            (void)_wssClient.connect(con);
#else
            onError("SSL not enabled");
            return;
#endif
        }
        else
        {
            if (!_wsInitialized)
            {
                _wsClient.init_asio();
                _wsInitialized = true;
            }

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

            con->set_open_handler   (std::bind(&NWebsocketpp::onOpened, this, std::placeholders::_1));
            con->set_message_handler(std::bind(&NWebsocketpp::onSocketMessage, this, std::placeholders::_1, std::placeholders::_2));
            con->set_fail_handler   (std::bind(&NWebsocketpp::onFailed, this, std::placeholders::_1));
            con->set_close_handler  (std::bind(&NWebsocketpp::onClosed, this, std::placeholders::_1));

            (void)_wsClient.connect(con);
        }

        NLOG_DEBUG("...");
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

#ifdef NAKAMA_SSL_ENABLED
    if (_ssl)
    {
        if (_wssInitialized)
            _wssClient.close(_con_hdl, websocketpp::close::status::normal, "");
    }
    else
#endif
    {
        if (_wsInitialized)
            _wsClient.close(_con_hdl, websocketpp::close::status::normal, "");
    }
}

bool NWebsocketpp::send(const NBytes & data)
{
    NLOG(NLogLevel::Debug, "sending %d bytes...", data.size());

    websocketpp::lib::error_code ec;

#ifdef NAKAMA_SSL_ENABLED
    if (_ssl)
    {
        _wssClient.send(_con_hdl, data.data(), data.size(), _op_code, ec);
    }
    else
#endif
    {
        _wsClient.send(_con_hdl, data.data(), data.size(), _op_code, ec);
    }

    if (ec)
    {
        NLOG(NLogLevel::Error, "error: %d, %s", ec.value(), ec.message().c_str());
    }

    return !ec;
}

void NWebsocketpp::onOpened(websocketpp::connection_hdl hdl)
{
    NLOG_DEBUG("socket connected");

    _con_hdl = hdl;

    onConnected();
}

void NWebsocketpp::onFailed(websocketpp::connection_hdl hdl)
{
    std::string errStr;

#ifdef NAKAMA_SSL_ENABLED
    if (_ssl)
    {
        WssClient::connection_ptr con = _wssClient.get_con_from_hdl(hdl);
        websocketpp::lib::error_code ec = con->get_ec();
        errStr = "connect failed: " + ec.message();
    }
    else
#endif
    {
        WsClient::connection_ptr con = _wsClient.get_con_from_hdl(hdl);
        websocketpp::lib::error_code ec = con->get_ec();
        errStr = "connect failed: " + ec.message();
    }

    NLOG_ERROR(errStr);

    onError(errStr);
}

void NWebsocketpp::onClosed(websocketpp::connection_hdl hdl)
{
    uint16_t code;
    std::string codeStr, reason;

#ifdef NAKAMA_SSL_ENABLED
    if (_ssl)
    {
        WssClient::connection_ptr con = _wssClient.get_con_from_hdl(hdl);

        code    = con->get_remote_close_code();
        codeStr = websocketpp::close::status::get_string(code);
        reason  = con->get_remote_close_reason();
    }
    else
#endif
    {
        WsClient::connection_ptr con = _wsClient.get_con_from_hdl(hdl);

        code    = con->get_remote_close_code();
        codeStr = websocketpp::close::status::get_string(code);
        reason  = con->get_remote_close_reason();
    }

    NLOG(NLogLevel::Debug, "socket closed. code: %u, %s, %s", code, codeStr.c_str(), reason.c_str());

    onDisconnected();
}

void NWebsocketpp::onSocketMessage(websocketpp::connection_hdl hdl, WsClient::message_ptr msg)
{
    auto& payload = msg->get_payload();

    NLOG(NLogLevel::Debug, "socket message received %d bytes", payload.size());

    onMessage(payload);
}

} // namespace Nakama

#endif // BUILD_WEBSOCKETPP
