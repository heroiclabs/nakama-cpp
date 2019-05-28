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

#ifdef BUILD_WEBSOCKET_CPPREST

#include "NWebsocketCppRest.h"
#include "nakama-cpp/log/NLogger.h"
#include "nakama-cpp/NakamaVersion.h"
#include "nakama-cpp/NUtils.h"
#include "CppRestUtils.h"

#undef NMODULE_NAME
#define NMODULE_NAME "NWebsocketCppRest"

namespace Nakama {

NWebsocketCppRest::NWebsocketCppRest():
_lastSentPingTimeMs(0),
_lastReceivedPongTimeMs(0)
{
    NLOG_DEBUG("");
}

NWebsocketCppRest::~NWebsocketCppRest()
{
    disconnect();
}

void NWebsocketCppRest::setPingSettings(const NRtPingSettings & settings)
{
    _settings = settings;
}

NRtPingSettings NWebsocketCppRest::getPingSettings() const
{
    return _settings;
}

void NWebsocketCppRest::tick()
{
    if (_wsClient && _connected) {
        
        if (_settings.intervalSec > 0 && getUnixTimestampMs()-_lastSentPingTimeMs >= 1000*_settings.intervalSec)
        {
            if (sendPing())
            {
                _lastSentPingTimeMs = getUnixTimestampMs();
            }
        }
        if (_settings.timeoutSec > 0 && getUnixTimestampMs()-_lastReceivedPongTimeMs >= 1000*_settings.timeoutSec)
        {
            disconnect(web::websockets::client::websocket_close_status::pong_timeout, "Pong timeout");
        }
    }

    std::lock_guard<std::mutex> guard(_mutex);

    if (_connectedEvent)
    {
        fireOnConnected();
        _connectedEvent = false;
    }

    while (!_errorEvents.empty())
    {
        fireOnError(_errorEvents.front());
        _errorEvents.pop_front();
    }

    while (!_messageEvents.empty())
    {
        const std::string& msg = _messageEvents.front();
        NLOG(NLogLevel::Debug, "socket message received %d bytes", msg.size());
        fireOnMessage(msg);
        _messageEvents.pop_front();
    }

    if (_disconnectEvent)
    {
        fireOnDisconnected(*_disconnectEvent);
        _disconnectEvent.reset();
    }
}

void NWebsocketCppRest::connect(const std::string & url, NRtTransportType type)
{
    try
    {
        NLOG_DEBUG("...");

        web::websockets::client::websocket_client_config config;

        config.set_user_agent(std::string("Nakama C++ ") + getNakamaSdkVersion());
        
        _wsClient.reset(new WsClient(config));

        _wsClient->set_message_handler(std::bind(&NWebsocketCppRest::onSocketMessage, this, std::placeholders::_1));
        _wsClient->set_close_handler(std::bind(&NWebsocketCppRest::onClosed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        
        _type = type;
        _disconnectInitiated = false;

        web::uri uri(FROM_STD_STR(url));

        auto task = _wsClient->connect(uri);
        // Task-based continuation.
        (void) task.then([this](pplx::task<void> previousTask)
        {
            try
            {
                previousTask.get();
                onOpened();
            }
            catch (const std::exception & e)
            {
                addErrorEvent("connect failed: " + std::string(e.what()));
            }
        });
    }
    catch (std::exception const & e)
    {
        addErrorEvent("connect failed: " + std::string(e.what()));
    }
}

void NWebsocketCppRest::disconnect()
{
    disconnect(web::websockets::client::websocket_close_status::normal, "Normal close");
}

void NWebsocketCppRest::disconnect(web::websockets::client::websocket_close_status status, const std::string& reason)
{
    if (!_wsClient)
        return;

    NLOG_DEBUG("...");

    _disconnectInitiated = true;
    _connected = false;

    auto task = _wsClient->close(status, reason);
    // Task-based continuation
    (void) task.then([this](pplx::task<void> previousTask)
    {
        try
        {
            previousTask.get();
        }
        catch (const std::exception & e)
        {
            addErrorEvent("[NWebsocketCppRest::disconnect] exception: " + std::string(e.what()));
        }
    });

    _wsClient.reset();
}

bool NWebsocketCppRest::sendData(const NBytes & data, bool isPing)
{
    if (!_wsClient || !_connected)
    {
        NLOG_ERROR("send failed - not connected");
        return false;
    }

    bool res = false;

    try
    {
        web::websockets::client::websocket_outgoing_message msg;

        if (isPing)
        {
            NLOG(NLogLevel::Debug, "sending ping %d bytes text ...", data.size());
            msg.set_ping_message();
        }
        else if (_type == NRtTransportType::Binary)
        {
            NLOG(NLogLevel::Debug, "sending %d bytes binary ...", data.size());
            msg.set_binary_message(concurrency::streams::bytestream::open_istream(data));
        }
        else
        {
            NLOG(NLogLevel::Debug, "sending %d bytes text ...", data.size());
            msg.set_utf8_message(data);
        }

        auto task = _wsClient->send(std::move(msg));
        // Task-based continuation
        (void) task.then([this](pplx::task<void> previousTask)
        {
            try
            {
                previousTask.get();
            }
            catch (const std::exception & e)
            {
                addErrorEvent("[NWebsocketCppRest::send] exception: " + std::string(e.what()));
            }
        });

        res = true;
    }
    catch (std::exception const& e)
    {
        addErrorEvent("[NWebsocketCppRest::send] exception: " + std::string(e.what()));
    }

    return res;
}

bool NWebsocketCppRest::sendPing()
{
    bool isPing = true;
    return sendData("", isPing);
}

bool NWebsocketCppRest::send(const NBytes & data)
{
    return sendData(data);
}

// will be executed from internal thread of WsClient
void NWebsocketCppRest::onOpened()
{
    _lastSentPingTimeMs = getUnixTimestampMs();
    _lastReceivedPongTimeMs = getUnixTimestampMs();

    std::lock_guard<std::mutex> guard(_mutex);
    _connectedEvent = true;
    _connected = true;
}

// will be executed from internal thread of WsClient
void NWebsocketCppRest::onClosed(web::websockets::client::websocket_close_status close_status,
    const utility::string_t& reason,
    const std::error_code& error)
{
    std::lock_guard<std::mutex> guard(_mutex);

    _disconnectEvent.reset(new NRtClientDisconnectInfo);

    _disconnectEvent->code   = static_cast<uint16_t>(close_status);
    _disconnectEvent->reason = TO_STD_STR(reason);

    if (_disconnectEvent->code == 1005) // No Status Received
    {
        _disconnectEvent->remote = !_disconnectInitiated;
    }
    else
    {
        _disconnectEvent->remote = !_disconnectInitiated/* && !websocketpp::close::status::invalid(info.code)*/;
    }
}

// will be executed from internal thread of WsClient
void NWebsocketCppRest::onSocketMessage(const web::websockets::client::websocket_incoming_message& msg)
{
    try
    {
        switch (msg.message_type())
        {
        case web::websockets::client::websocket_message_type::binary_message:
        {
            auto buf = msg.body().streambuf();
            auto size = static_cast<size_t>(buf.size());

            if (size > 0)
            {
                NBytes payload;
                payload.resize(size);
                (void) buf.scopy(reinterpret_cast<uint8_t*>(&payload[0]), size);

                std::lock_guard<std::mutex> guard(_mutex);
                _messageEvents.emplace_back(std::move(payload));
            }
            break;
        }

        case web::websockets::client::websocket_message_type::text_message:
        {
            auto task = msg.extract_string();
            auto payload = task.get();

            std::lock_guard<std::mutex> guard(_mutex);
            _messageEvents.emplace_back(std::move(payload));
            break;
        }

        case web::websockets::client::websocket_message_type::ping:
        {
            NLOG_DEBUG("ping");
            break;
        }

        case web::websockets::client::websocket_message_type::pong:
        {
            NLOG_DEBUG("pong");
            _lastReceivedPongTimeMs = getUnixTimestampMs();
            break;
        }

        default:
            break;
        }
    }
    catch (std::exception const& e)
    {
        addErrorEvent("[NWebsocketCppRest::onSocketMessage] exception: " + std::string(e.what()));
    }
}

// might be executed from internal thread of WsClient
void NWebsocketCppRest::addErrorEvent(std::string&& err)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _errorEvents.emplace_back(std::move(err));
}

} // namespace Nakama

#endif // BUILD_WEBSOCKET_CPPREST
