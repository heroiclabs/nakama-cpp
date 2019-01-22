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

#include "realtime/NDefaultWebsocket.h"
#include "nakama-cpp/realtime/NDefaultWebsocket.h"

namespace Nakama {

NRtTransportPtr createDefaultWebsocket()
{
    return NRtTransportPtr(new NDefaultWebsocket());
}

void NDefaultWebsocket::tick()
{
    _wsClient.poll();
}

void NDefaultWebsocket::connect(const std::string & url, const std::vector<std::string>& protocols)
{
    try {
        websocketpp::lib::error_code ec;

        _wsClient.init_asio();

        WsClient::connection_ptr con = _wsClient.get_connection(url, ec);

        con->set_open_handler([this](websocketpp::connection_hdl hdl)
        {
            _con_hdl = hdl;
            onConnected();
        });

        con->set_message_handler([this](websocketpp::connection_hdl, WsClient::message_ptr msg)
        {
            NBytes bytes;

            bytes.assign(msg->get_payload().begin(), msg->get_payload().end());

            onMessage(bytes);
        });

        con->set_fail_handler([this](websocketpp::connection_hdl hdl)
        {
            onError("fail");
        });

        con->set_close_handler([this](websocketpp::connection_hdl hdl)
        {
            onDisconnected();
        });

        _wsClient.connect(con);
    }
    catch (websocketpp::exception const & e)
    {
        std::cout << __func__ << " - " << e.what() << std::endl;
    }
}

void NDefaultWebsocket::disconnect()
{
    _wsClient.close(_con_hdl, websocketpp::close::status::normal, "");
}

void NDefaultWebsocket::send(const std::string & data)
{
    _wsClient.send(_con_hdl, data.data(), data.size(), websocketpp::frame::opcode::text);
}

void NDefaultWebsocket::send(const NBytes & data)
{
    _wsClient.send(_con_hdl, data.data(), data.size(), websocketpp::frame::opcode::binary);
}

}
