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

#ifdef BUILD_C_API

#include "nakama-c/realtime/NRtClient.h"
#include "nakama-c/DataHelperC.h"
#include "nakama-cpp/realtime/NRtClientInterface.h"
#include "nakama-cpp/realtime/NRtDefaultClientListener.h"

NAKAMA_NAMESPACE_BEGIN

NSessionPtr getSession(::NSession session);

struct RtClientData
{
    NRtClientPtr client;
    std::unique_ptr<NRtDefaultClientListener> listener;
};

static std::vector<RtClientData> g_clients;

::NRtClient saveRtClient(NRtClientPtr client)
{
    NRtDefaultClientListener* listener = new NRtDefaultClientListener();
    g_clients.push_back({ client, std::unique_ptr<NRtDefaultClientListener>(listener) });
    client->setListener(listener);
    return (::NRtClient)client.get();
}

void removeRtClient(::NRtClient client)
{
    if (client)
    {
        for (auto it = g_clients.begin(); it != g_clients.end(); ++it)
        {
            if ((::NRtClient)it->client.get() == client)
            {
                g_clients.erase(it);
                break;
            }
        }
    }
}

Nakama::NRtDefaultClientListener* getRtClientListener(::NRtClient client)
{
    for (auto it = g_clients.begin(); it != g_clients.end(); ++it)
    {
        if ((::NRtClient)it->client.get() == client)
        {
            return it->listener.get();
        }
    }

    return nullptr;
}

std::function<void()> createOkEmptyCallback(NRtClient client, NRtClientReqData reqData, void (*successCallback)(NRtClient, NRtClientReqData))
{
    if (!successCallback)
        return nullptr;

    return [client, reqData, successCallback]()
    {
        successCallback(client, reqData);
    };
}

Nakama::RtErrorCallback createErrorCallback(NRtClient client, NRtClientReqData reqData, ::NRtClientErrorCallback errorCallback)
{
    if (!errorCallback)
        return nullptr;

    return [client, reqData, errorCallback](const Nakama::NRtError& error)
    {
        ::sNRtError cError;
        assign(cError, error);
        errorCallback(client, reqData, &cError);
        Nakama::sNRtError_free(cError);
    };
}

NAKAMA_NAMESPACE_END

