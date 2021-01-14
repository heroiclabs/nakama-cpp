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

#include "realtime/RtClientTestBase.h"
#include <thread>

namespace Nakama {
namespace Test {

using namespace std;

void test_rt_match();
void test_notifications();
void test_authoritative_match();
void test_tournament();
void test_rpc();

void test_rt_joinChat()
{
    NRtClientTest test(__func__);

    thread::id main_thread_id = this_thread::get_id();

    test.onRtConnect = [&test, &main_thread_id]()
    {
        if (main_thread_id != this_thread::get_id())
        {
            std::cout << "ERROR: onRtConnect executed not from main thread!" << std::endl;
            test.stopTest();
            return;
        }

        auto successCallback = [&test, &main_thread_id](NChannelPtr channel)
        {
            std::cout << "joined chat: " << channel->id << std::endl;

            if (main_thread_id != this_thread::get_id())
            {
                std::cout << "ERROR: successCallback executed not from main thread!" << std::endl;
                test.stopTest();
                return;
            }

            auto ackCallback = [&test](const NChannelMessageAck& ack)
            {
                std::cout << "message sent successfuly. msg id: " << ack.messageId << std::endl;
                test.stopTest(true);
            };

            // data must be JSON
            std::string json_data = "{\"msg\":\"Hello there!\"}";

            test.rtClient->writeChatMessage(
                channel->id,
                json_data,
                ackCallback
            );
        };

        test.rtClient->joinChat(
            "chat",
            NChannelType::ROOM,
            {},
            {},
            successCallback
        );
    };

    test.runTest();
}

void test_rt_joinGroupChat()
{
    NRtClientTest test(__func__);

    thread::id main_thread_id = this_thread::get_id();

    test.onRtConnect = [&test, &main_thread_id]()
    {
        if (main_thread_id != this_thread::get_id())
        {
            std::cout << "ERROR: onRtConnect executed not from main thread!" << std::endl;
            test.stopTest();
            return;
        }

        auto successCallback = [&test, &main_thread_id](NGroup group)
        {
            std::cout << "joined group: " << group.id << std::endl;

            auto joinedChatSucceeded = [&test](NChannelPtr channel)
            {
                std::cout << "Group chat joined successfully." << std::endl;
                test.stopTest(true);
            };

            auto joinedChatFailed = [&test](const NRtError& err)
            {
                std::cout << "Could not join group chat." << std::endl;
                test.stopTest(false);
            };

            test.rtClient->joinChat(
                group.id,
                NChannelType::GROUP,
                {},
                {},
                joinedChatSucceeded,
                joinedChatFailed
            );
        };

        test.client->createGroup(test.session, "test group", "a group for chatting", "", "", false, {}, successCallback);
    };

    test.runTest();
}

void run_realtime_tests()
{
    test_rt_joinChat();
    test_rt_joinGroupChat();
    test_rt_match();
    test_notifications();
    test_authoritative_match();
    test_tournament();
    test_rpc();
}

void test_realtime()
{
    run_realtime_tests();

    NRtClientTest::protocol = NRtClientProtocol::Json;
    std::cout << std::endl << "using Json protocol" << std::endl;

    run_realtime_tests();
}

} // namespace Test
} // namespace Nakama
