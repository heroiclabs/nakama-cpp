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
#include "StrUtil.h"
#include "nakama-cpp/log/NLogger.h"
#include "NRtClientProtocol_Protobuf.h"
#include "NRtClientProtocol_Json.h"
#include "nakama-cpp/NUtils.h"
#include "nakama-cpp/realtime/rtdata/NRtException.h"

#undef NMODULE_NAME
#define NMODULE_NAME "NRtClient"

namespace Nakama {

NRtClient::NRtClient(NRtTransportPtr transport, const std::string& host, int32_t port, bool ssl)
    : _host(host)
    , _port(port)
    , _ssl(ssl)
    , _transport(transport)
    , _connectPromise(std::make_unique<std::promise<void>>())
{
    NLOG_INFO("Created");

    if (_port == DEFAULT_PORT)
    {
        _port = _ssl ? 443 : 7350;

        NLOG(NLogLevel::Info, "using default port %d", _port);
    }

    _transport->setConnectCallback(std::bind(&NRtClient::onTransportConnected, this));
    _transport->setErrorCallback(std::bind(&NRtClient::onTransportError, this, std::placeholders::_1));
    _transport->setDisconnectCallback(std::bind(&NRtClient::onTransportDisconnected, this, std::placeholders::_1));
    _transport->setMessageCallback(std::bind(&NRtClient::onTransportMessage, this, std::placeholders::_1));
}

NRtClient::~NRtClient()
{
    // transport destructor can invoke outstanding callbacks. Call destructor here manually
    // while rest of NRtClient is still intact
    _transport.reset();

    // Not taking lock here, because if it comes to destroying this object, then no other
    // methods should be called on it anyway
    if (_reqContexts.size() > 0)
    {
        NLOG(NLogLevel::Warn, "Not handled %u realtime requests detected.", _reqContexts.size());
    }
}

void NRtClient::tick()
{
    heartbeat();
    _transport->tick();
}

void NRtClient::setListener(NRtClientListenerInterface * listener)
{
    _listener = listener;
}

void NRtClient::connect(NSessionPtr session, bool createStatus, NRtClientProtocol protocol)
{
    if (_transport->isConnected() || _transport->isConnecting())
    {
        return;
    }

    std::string url;
    NRtTransportType transportType;

    if (_ssl)
        url.append("wss://");
    else
        url.append("ws://");

    url.append(_host).append(":").append(std::to_string(_port)).append("/ws");
    url.append("?token=").append(encodeURIComponent(session->getAuthToken()));
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
    _wantDisconnect = false;
    _transport->connect(url, transportType);
}

std::future<void> NRtClient::connectAsync(NSessionPtr session, bool createStatus, NRtClientProtocol protocol)
{
    if (_transport->isConnected() || _transport->isConnecting())
    {
        std::promise<void> emptyPromise = std::promise<void>();
        emptyPromise.set_value();
        return emptyPromise.get_future();
    }

    // stomp the old promise
    _connectPromise = std::make_unique<std::promise<void>>();

    // already connected
    if (_transport->isConnected()) {

        _connectPromise->set_value();
        return _connectPromise->get_future();
    }

    connect(session, createStatus, protocol);

    return _connectPromise->get_future();
}

bool NRtClient::isConnecting() const
{
    return _transport->isConnecting();
}

bool NRtClient::isConnected() const
{
    return _transport->isConnected();
}

void NRtClient::disconnect()
{
    NRtClientDisconnectInfo info{NRtClientDisconnectInfo::Code::NORMAL_CLOSURE, "Disconnect initiated by us", false};
    disconnect(info);
}

std::future<void> NRtClient::disconnectAsync()
{
    // currently, disconnect callback is invoked immediately by client here, so we just return a completed future.
    disconnect();
    std::promise<void> emptyPromise = std::promise<void>();
    emptyPromise.set_value();
    return emptyPromise.get_future();
}

void NRtClient::disconnect(const NRtClientDisconnectInfo& info)
{
    // This weird logic to check predisconnect state is here to support case
    // where disconnect is called after connection initialized, but hasn't complete yet and
    // transport's fireOnConnected didn't trigger.
    //
    // Unconditional transport->disconnect() call gives transport a chance to either
    // cancel connection and trigger neither onConnected nor onDisconnected or
    // arrange onDisconnected to be triggered after onConnected eventually happens.
    //
    // Because it is a requirement for transport->isConnected() to return false immediately after
    // transport->disconnect() returns, we query connection status upfront and fire
    // callback only if we were connected. Even though behaviour of disconnecting already
    // disconnected client is not well-defined right now, it's kinda sensible not to trigger
    // callback if there were no connection in the first place to keep connect/disconnect callbacks
    // symmetrical.
    bool wasConnected = _transport->isConnected();
    _transport->disconnect();
    _wantDisconnect = true;

    if (wasConnected) {
        /*
        When initiating disconnect from the client side, call onDisconnect
        callback immediately without waiting for confirmation from the server.

        Confirmation might never arrive and even when it arrives, transport is
        supposed to ignore any incoming messages after disconnect returns.
         */
        onTransportDisconnected(info);
    }
}

void NRtClient::onTransportConnected()
{
    _heartbeatFailureReported = false;

    if (_listener)
    {
        _listener->onConnect();
    }

    try
    {
        // signal to the user's future that the connection has completed.
        _connectPromise->set_value();
    }
    catch (const std::future_error&)
    {
        // if we get an exception here, it means the connect promise has completed already from a previous connect.
        // this can happen if the transport double fires or some other unexpected cases, like the user disconnecting while a connection is being made.
    }
}

void NRtClient::onTransportDisconnected(const NRtClientDisconnectInfo& info)
{
    NLOG(NLogLevel::Debug, "code: %u, remote: %d, %s", info.code, info.remote, info.reason.c_str());

    cancelAllRequests(RtErrorCode::DISCONNECTED);

    if (_listener)
    {
        _listener->onDisconnect(info);
    }

    try
    {
        // assume we are disconnecting mid-connect
       _connectPromise->set_exception(std::make_exception_ptr<NRtException>(NRtException(NRtError(RtErrorCode::CONNECT_ERROR, "Disconnected while connecting."))));
    }
    catch(const std::future_error& e)
    {
        // we've already set the state on this, so we've already connected, so nothing else to do.
    }


}

void NRtClient::onTransportError(const std::string& description)
{
    NRtError error;

    error.message = description;
    //TODO: don't guess, let transport report connection error explicitly
    error.code    = _transport->isConnected() ? RtErrorCode::TRANSPORT_ERROR : RtErrorCode::CONNECT_ERROR;

    NLOG_ERROR(toString(error));

    if (_listener)
    {
        _listener->onError(error);
    }

    bool futureCompleted = _connectPromise->get_future().wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    if (!futureCompleted)
    {
        _connectPromise->set_exception(std::make_exception_ptr<NRtException>(NRtException(NRtError(RtErrorCode::CONNECT_ERROR, "An error occurred while connecting."))));
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
                NStreamData streamData;
                assign(streamData, msg.stream_data());
                _listener->onStreamData(streamData);
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
        std::shared_ptr<RtRequestContext> ctx;
        {
            // Important not to hold lock while calling callbacks
            std::lock_guard<std::mutex> lock(_reqContextsLock);
            auto it = _reqContexts.find(cid);
            if (it != _reqContexts.end())
            {
                ctx.swap(it->second);
                _reqContexts.erase(it);
            }
        }

        if (ctx)
        {
            if (msg.has_error())
            {
                if (ctx->errorCallback)
                {
                    ctx->errorCallback(error);
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
            else if (ctx->successCallback)
            {
                ctx->successCallback(msg);
            }
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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
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
    const opt::optional<int32_t>& countMultiple,
    std::function<void(const NMatchmakerTicket&)> successCallback,
    RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;
    auto* data = msg.mutable_matchmaker_add();

    if (minCount) data->set_min_count(*minCount);
    if (maxCount) data->set_max_count(*maxCount);
    if (query) data->set_query(*query);
    if (countMultiple) data->mutable_count_multiple()->set_value(*countMultiple);

    for (auto it : stringProperties)
    {
        (*data->mutable_string_properties())[it.first] = it.second;
    }

    for (auto it : numericProperties)
    {
        (*data->mutable_numeric_properties())[it.first] = it.second;
    }

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
        {
            successCallback();
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::sendMatchData(const std::string & matchId, std::int64_t opCode, const NBytes & data, const std::vector<NUserPresence>& presences)
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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
        {
            successCallback();
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

void NRtClient::addMatchmakerParty(const std::string& partyId, const std::string& query, int minCount, int maxCount,
    const NStringMap& stringProperties, const NStringDoubleMap& numericProperties, const opt::optional<int32_t>& countMultiple, std::function<void(const NPartyMatchmakerTicket&)> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_party_matchmaker_add()->set_party_id(partyId);
    msg.mutable_party_matchmaker_add()->set_query(query);
    msg.mutable_party_matchmaker_add()->set_min_count(minCount);
    msg.mutable_party_matchmaker_add()->set_max_count(maxCount);

    if (countMultiple)
        msg.mutable_party_matchmaker_add()->mutable_count_multiple()->set_value(*countMultiple);

    for (auto& it : stringProperties)
    {
        (*(msg.mutable_party_matchmaker_add()->mutable_string_properties()))[it.first] = it.second;
    }

    for (auto& it : numericProperties)
    {
        (*(msg.mutable_party_matchmaker_add()->mutable_numeric_properties()))[it.first] = it.second;
    }

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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
    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
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

    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
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

    createReqContext(msg);

    send(msg);
}

void NRtClient::ping(std::function<void()> successCallback, RtErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ::nakama::realtime::Envelope msg;

    msg.mutable_ping();
    std::shared_ptr<RtRequestContext> ctx = createReqContext(msg);

    if (successCallback)
    {
        ctx->successCallback = [successCallback](::nakama::realtime::Envelope& /*msg*/)
        {
            successCallback();
        };
    }
    ctx->errorCallback = errorCallback;

    send(msg);
}

std::future<NChannelPtr> NRtClient::joinChatAsync(
            const std::string& target,
            NChannelType type,
            const opt::optional<bool>& persistence,
            const opt::optional<bool>& hidden
)
{
    auto promise = std::make_shared<std::promise<NChannelPtr>>();

    joinChat(target, type, persistence, hidden,
        [=](NChannelPtr channel) {
            promise->set_value(channel);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<void> NRtClient::leaveChatAsync(
    const std::string& channelId
)
{
    leaveChatAsync(channelId);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<NChannelMessageAck> NRtClient::writeChatMessageAsync(
    const std::string& channelId,
    const std::string& content
)
{
    auto promise = std::make_shared<std::promise<NChannelMessageAck>>();

    writeChatMessage(channelId, content,
        [=](const NChannelMessageAck& ack) {
            promise->set_value(ack);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<NChannelMessageAck> NRtClient::updateChatMessageAsync(
    const std::string& channelId,
    const std::string& messageId,
    const std::string& content
)
{
    auto promise = std::make_shared<std::promise<NChannelMessageAck>>();

    updateChatMessage(channelId, messageId, content,
        [=](const NChannelMessageAck& ack) {
            promise->set_value(ack);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<void> NRtClient::removeChatMessageAsync(
    const std::string& channelId,
    const std::string& messageId
)
{
    removeChatMessage(channelId, messageId);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<NMatch> NRtClient::createMatchAsync()
{
    auto promise = std::make_shared<std::promise<NMatch>>();

    createMatch(
        [=](const NMatch& match) {
            promise->set_value(match);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

        return promise->get_future();
}

std::future<NMatch> NRtClient::joinMatchAsync(
    const std::string& matchId,
    const NStringMap& metadata
)
{
    auto promise = std::make_shared<std::promise<NMatch>>();

    joinMatch(matchId, metadata,
        [=](const NMatch& match) {
            promise->set_value(match);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<NMatch> NRtClient::joinMatchByTokenAsync(
    const std::string& token
)
{
    auto promise = std::make_shared<std::promise<NMatch>>();

    joinMatchByToken(token,
        [=](const NMatch& match) {
            promise->set_value(match);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<void> NRtClient::leaveMatchAsync(
    const std::string& matchId
)
{
    leaveMatch(matchId);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<NMatchmakerTicket> NRtClient::addMatchmakerAsync(
    const opt::optional<int32_t>& minCount,
    const opt::optional<int32_t>& maxCount,
    const opt::optional<std::string>& query,
    const NStringMap& stringProperties,
    const NStringDoubleMap& numericProperties,
    const opt::optional<int32_t>& countMultiple
)
{
    auto promise = std::make_shared<std::promise<NMatchmakerTicket>>();

    addMatchmaker(minCount, maxCount, query, stringProperties, numericProperties, countMultiple,
        [=](const NMatchmakerTicket& ticket) {
            promise->set_value(ticket);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<void> NRtClient::removeMatchmakerAsync(
    const std::string& ticket
)
{
    removeMatchmaker(ticket);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<void> NRtClient::sendMatchDataAsync(
    const std::string& matchId,
    std::int64_t opCode,
    const NBytes& data,
    const std::vector<NUserPresence>& presences
)
{
    sendMatchData(matchId, opCode, data);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<NStatus> NRtClient::followUsersAsync(
    const std::vector<std::string>& userIds
)
{
    auto promise = std::make_shared<std::promise<NStatus>>();

    followUsers(userIds,
        [=](const NStatus& ticket) {
            promise->set_value(ticket);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<void> NRtClient::unfollowUsersAsync(
    const std::vector<std::string>& userIds
)
{
    unfollowUsers(userIds);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<void> NRtClient::updateStatusAsync(
    const std::string& status
)
{
    updateStatusAsync(status);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<NRpc> NRtClient::rpcAsync(
    const std::string& id,
    const opt::optional<std::string>& payload
)
{
    auto promise = std::make_shared<std::promise<NRpc>>();
    std::cout << "rpc async";

    rpc(id, payload,
        [=](const NRpc& rpc) {
            promise->set_value(rpc);
        },
        [=](const NRtError& error) {
    std::cout << "exception in lambda";
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<void> NRtClient::acceptPartyMemberAsync(const std::string& partyId, NUserPresence& presence)
{
    acceptPartyMember(partyId, presence);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<NPartyMatchmakerTicket> NRtClient::addMatchmakerPartyAsync(const std::string& partyId, const std::string& query, int32_t minCount, int32_t maxCount,
    const NStringMap& stringProperties, const NStringDoubleMap& numericProperties,
    const opt::optional<int32_t>& countMultiple)
{
    auto promise = std::make_shared<std::promise<NPartyMatchmakerTicket>>();

    addMatchmakerParty(partyId, query, minCount, maxCount, stringProperties, numericProperties, countMultiple,
        [=](const NPartyMatchmakerTicket& ticket) {
            promise->set_value(ticket);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<void> NRtClient::closePartyAsync(const std::string& partyId)
{
    closeParty(partyId);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<NParty> NRtClient::createPartyAsync(bool open, int maxSize)
{
    auto promise = std::make_shared<std::promise<NParty>>();

    createParty(open, maxSize,
        [=](const NParty& party) {
            promise->set_value(party);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<void> NRtClient::joinPartyAsync(const std::string& partyId)
{
    joinParty(partyId);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<void> NRtClient::leavePartyAsync(const std::string& partyId)
{
    leaveParty(partyId);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<NPartyJoinRequest> NRtClient::listPartyJoinRequestsAsync(const std::string& partyId)
{
    auto promise = std::make_shared<std::promise<NPartyJoinRequest>>();

    listPartyJoinRequests(partyId,
        [=](const NPartyJoinRequest& partyJoinRequest) {
            promise->set_value(partyJoinRequest);
        },
        [=](const NRtError& error) {
            promise->set_exception(std::make_exception_ptr<NRtException>(error));
        });

    return promise->get_future();
}

std::future<void> NRtClient::promotePartyMemberAsync(const std::string& partyId, NUserPresence& partyMember)
{
    promotePartyMember(partyId, partyMember);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<void> NRtClient::removeMatchmakerPartyAsync(const std::string& partyId, const std::string& ticket)
{
    removeMatchmakerParty(partyId, ticket);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<void> NRtClient::removePartyMemberAsync(const std::string& partyId, NUserPresence& presence)
{
    removePartyMember(partyId, presence);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::future<void> NRtClient::sendPartyDataAsync(const std::string& partyId, long opCode, NBytes& data)
{
    sendPartyData(partyId, opCode, data);
    auto promise = std::make_shared<std::promise<void>>();
    promise->set_value();
    return promise->get_future();
}

std::shared_ptr<RtRequestContext> NRtClient::createReqContext(::nakama::realtime::Envelope& msg)
{
    std::lock_guard<std::mutex> lock(_reqContextsLock);

    if (_reqContexts.empty() && _nextCid > 9)
    {
        // reset just to be one digit
        // we can reset because there are no pending requests
        _nextCid = 0;
    }

    decltype(_reqContexts)::iterator it;
    int32_t cid = 0;
    bool inserted = false;

    int numTries = 10;
    while(!inserted && --numTries > 0) {
        cid = _nextCid++;
        std::tie(it, inserted) = _reqContexts.emplace(cid, std::make_shared<RtRequestContext>());
        if (!inserted) {
            NLOG(NLogLevel::Error, "Creating request with already assigned CID=%d, please report this bug to Heroic Labs", cid);
        }
    }
    msg.set_cid(std::to_string(cid));

    //it is safe to return raw pointer from unique_ptr because only way entry
    // can be removed from _reqContexts is after request was sent, but all callers
    // don't keep raw pointer we return beyond send();

    // For the same reason it is safe to return it without holding _reqContextsLock
    return it->second;
}

void NRtClient::reqInternalError(int32_t cid, const NRtError & error)
{
    NLOG_ERROR(toString(error));

    std::shared_ptr<RtRequestContext> ctx;
    {
        std::lock_guard<std::mutex> lock(_reqContextsLock);
        auto it = _reqContexts.find(cid);
        if (it != _reqContexts.end())
        {
            ctx.swap(it->second);
            _reqContexts.erase(it);
        }
        else
        {
            NLOG(NLogLevel::Error, "request context not found. cid: %d", cid);
            if (_listener)
            {
                _listener->onError(error);
            }
            return;
        }
    }

    if (ctx && ctx->errorCallback)
    {
        ctx->errorCallback(error);
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

void NRtClient::cancelAllRequests(RtErrorCode code)
{
    NRtError err(code, "");
    std::lock_guard<std::mutex> lock(_reqContextsLock);
    for (auto& r: _reqContexts) {
        if (r.second->errorCallback)
        {
            r.second->errorCallback(err);
        }
    }
    _reqContexts.clear();

}

void NRtClient::heartbeat()
{
    if (!_heartbeatIntervalMs.has_value()) { return; }

    NTimestamp now = getUnixTimestampMs();
    auto heartbeatIntervalMs = _heartbeatIntervalMs.value();

    // _lastHeartbeatTs is set only when we are waiting for a heartbeat response
    if (_lastHeartbeatTs && _lastHeartbeatTs + heartbeatIntervalMs < now) {
        if (_heartbeatFailureReported) { return; }
        NRtClientDisconnectInfo info{ NRtClientDisconnectInfo::Code::HEARTBEAT_FAILURE, "Heartbeat failure", false};
        disconnect(info);

        _lastHeartbeatTs = 0;
        _heartbeatFailureReported = true;
    }

    // Nothing to worry about
    if (_wantDisconnect || !_transport->isConnected()) { return; }

    // ping messages might not go through if there is active bulk data transfer
    // We don't know if transport is currently sending/receiving data, so instead we rely
    // on outstaning requests to indicate if it might be happening.
    {
        std::lock_guard<std::mutex> lock(_reqContextsLock);
        if (!_reqContexts.empty()) { return; }
    }

    // not yet
    if (_lastMessageTs + heartbeatIntervalMs > now) { return; }

    auto pingSuccess = [this]() {
        _lastHeartbeatTs = 0;
    };

    NLOG_DEBUG("Sending heartbeat");

    // We don't trigger heartbeat failure in ping's errback, because:
    // - transport error on send will trigger disconnect anyway
    // - even if there was a server side error for pings,the fact that server
    //   responded at all means we shouldn't fail heartbeat
    ping(pingSuccess);
    _lastHeartbeatTs = now;
}

void NRtClient::send(const ::nakama::realtime::Envelope & msg)
{
    int cid = -1;
    if (msg.cid() != "")
    {
        cid = std::stoi(msg.cid());
    }

    if (!_wantDisconnect && isConnected())
    {
        NBytes bytes;

        if (_protocol->serialize(msg, bytes))
        {
            if (!_transport->send(bytes))
            {
                reqInternalError(cid, NRtError(RtErrorCode::TRANSPORT_ERROR, "Send message failed"));
                _transport->disconnect();
            }
            _lastMessageTs = getUnixTimestampMs();
        }
        else
        {
            reqInternalError(cid, NRtError(RtErrorCode::TRANSPORT_ERROR, "Serialize message failed"));
        }
    }
    else
    {
        reqInternalError(cid, NRtError(RtErrorCode::CONNECT_ERROR, "Not connected"));
    }
}

}
