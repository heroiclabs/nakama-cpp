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

NWebsocketCppRest::NWebsocketCppRest()
{
    NLOG_DEBUG("");
}

NWebsocketCppRest::~NWebsocketCppRest()
{
    disconnect();
}

void NWebsocketCppRest::setActivityTimeout(uint32_t timeoutMs)
{
    _activityTimeoutMs = timeoutMs;
}

uint32_t NWebsocketCppRest::getActivityTimeout() const
{
    return _activityTimeoutMs;
}

void NWebsocketCppRest::tick()
{
    if (isConnected())
    {
        if (_activityTimeoutMs > 0 && getUnixTimestampMs()-_lastReceivedMessageTimeMs >= _activityTimeoutMs)
        {
            disconnect(web::websockets::client::websocket_close_status::activity_timeout, "Activity timeout");
        }
    }

    std::lock_guard<std::mutex> guard(_mutex);

    while (!_userThreadFuncs.empty())
    {
        _userThreadFuncs.front()();
        _userThreadFuncs.pop_front();
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

    NLOG_DEBUG(reason);

    _disconnectInitiated = true;
    _connected = false;

    auto task = _wsClient->close(status, FROM_STD_STR(reason));
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

bool NWebsocketCppRest::send(const NBytes & data)
{
    if (!isConnected())
    {
        NLOG_ERROR("send failed - not connected");
        return false;
    }

    bool res = false;

    try
    {
        web::websockets::client::websocket_outgoing_message msg;

        if (_type == NRtTransportType::Binary)
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

void NWebsocketCppRest::executeInUserThread(UserThreadFunc&& userThreadFunc)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _userThreadFuncs.push_back(std::move(userThreadFunc));
}

// will be executed from internal thread of WsClient
void NWebsocketCppRest::onOpened()
{
    _lastReceivedMessageTimeMs = getUnixTimestampMs();

    executeInUserThread([this]()
    {
        fireOnConnected();
    });
}

// will be executed from internal thread of WsClient
void NWebsocketCppRest::onClosed(web::websockets::client::websocket_close_status close_status,
    const utility::string_t& reason,
    const std::error_code& error)
{
    std::shared_ptr<NRtClientDisconnectInfo> disconnectInfo(new NRtClientDisconnectInfo);

    disconnectInfo->code = static_cast<uint16_t>(close_status);
    disconnectInfo->reason = TO_STD_STR(reason);

    if (disconnectInfo->code == 1005) // No Status Received
    {
        disconnectInfo->remote = !_disconnectInitiated;
    }
    else
    {
        disconnectInfo->remote = !_disconnectInitiated/* && !websocketpp::close::status::invalid(info.code)*/;
    }

    executeInUserThread([this, disconnectInfo]()
    {
        fireOnDisconnected(*disconnectInfo);
    });
}

// will be executed from internal thread of WsClient
void NWebsocketCppRest::onSocketMessage(const web::websockets::client::websocket_incoming_message& msg)
{
    try
    {
        _lastReceivedMessageTimeMs = getUnixTimestampMs();

        switch (msg.message_type())
        {
        case web::websockets::client::websocket_message_type::binary_message:
        {
            auto buf = msg.body().streambuf();
            auto size = static_cast<size_t>(buf.size());

            if (size > 0)
            {
                std::shared_ptr<NBytes> payload(new NBytes());
                payload->resize(size);
                (void) buf.scopy(reinterpret_cast<uint8_t*>(&payload->at(0)), size);

                executeInUserThread([this, payload]()
                {
                    NLOG(NLogLevel::Debug, "socket message received %d bytes", payload->size());
                    fireOnMessage(*payload);
                });
            }
            break;
        }

        case web::websockets::client::websocket_message_type::text_message:
        {
            std::shared_ptr<NBytes> payload(new NBytes());
            auto task = msg.extract_string();
            *payload = task.get();

            executeInUserThread([this, payload]()
            {
                NLOG(NLogLevel::Debug, "socket message received %d bytes", payload->size());
                fireOnMessage(*payload);
            });
            break;
        }

        case web::websockets::client::websocket_message_type::ping:
        {
            executeInUserThread([]()
            {
                NLOG_DEBUG("ping");
            });
            break;
        }

        case web::websockets::client::websocket_message_type::pong:
        {
            executeInUserThread([]()
            {
                NLOG_DEBUG("pong");
            });
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
    std::shared_ptr<std::string> errPtr(new std::string());

    *errPtr = std::move(err);

    executeInUserThread([this, errPtr]()
    {
        fireOnError(*errPtr);
    });
}

} // namespace Nakama

#endif // BUILD_WEBSOCKET_CPPREST
