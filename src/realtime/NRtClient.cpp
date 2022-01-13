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

NRtClient::NRtClient(NRtTransportPtr transport, const std::string& host, int32_t port, bool ssl)
    : _host(host)
    , _port(port)
    , _ssl(ssl)
    , _transport(transport)
{
    NLOG_INFO("Created");

    if (_port == DEFAULT_PORT)
    {
        _port = _ssl ? 443 : 7350;

        NLOG(NLogLevel::Info, "using default port %d", _port);
    }

    _transport->setConnectCallback([this]()
    {
        NLOG_DEBUG("connected");

        if (_listener)
        {
            _listener->onConnect();
        }
    });

    _transport->setErrorCallback(std::bind(&NRtClient::onTransportError, this, std::placeholders::_1));
    _transport->setDisconnectCallback(std::bind(&NRtClient::onTransportDisconnected, this, std::placeholders::_1));
    _transport->setMessageCallback(std::bind(&NRtClient::onTransportMessage, this, std::placeholders::_1));
}

NRtClient::~NRtClient()
{
    if (_reqContexts.size() > 0)
    {
        NLOG(NLogLevel::Warn, "Not handled %u realtime requests detected.", _reqContexts.size());
    }
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
    url.append("?token=").append(urlEncode(session->getAuthToken()));
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

void NRtClient::onTransportDisconnected(const NRtClientDisconnectInfo& info)
{
    NLOG(NLogLevel::Debug, "code: %u, %s", info.code, info.reason.c_str());

    if (_listener)
    {
        _listener->onDisconnect(info);
    }
}

void NRtClient::onTransportError(const std::string& description)
{
    NRtError error;

    error.message = description;
    error.code    = _transport->isConnected() ? RtErrorCode::TRANSPORT_ERROR : RtErrorCode::CONNECT_ERROR;

    NLOG_ERROR(toString(error));

    if (_listener)
    {
        _listener->onError(error);
    }
}

void NRtClient::onTransportMessage(const NBytes & data)
{
    ::nakama::realtime::Envelope msg;

    if (!_protocol->parse(data, msg))
    {
        onTransportError("parse message failed");
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
            else if (msg.has_party())
            {
                NParty party;
                assign(party, msg.party());
                _listener->onParty(party);
            }
            else if (msg.has_party_close())
            {
                NPartyClose partyClose;
                assign(partyClose, msg.party_close());
                _listener->onPartyClosed(partyClose);
            }
            else if (msg.has_party_data())
            {
                NPartyData partyData;
                assign(partyData, msg.party_data());
                _listener->onPartyData(partyData);
            }
            else if (msg.has_party_join_request())
            {
                NPartyJoinRequest partyRequest;
                assign(partyRequest, msg.party_join_request());
                _listener->onPartyJoinRequest(partyRequest);
            }
            else if (msg.has_party_leader())
            {
                NPartyLeader partyLeader;
                assign(partyLeader, msg.party_leader());
                _listener->onPartyLeader(partyLeader);
            }
            else if (msg.has_party_matchmaker_ticket())
            {
                NPartyMatchmakerTicket partyTicket;
                assign(partyTicket, msg.party_matchmaker_ticket());
                _listener->onPartyMatchmakerTicket(partyTicket);
            }
            else if (msg.has_party_presence_event())
            {
                NPartyPresenceEvent presenceEvent;
                assign(presenceEvent, msg.party_presence_event());
                _listener->onPartyPresence(presenceEvent);
            }
            else
            {
                onTransportError("Unknown message received");
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
                    NLOG_WARN("^ error not handled");
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
            onTransportError("request context not found. cid: " + msg.cid());
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

    {
        auto *channel_message = msg.mutable_channel_message_update();

        channel_message->set_channel_id(channelId);
        channel_message->set_message_id(messageId);
        channel_message->set_content(content);
    }

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

void NRtClient::joinMatch(
    const std::string & matchId,
    const NStringMap& metadata,
    std::function<void(const NMatch&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;
    auto match_join = msg.mutable_match_join();

    match_join->set_match_id(matchId);

    for (auto p : metadata)
    {
        match_join->mutable_metadata()->insert({ p.first, p.second });
    }

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
    auto* match_data = msg.mutable_match_data_send();

    match_data->set_match_id(matchId);
    match_data->set_op_code(opCode);
    match_data->set_data(data.data(), data.size());

    for (auto& presence : presences)
    {
        if (presence.userId.empty())
        {
            NLOG_ERROR("Please set 'userId' for user presence");
            continue;
        }

        if (presence.sessionId.empty())
        {
            NLOG_ERROR("Please set 'sessionId' for user presence");
            continue;
        }

        auto* presenceData = match_data->mutable_presences()->Add();

        presenceData->set_user_id(presence.userId);
        presenceData->set_session_id(presence.sessionId);

        if (!presence.username.empty())
            presenceData->set_username(presence.username);

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

void NRtClient::acceptPartyMember(const std::string& partyId, NUserPresence& presence, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_accept()->set_party_id(partyId);
    msg.mutable_party_accept()->mutable_presence()->set_user_id(presence.userId);
    msg.mutable_party_accept()->mutable_presence()->set_persistence(presence.persistence);
    msg.mutable_party_accept()->mutable_presence()->set_session_id(presence.sessionId);
    msg.mutable_party_accept()->mutable_presence()->set_username(presence.username);

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

void NRtClient::addMatchmakerParty(const std::string& partyId, const std::string& query, int minCount, int maxCount,
    const NStringMap stringProperties, const NStringDoubleMap numericProperties, std::function<void(const NPartyMatchmakerTicket&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_matchmaker_add()->set_party_id(partyId);
    msg.mutable_party_matchmaker_add()->set_query(query);
    msg.mutable_party_matchmaker_add()->set_min_count(minCount);
    msg.mutable_party_matchmaker_add()->set_max_count(maxCount);

    for (auto it : stringProperties)
    {
        (*(msg.mutable_party_matchmaker_add()->mutable_string_properties()))[it.first] = it.second;
    }

    for (auto it : numericProperties)
    {
        (*(msg.mutable_party_matchmaker_add()->mutable_numeric_properties()))[it.first] = it.second;
    }

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NPartyMatchmakerTicket ticket;
            assign(ticket, msg.party_matchmaker_ticket());
            successCallback(ticket);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::closeParty(const std::string& partyId, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_close()->set_party_id(partyId);

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

void NRtClient::createParty(bool open, int maxSize, std::function<void(const NParty&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_create()->set_open(open);
    msg.mutable_party_create()->set_max_size(maxSize);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NParty party;
            assign(party, msg.party());
            successCallback(party);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::joinParty(const std::string& partyId, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_join()->set_party_id(partyId);
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

void NRtClient::leaveParty(const std::string& partyId, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_leave()->set_party_id(partyId);

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

void NRtClient::listPartyJoinRequests(const std::string& partyId, std::function<void(const NPartyJoinRequest&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_join_request_list()->set_party_id(partyId);

    RtRequestContext * ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& msg)
        {
            NPartyJoinRequest joinRequest;
            assign(joinRequest, msg.party_join_request());
            successCallback(joinRequest);
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::promotePartyMember(const std::string& partyId, NUserPresence& partyMember, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_promote()->set_party_id(partyId);

    msg.mutable_party_promote()->mutable_presence()->set_user_id(partyMember.userId);
    msg.mutable_party_promote()->mutable_presence()->set_persistence(partyMember.persistence);
    msg.mutable_party_promote()->mutable_presence()->set_session_id(partyMember.sessionId);
    msg.mutable_party_promote()->mutable_presence()->set_username(partyMember.username);

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

void NRtClient::removeMatchmakerParty(const std::string& partyId, const std::string& ticket, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_matchmaker_remove()->set_party_id(partyId);
    msg.mutable_party_matchmaker_remove()->set_ticket(ticket);

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

void NRtClient::removePartyMember(const std::string& partyId, NUserPresence& presence, std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_remove()->set_party_id(partyId);

    msg.mutable_party_remove()->mutable_presence()->set_user_id(presence.userId);
    msg.mutable_party_remove()->mutable_presence()->set_persistence(presence.persistence);
    msg.mutable_party_remove()->mutable_presence()->set_session_id(presence.sessionId);
    msg.mutable_party_remove()->mutable_presence()->set_username(presence.username);

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

void NRtClient::sendPartyData(const std::string& partyId, long opCode, NBytes& data)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_data_send()->set_party_id(partyId);
    msg.mutable_party_data_send()->set_op_code(opCode);
    msg.mutable_party_data_send()->set_data(data);

    RtRequestContext * ctx = createReqContext(msg);

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

void NRtClient::reqInternalError(int32_t cid, const NRtError & error)
{
    NLOG_ERROR(toString(error));

    auto it = _reqContexts.find(cid);

    if (it != _reqContexts.end())
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

        _reqContexts.erase(it);
    }
    else
    {
        NLOG(NLogLevel::Error, "request context not found. cid: %d", cid);
    }
}

void NRtClient::send(const ::nakama::realtime::Envelope & msg)
{
    if (isConnected())
    {
        NBytes bytes;

        if (_protocol->serialize(msg, bytes))
        {
            if (!_transport->send(bytes))
            {
                reqInternalError(std::stoi(msg.cid()), NRtError(RtErrorCode::TRANSPORT_ERROR, "Send message failed"));

                _transport->disconnect();
            }
        }
        else
        {
            reqInternalError(std::stoi(msg.cid()), NRtError(RtErrorCode::TRANSPORT_ERROR, "Serialize message failed"));
        }
    }
    else
    {
        reqInternalError(std::stoi(msg.cid()), NRtError(RtErrorCode::CONNECT_ERROR, "Not connected"));
    }
}

}
