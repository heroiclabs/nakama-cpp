/*
 * Copyright 2023 The Nakama Authors
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

#include "nakama-cpp/log/NLogger.h"
#include "NTest.h"
#include "TestGuid.h"

namespace Nakama {
    namespace Test {

        void test_rt_joinChat()
        {
            bool threadedTick = true;
            NTest test(__func__, threadedTick);
            test.runTest();

            NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
            bool createStatus = false;
            test.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();

            // data must be JSON
            std::string json_data = "{\"msg\":\"Hello there!\"}";

            const NChannelPtr channel = test.rtClient->joinChatAsync("chat", NChannelType::ROOM, {}, {}).get();
            const NChannelMessageAck ack = test.rtClient->writeChatMessageAsync(channel->id, json_data).get();

            NLOG_INFO("call stop test");
            test.stopTest(ack.channelId == channel->id);
            NLOG_INFO("done call stop test");
        }

        void test_rt_joinGroupChat()
        {
            NLOG_INFO("calling join group chat");

            bool threadedTick = true;
            NTest test(__func__, threadedTick);
            test.runTest();
            NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
            bool createStatus = false;
            test.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();
            const NGroup group = test.client->createGroupAsync(session, "group chat " + session->getAuthToken(), "a group for chatting", "", "", false, {}).get();
            const NChannelPtr channelPtr = test.rtClient->joinChatAsync(group.id, NChannelType::GROUP, {}, {}).get();
            test.stopTest(true);
        }
    }
}