extern "C" {

Nakama::NRtClientInterface* getCppRtClient(NRtClient client)
{
    return (Nakama::NRtClientInterface*)client;
}

void NRtClient_tick(NRtClient client)
{
    getCppRtClient(client)->tick();
}

void NRtClient_setUserData(NRtClient client, void* userData)
{
    getCppRtClient(client)->setUserData(userData);
}

void* NRtClient_getUserData(NRtClient client)
{
    return getCppRtClient(client)->getUserData();
}

void NRtClient_connect(NRtClient client, NSession session, bool createStatus, eNRtClientProtocol protocol)
{
    auto cppSession = Nakama::getSession(session);

    getCppRtClient(client)->connect(cppSession, createStatus, (Nakama::NRtClientProtocol)protocol);
}

bool NRtClient_isConnected(NRtClient client)
{
    return getCppRtClient(client)->isConnected();
}

void NRtClient_disconnect(NRtClient client)
{
    getCppRtClient(client)->disconnect();
}

//NRtTransportPtr NRtClient_getTransport(NRtClient client);

void NRtClient_joinChat(
    NRtClient client,
    const char* target,
    eNChannelType type,
    bool persistence,
    bool hidden,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNChannel*),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->joinChat(
        target,
        (Nakama::NChannelType)type,
        persistence,
        hidden,
        [client, successCallback, reqData](const Nakama::NChannelPtr& channel)
        {
            if (successCallback)
            {
                sNChannel cChannel;
                assign(cChannel, *channel);
                successCallback(client, reqData, &cChannel);
                Nakama::sNChannel_free(cChannel);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_leaveChat(
    NRtClient client,
    const char* channelId,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->leaveChat(
        channelId,
        Nakama::createOkEmptyCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_writeChatMessage(
    NRtClient client,
    const char* channelId,
    const char* content,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNChannelMessageAck*),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->writeChatMessage(
        channelId,
        content,
        [client, successCallback, reqData](const Nakama::NChannelMessageAck& ack)
        {
            if (successCallback)
            {
                sNChannelMessageAck cAck;
                assign(cAck, ack);
                successCallback(client, reqData, &cAck);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_updateChatMessage(
    NRtClient client,
    const char* channelId,
    const char* messageId,
    const char* content,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNChannelMessageAck*),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->updateChatMessage(
        channelId,
        messageId,
        content,
        [client, successCallback, reqData](const Nakama::NChannelMessageAck& ack)
        {
            if (successCallback)
            {
                sNChannelMessageAck cAck;
                assign(cAck, ack);
                successCallback(client, reqData, &cAck);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_removeChatMessage(
    NRtClient client,
    const char* channelId,
    const char* messageId,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNChannelMessageAck*),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->removeChatMessage(
        channelId,
        messageId,
        [client, successCallback, reqData](const Nakama::NChannelMessageAck& ack)
        {
            if (successCallback)
            {
                sNChannelMessageAck cAck;
                assign(cAck, ack);
                successCallback(client, reqData, &cAck);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_createMatch(
    NRtClient client,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNMatch*),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->createMatch(
        [client, successCallback, reqData](const Nakama::NMatch& match)
        {
            if (successCallback)
            {
                sNMatch cMatch;
                assign(cMatch, match);
                successCallback(client, reqData, &cMatch);
                Nakama::sNMatch_free(cMatch);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_joinMatch(
    NRtClient client,
    const char* matchId,
    NStringMap metadata,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNMatch*),
    NRtClientErrorCallback errorCallback)
{
    Nakama::NStringMap cppMetadata;

    Nakama::assign(cppMetadata, metadata);

    getCppRtClient(client)->joinMatch(
        matchId,
        cppMetadata,
        [client, successCallback, reqData](const Nakama::NMatch& match)
        {
            if (successCallback)
            {
                sNMatch cMatch;
                assign(cMatch, match);
                successCallback(client, reqData, &cMatch);
                Nakama::sNMatch_free(cMatch);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_joinMatchByToken(
    NRtClient client,
    const char* token,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNMatch*),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->joinMatchByToken(
        token,
        [client, successCallback, reqData](const Nakama::NMatch& match)
        {
            if (successCallback)
            {
                sNMatch cMatch;
                assign(cMatch, match);
                successCallback(client, reqData, &cMatch);
                Nakama::sNMatch_free(cMatch);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_leaveMatch(
    NRtClient client,
    const char* matchId,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->leaveMatch(
        matchId,
        Nakama::createOkEmptyCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_addMatchmaker(
    NRtClient client,
    int32_t minCount,
    int32_t maxCount,
    const char* query,
    NStringMap stringProperties,
    NStringDoubleMap numericProperties,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNMatchmakerTicket*),
    NRtClientErrorCallback errorCallback)
{
    Nakama::NStringMap cppStringProperties;
    Nakama::NStringDoubleMap cppNumericProperties;

    Nakama::assign(cppStringProperties, stringProperties);
    Nakama::assign(cppNumericProperties, numericProperties);

    getCppRtClient(client)->addMatchmaker(
        minCount ? Nakama::opt::optional<int32_t>(minCount) : Nakama::opt::nullopt,
        maxCount ? Nakama::opt::optional<int32_t>(maxCount) : Nakama::opt::nullopt,
        query ? Nakama::opt::optional<std::string>(query) : Nakama::opt::nullopt,
        cppStringProperties,
        cppNumericProperties,
        [client, successCallback, reqData](const Nakama::NMatchmakerTicket& ticket)
        {
            if (successCallback)
            {
                sNMatchmakerTicket cTicket;
                assign(cTicket, ticket);
                successCallback(client, reqData, &cTicket);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_removeMatchmaker(
    NRtClient client,
    const char* ticket,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->removeMatchmaker(
        ticket,
        Nakama::createOkEmptyCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_sendMatchData(
    NRtClient client,
    const char* matchId,
    int64_t opCode,
    const sNBytes* data,
    const sNUserPresence* presences,
    uint16_t presencesCount)
{
    Nakama::NBytes cppData;
    std::vector<Nakama::NUserPresence> cppPresences;

    Nakama::assign(cppData, data);
    Nakama::assign(cppPresences, presences, presencesCount);

    getCppRtClient(client)->sendMatchData(
        matchId,
        opCode,
        cppData,
        cppPresences);
}

void NRtClient_followUsers(
    NRtClient client,
    const char** userIds,
    uint16_t userIdsCount,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNStatus*),
    NRtClientErrorCallback errorCallback)
{
    std::vector<std::string> cppUserIds;

    Nakama::assign(cppUserIds, userIds, userIdsCount);

    getCppRtClient(client)->followUsers(
        cppUserIds,
        [client, successCallback, reqData](const Nakama::NStatus& status)
        {
            if (successCallback)
            {
                sNStatus cStatus;
                assign(cStatus, status);
                successCallback(client, reqData, &cStatus);
                Nakama::sNStatus_free(cStatus);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_unfollowUsers(
    NRtClient client,
    const char** userIds,
    uint16_t userIdsCount,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData),
    NRtClientErrorCallback errorCallback)
{
    std::vector<std::string> cppUserIds;

    Nakama::assign(cppUserIds, userIds, userIdsCount);

    getCppRtClient(client)->unfollowUsers(
        cppUserIds,
        Nakama::createOkEmptyCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_updateStatus(
    NRtClient client,
    const char* status,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->updateStatus(
        status,
        Nakama::createOkEmptyCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_rpc(
    NRtClient client,
    const char* id,
    const char* payload,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNRpc*),
    NRtClientErrorCallback errorCallback)
{
    getCppRtClient(client)->rpc(
        id,
        payload ? Nakama::opt::optional<std::string>(payload) : Nakama::opt::nullopt,
        [client, successCallback, reqData](const Nakama::NRpc& rpc)
        {
            if (successCallback)
            {
                sNRpc cRpc;
                assign(cRpc, rpc);
                successCallback(client, reqData, &cRpc);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NRtClient_setConnectCallback(NRtClient client, void (*callback)(NRtClient))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setConnectCallback([client, callback]()
        {
            callback(client);
        });
    }
    else
        listener->setConnectCallback(nullptr);
}

void NRtClient_setDisconnectCallback(NRtClient client, void (*callback)(NRtClient, const sNRtClientDisconnectInfo*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setDisconnectCallback([client, callback](const Nakama::NRtClientDisconnectInfo& info)
        {
            sNRtClientDisconnectInfo cInfo;
            assign(cInfo, info);
            callback(client , &cInfo);
        });
    }
    else
        listener->setDisconnectCallback(nullptr);
}

void NRtClient_setErrorCallback(NRtClient client, void (*callback)(NRtClient, const sNRtError*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setErrorCallback([client, callback](const Nakama::NRtError& error)
        {
            sNRtError cError;
            assign(cError, error);
            callback(client, &cError);
            Nakama::sNRtError_free(cError);
        });
    }
    else
        listener->setErrorCallback(nullptr);
}

void NRtClient_setChannelMessageCallback(NRtClient client, void (*callback)(NRtClient, const sNChannelMessage*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setChannelMessageCallback([client, callback](const Nakama::NChannelMessage& msg)
        {
            sNChannelMessage cMsg;
            assign(cMsg, msg);
            callback(client, &cMsg);
        });
    }
    else
        listener->setChannelMessageCallback(nullptr);
}

void NRtClient_setChannelPresenceCallback(NRtClient client, void (*callback)(NRtClient, const sNChannelPresenceEvent*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setChannelPresenceCallback([client, callback](const Nakama::NChannelPresenceEvent& presence)
        {
            sNChannelPresenceEvent cPresence;
            Nakama::assign(cPresence, presence);
            callback(client, &cPresence);
            Nakama::sNChannelPresenceEvent_free(cPresence);
        });
    }
    else
        listener->setChannelPresenceCallback(nullptr);
}

void NRtClient_setMatchmakerMatchedCallback(NRtClient client, void (*callback)(NRtClient, const sNMatchmakerMatched*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setMatchmakerMatchedCallback([client, callback](const Nakama::NMatchmakerMatchedPtr& matched)
        {
            sNMatchmakerMatched cMatched;
            Nakama::assign(cMatched, *matched);
            callback(client, &cMatched);
            Nakama::sNMatchmakerMatched_free(cMatched);
        });
    }
    else
        listener->setMatchmakerMatchedCallback(nullptr);
}

void NRtClient_setMatchDataCallback(NRtClient client, void (*callback)(NRtClient, const sNMatchData*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setMatchDataCallback([client, callback](const Nakama::NMatchData& data)
        {
            sNMatchData cData;
            assign(cData, data);
            callback(client, &cData);
        });
    }
    else
        listener->setMatchDataCallback(nullptr);
}

void NRtClient_setMatchPresenceCallback(NRtClient client, void (*callback)(NRtClient, const sNMatchPresenceEvent*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setMatchPresenceCallback([client, callback](const Nakama::NMatchPresenceEvent& event)
        {
            sNMatchPresenceEvent cEvent;
            assign(cEvent, event);
            callback(client, &cEvent);
            Nakama::sNMatchPresenceEvent_free(cEvent);
        });
    }
    else
        listener->setMatchPresenceCallback(nullptr);
}

void NRtClient_setNotificationsCallback(NRtClient client, void (*callback)(NRtClient, const sNNotificationList*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setNotificationsCallback([client, callback](const Nakama::NNotificationList& list)
        {
            sNNotificationList cList;
            assign(cList, list);
            callback(client, &cList);
            Nakama::sNNotificationList_free(cList);
        });
    }
    else
        listener->setNotificationsCallback(nullptr);
}

void NRtClient_setStatusPresenceCallback(NRtClient client, void (*callback)(NRtClient, const sNStatusPresenceEvent*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setStatusPresenceCallback([client, callback](const Nakama::NStatusPresenceEvent& event)
        {
            sNStatusPresenceEvent cEvent;
            assign(cEvent, event);
            callback(client, &cEvent);
            Nakama::sNStatusPresenceEvent_free(cEvent);
        });
    }
    else
        listener->setStatusPresenceCallback(nullptr);
}

void NRtClient_setStreamPresenceCallback(NRtClient client, void (*callback)(NRtClient, const sNStreamPresenceEvent*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setStreamPresenceCallback([client, callback](const Nakama::NStreamPresenceEvent& event)
        {
            sNStreamPresenceEvent cEvent;
            assign(cEvent, event);
            callback(client, &cEvent);
            Nakama::sNStreamPresenceEvent_free(cEvent);
        });
    }
    else
        listener->setStreamPresenceCallback(nullptr);
}

void NRtClient_setStreamDataCallback(NRtClient client, void (*callback)(NRtClient, const sNStreamData*))
{
    auto listener = Nakama::getRtClientListener(client);

    if (callback)
    {
        listener->setStreamDataCallback([client, callback](const Nakama::NStreamData& data)
        {
            sNStreamData cData;
            assign(cData, data);
            callback(client, &cData);
        });
    }
    else
        listener->setStreamDataCallback(nullptr);
}

void NRtClient_destroy(NRtClient client)
{
    Nakama::removeRtClient(client);
}

} // extern "C"

#endif // BUILD_C_API
