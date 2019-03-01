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

#include "NRtClient.h"
#include "DataHelper.h"
#include "nakama-cpp/StrUtil.h"
#include "nakama-cpp/log/NLogger.h"
#include "realtime/NRtClientProtocol_Protobuf.h"
#include "realtime/NRtClientProtocol_Json.h"

#undef NMODULE_NAME
#define NMODULE_NAME "NRtClient"

namespace Nakama {

NRtClient::NRtClient(NRtTransportPtr transport, const std::string& host, int port, bool ssl)
    : _transport(transport)
    , _host(host)
    , _port(port)
    , _ssl(ssl)
{
    _transport->setConnectCallback([this]()
    {
        if (_listener)
        {
            _listener->onConnect();
        }
    });

    _transport->setDisconnectCallback(std::bind(&NRtClient::onDisconnected, this));
    _transport->setMessageCallback(std::bind(&NRtClient::onMessage, this, std::placeholders::_1));

    NLOG_INFO("Created");
}

NRtClient::~NRtClient()
{
}

void NRtClient::tick()
{
    _transport->tick();
}

void NRtClient::setListener(NRtClientListenerInterface * listener)
{
    _listener = listener;
}

void NRtClient::connect(NSessionPtr session, bool createStatus, NRtClientProtocol protocol)
{
    std::string url;
    NRtTransportType transportType;

    if (_ssl)
        url.append("wss://");
    else
        url.append("ws://");

    url.append(_host).append(":").append(std::to_string(_port)).append("/ws");
    url.append("?token=").append(url_encode(session->getAuthToken()));
    url.append("&status=").append(createStatus ? "true" : "false");

    // by default server uses Json protocol
    if (protocol == NRtClientProtocol::Protobuf)
    {
        url.append("&format=protobuf");
        _protocol.reset(new NRtClientProtocol_Protobuf());
        transportType = NRtTransportType::Binary;
    }
    else
    {
        _protocol.reset(new NRtClientProtocol_Json());
        transportType = NRtTransportType::Text;
    }

    NLOG_INFO("...");
    _transport->connect(url, transportType);
}

bool NRtClient::isConnected() const
{
    return _transport->isConnected();
}

void NRtClient::disconnect()
{
    _transport->disconnect();
}

void NRtClient::onDisconnected()
{
    if (_listener)
    {
        _listener->onDisconnect();
    }
}

void NRtClient::onMessage(const NBytes & data)
{
    ::nakama::realtime::Envelope msg;

    if (!_protocol->parse(data, msg))
    {
        NLOG_ERROR("parse message failed");
        return;
    }

    NRtError error;

    if (msg.has_error())
    {
        assign(error, msg.error());

        NLOG_ERROR(toString(error));
    }

    if (msg.cid().empty())
    {
        if (_listener)
        {
            if (msg.has_error())
            {
                _listener->onError(error);
            }
            else if (msg.has_channel_message())
            {
                NChannelMessage channelMessage;
                assign(channelMessage, msg.channel_message());
                _listener->onChannelMessage(channelMessage);
            }
            else if (msg.has_channel_presence_event())
            {
                NChannelPresenceEvent channelPresenceEvent;
                assign(channelPresenceEvent, msg.channel_presence_event());
                _listener->onChannelPresence(channelPresenceEvent);
            }
            else if (msg.has_match_data())
            {
                NMatchData matchData;
                assign(matchData, msg.match_data());
                _listener->onMatchData(matchData);
            }
            else if (msg.has_match_presence_event())
            {
                NMatchPresenceEvent matchPresenceEvent;
                assign(matchPresenceEvent, msg.match_presence_event());
                _listener->onMatchPresence(matchPresenceEvent);
            }
            else if (msg.has_matchmaker_matched())
            {
                NMatchmakerMatchedPtr matchmakerMatched(new NMatchmakerMatched());
                assign(*matchmakerMatched, msg.matchmaker_matched());
                _listener->onMatchmakerMatched(matchmakerMatched);
            }
            else if (msg.has_notifications())
            {
                NNotificationList list;
                assign(list, msg.notifications());
                _listener->onNotifications(list);
            }
            else if (msg.has_status_presence_event())
            {
                NStatusPresenceEvent event;
                assign(event, msg.status_presence_event());
                _listener->onStatusPresence(event);
            }
            else if (msg.has_stream_data())
            {
                NStreamData data;
                assign(data, msg.stream_data());
                _listener->onStreamData(data);
            }
            else if (msg.has_stream_presence_event())
            {
                NStreamPresenceEvent event;
                assign(event, msg.stream_presence_event());
                _listener->onStreamPresence(event);
            }
            else
            {
                NLOG_ERROR("Unknown message received");
            }
        }
        else
        {
            NLOG_ERROR("No listener. Received message has been ignored.");
        }
    }
    else
    {
        int32_t cid = std::stoi(msg.cid());
        auto it = _reqContexts.find(cid);

        if (it != _reqContexts.end())
        {
            if (msg.has_error())
            {
                if (it->second->errorCallback)
                {
                    it->second->errorCallback(error);
                }
                else if (_listener)
                {
                    _listener->onError(error);
                }
                else
                {
                    NLOG_WARN("error not handled");
                }
            }
            else if (it->second->successCallback)
            {
                it->second->successCallback(msg);
            }

            _reqContexts.erase(it);
        }
        else
        {
            NLOG_ERROR("request context not found. cid: " + msg.cid());
        }
    }
}

void NRtClient::joinChat(
    const std::string & target,
    NChannelType type,
    const opt::optional<bool>& persistence,
    const opt::optional<bool>& hidden,
    std::function<void(NChannelPtr)> successCallback,
    RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;
    auto* channelJoin = msg.mutable_channel_join();

    channelJoin->set_target(target);
    channelJoin->set_type(static_cast<int32_t>(type));

    if (persistence) channelJoin->mutable_persistence()->set_value(*persistence);
    if (hidden) channelJoin->mutable_hidden()->set_value(*hidden);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NChannelPtr channel(new NChannel());
            assign(*channel, msg.channel());
            successCallback(channel);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::leaveChat(const std::string & channelId, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_channel_leave()->set_channel_id(channelId);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            successCallback();
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::writeChatMessage(const std::string & channelId, const std::string & content, std::function<void(const NChannelMessageAck&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_channel_message_send()->set_channel_id(channelId);
    msg.mutable_channel_message_send()->set_content(content);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NChannelMessageAck ack;
            assign(ack, msg.channel_message_ack());
            successCallback(ack);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::updateChatMessage(const std::string & channelId, const std::string & messageId, const std::string & content, std::function<void(const NChannelMessageAck&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_channel_message_update()->set_channel_id(channelId);
    msg.mutable_channel_message_update()->set_message_id(messageId);
    msg.mutable_channel_message_update()->set_content(content);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NChannelMessageAck ack;
            assign(ack, msg.channel_message_ack());
            successCallback(ack);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::removeChatMessage(const std::string & channelId, const std::string & messageId, std::function<void(const NChannelMessageAck&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_channel_message_remove()->set_channel_id(channelId);
    msg.mutable_channel_message_remove()->set_message_id(messageId);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NChannelMessageAck ack;
            assign(ack, msg.channel_message_ack());
            successCallback(ack);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::createMatch(std::function<void(const NMatch&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_match_create();

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NMatch match;
            assign(match, msg.match());
            successCallback(match);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::joinMatch(const std::string & matchId, std::function<void(const NMatch&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_match_join()->set_match_id(matchId);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NMatch match;
            assign(match, msg.match());
            successCallback(match);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::joinMatchByToken(const std::string & token, std::function<void(const NMatch&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_match_join()->set_token(token);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NMatch match;
            assign(match, msg.match());
            successCallback(match);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::leaveMatch(const std::string & matchId, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_match_leave()->set_match_id(matchId);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            successCallback();
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::addMatchmaker(
    const opt::optional<int32_t>& minCount,
    const opt::optional<int32_t>& maxCount,
    const opt::optional<std::string>& query,
    const NStringMap & stringProperties,
    const NStringDoubleMap & numericProperties,
    std::function<void(const NMatchmakerTicket&)> successCallback,
    RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;
    auto* data = msg.mutable_matchmaker_add();

    if (minCount) data->set_min_count(*minCount);
    if (maxCount) data->set_max_count(*maxCount);
    if (query) data->set_query(*query);

    for (auto it : stringProperties)
    {
        (*data->mutable_string_properties())[it.first] = it.second;
    }

    for (auto it : numericProperties)
    {
        (*data->mutable_numeric_properties())[it.first] = it.second;
    }

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NMatchmakerTicket ticket;
            assign(ticket, msg.matchmaker_ticket());
            successCallback(ticket);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::removeMatchmaker(const std::string & ticket, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_matchmaker_remove()->set_ticket(ticket);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            successCallback();
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::sendMatchData(const std::string & matchId, int64_t opCode, const NBytes & data, const std::vector<NUserPresence>& presences)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_match_data_send()->set_match_id(matchId);
    msg.mutable_match_data_send()->set_op_code(opCode);
    msg.mutable_match_data_send()->set_data(data.data(), data.size());

    for (auto& presence : presences)
    {
        auto* presenceData = msg.mutable_match_data_send()->mutable_presences()->Add();

        if (!presence.user_id.empty())
            presenceData->set_user_id(presence.user_id);

        if (!presence.username.empty())
            presenceData->set_username(presence.username);

        if (!presence.session_id.empty())
            presenceData->set_session_id(presence.session_id);

        if (!presence.status.empty())
            presenceData->mutable_status()->set_value(presence.status);

        presenceData->set_persistence(presence.persistence);
    }

    send(msg);
}

void NRtClient::followUsers(const std::vector<std::string>& userIds, std::function<void(const NStatus&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;
    auto* data = msg.mutable_status_follow();

    for (auto& id : userIds)
    {
        data->add_user_ids(id);
    }

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NStatus status;
            assign(status, msg.status());
            successCallback(status);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::unfollowUsers(const std::vector<std::string>& userIds, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;
    auto* data = msg.mutable_status_unfollow();

    for (auto& id : userIds)
    {
        data->add_user_ids(id);
    }

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            successCallback();
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::updateStatus(const std::string & status, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_status_update()->mutable_status()->set_value(status);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            successCallback();
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::rpc(const std::string & id, const opt::optional<std::string>& payload, std::function<void(const NRpc&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;
    auto* data = msg.mutable_rpc();

    data->set_id(id);

    if (payload)
        data->set_payload(*payload);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NRpc rpc;
            assign(rpc, msg.rpc());
            successCallback(rpc);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

RtRequestContext * NRtClient::createReqContext(::nakama::realtime::Envelope& msg)
{
    if (_reqContexts.empty() && _nextCid > 9)
    {
        // reset just to be one digit
        // we can reset because there are no pendig requests
        _nextCid = 0;
    }

    RtRequestContext * ctx = new RtRequestContext();
    int32_t cid = _nextCid++;

    _reqContexts.emplace(cid, std::unique_ptr<RtRequestContext>(ctx));
    msg.set_cid(std::to_string(cid));

    return ctx;
}

void NRtClient::send(const::google::protobuf::Message & msg)
{
    if (isConnected())
    {
        NBytes bytes;

        if (_protocol->serialize(msg, bytes))
        {
            _transport->send(bytes);
        }
        else
        {
            NLOG_ERROR("serialize message failed");
        }
    }
    else
    {
        NLOG_ERROR("not connected");
    }
}

}
