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

#include "nakama-c/realtime/NRtClient.h"
#include "nakama-c/DataHelperC.h"
#include "nakama-cpp/realtime/NRtClientInterface.h"

NAKAMA_NAMESPACE_BEGIN

NSessionPtr getSession(::NSession session);

static std::vector<Nakama::NRtClientPtr> g_clients;

::NRtClient saveRtClient(NRtClientPtr client)
{
    g_clients.push_back(client);
    return client.get();
}

void removeRtClient(::NRtClient client)
{
    if (client)
    {
        for (auto it = g_clients.begin(); it != g_clients.end(); ++it)
        {
            if (it->get() == client)
            {
                g_clients.erase(it);
                break;
            }
        }
    }
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

//void NRtClient_setListener(NRtClient client, NRtClientListenerInterface* listener);

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

}

void NRtClient_writeChatMessage(
    NRtClient client,
    const char* channelId,
    const char* content,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNChannelMessageAck*),
    NRtClientErrorCallback errorCallback)
{

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

}

void NRtClient_removeChatMessage(
    NRtClient client,
    const char* channelId,
    const char* messageId,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNChannelMessageAck*),
    NRtClientErrorCallback errorCallback)
{

}

void NRtClient_createMatch(
    NRtClient client,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNMatch*),
    NRtClientErrorCallback errorCallback)
{

}

void NRtClient_joinMatch(
    NRtClient client,
    const char* matchId,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNMatch*),
    NRtClientErrorCallback errorCallback)
{

}

void NRtClient_joinMatchByToken(
    NRtClient client,
    const char* token,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNMatch*),
    NRtClientErrorCallback errorCallback)
{

}

void NRtClient_leaveMatch(
    NRtClient client,
    const char* matchId,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData),
    NRtClientErrorCallback errorCallback)
{

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

}

void NRtClient_removeMatchmaker(
    NRtClient client,
    const char* ticket,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData),
    NRtClientErrorCallback errorCallback)
{

}

void NRtClient_sendMatchData(
    NRtClient client,
    const char* matchId,
    int64_t opCode,
    const sNBytes* data,
    const sNUserPresence* presences,
    uint16_t presencesCount)
{

}

void NRtClient_followUsers(
    NRtClient client,
    const char** userIds,
    uint16_t userIdsCount,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNStatus*),
    NRtClientErrorCallback errorCallback)
{

}

void NRtClient_unfollowUsers(
    NRtClient client,
    const char** userIds,
    uint16_t userIdsCount,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData),
    NRtClientErrorCallback errorCallback)
{

}

void NRtClient_updateStatus(
    NRtClient client,
    const char* status,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData),
    NRtClientErrorCallback errorCallback)
{

}

void NRtClient_rpc(
    NRtClient client,
    const char* id,
    const char* payload,
    NRtClientReqData reqData,
    void (*successCallback)(NRtClient, NRtClientReqData, const sNRpc*),
    NRtClientErrorCallback errorCallback)
{

}

void NRtClient_destroy(NRtClient client)
{
    Nakama::removeRtClient(client);
}

} // extern "C"
