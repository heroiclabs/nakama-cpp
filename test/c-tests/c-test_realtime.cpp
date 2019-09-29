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

#include "c-test.h"
#include "test_serverConfig.h"

namespace Nakama {
namespace Test {

using namespace std;

static void channelMessageCallback(NRtClient, const sNChannelMessage* message)
{
    std::cout << "channel message received: " << message->content << std::endl;
    stopCTest(getCurCTest()->client, true);
}

static void writeChatMessageCallback(NRtClient client, NRtClientReqData, const sNChannelMessageAck* ack)
{
    std::cout << "message sent successfuly. msg id: " << ack->messageId << std::endl;
}

static void joinChatCallback(NRtClient client, NRtClientReqData, const sNChannel* channel)
{
    std::cout << "joined chat: " << channel->id << std::endl;

    NRtClient_setChannelMessageCallback(client, channelMessageCallback);

    // data must be JSON
    const char* json_data = "{\"msg\":\"Hello there!\"}";

    NRtClient_writeChatMessage(client,
        channel->id,
        json_data,
        0,
        writeChatMessageCallback,
        NULL
    );
}

static void connectCallback(NRtClient client)
{
    std::cout << "connected" << std::endl;

    NRtClient_joinChat(client, "my-chat", NChannelType_ROOM, false, true, 0, joinChatCallback, NULL);
}

static void successCallback(NClient client, NClientReqData reqData, NSession session)
{
    const char* token = NSession_getAuthToken(session);

    std::cout << "session token: " << token << std::endl;

    NRtClient rtClient = NClient_createRtClient(client, SERVER_HTTP_PORT);
    getCurCTest()->rtClient = rtClient;

    NRtClient_setConnectCallback(rtClient, connectCallback);

    NRtClient_connect(rtClient, session, true, NRtClientProtocol_Protobuf);

    NSession_destroy(session);
}

void ctest_realtime_joinChat()
{
    CTest test(__func__);

    test.createWorkingClient();

    NClient_authenticateDevice(test.client,
        "mytestdevice0000",
        nullptr, // username
        true,    // create
        nullptr,
        nullptr,
        successCallback,
        nullptr
    );

    test.runTest();
    
    NRtClient_destroy(getCurCTest()->rtClient);
    destroyNakamaClient(getCurCTest()->client);
}

void ctest_realtime()
{
    ctest_realtime_joinChat();
}

} // namespace Test
} // namespace Nakama
