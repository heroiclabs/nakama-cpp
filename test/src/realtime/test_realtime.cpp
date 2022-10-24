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
void test_rt_party();

void test_rt_quickdestroy()
{
    NRtClientTest test(__func__);
    test.runTest([&test]() {
        test.listener.setErrorCallback([&test](const NRtError&) {
            test.stopTest(false);
        });

        test.listener.setConnectCallback([&test]() {
            test.rtClient->disconnect();
            test.stopTest(true);
        });
        test.rtClient->connect(test.session, true, test.protocol);
    });
}

void test_rt_rapiddisconnect()
{
    NRtClientTest test(__func__);
    test.setTestTimeoutMs(1000);
    bool hadConnectCallback = false;

    test.runTest([&test, &hadConnectCallback]() {

        // if no callbacks were fired, it is a success
        test.onTimeoutCb = []() { return true; };

        test.listener.setConnectCallback([&hadConnectCallback]() {
            std::cout << "TEST: Connect" << std::endl;
            hadConnectCallback = true;
        });
        test.listener.setDisconnectCallback([&test, &hadConnectCallback](const NRtClientDisconnectInfo& /*info*/) {
            std::cout << "TEST: Disconnect" << std::endl;
            // both connect and disconnect callback fired and disconnect fired after connect
            test.stopTest(hadConnectCallback);
        });
        test.rtClient->connect(test.session, true, test.protocol);
        test.rtClient->disconnect();
    });
}

void test_rt_reconnect()
{
    NRtClientTest test(__func__);
    bool connectedOnce = false;
    test.onRtConnect = [&test, &connectedOnce](){
        std::cout << "TEST: connected" << std::endl;
        test.listener.setDisconnectCallback([&test](const NRtClientDisconnectInfo& /*info*/){
            std::cout << "TEST: disconnected, connecting again" << std::endl;
            test.rtClient->connect(test.session, true, test.protocol);
        });
        test.rtClient->disconnect();
        if(connectedOnce) {
            std::cout << "TEST: Stopping test with success" << std::endl;
            test.stopTest(true); // connect after disconnect succeeded
        } else {
            connectedOnce = true;
        }
    };
    test.runTest();
}

void test_rt_heartbeat()
{
    NRtClientTest test(__func__);
    test.setTestTimeoutMs(100000);
    int i = 0;
    test.onRtConnect = [&test, &i](){
        std::cout << "CONNECTED" << std::endl;
        // Fail heartbeat immediately on the second tick
        // Adjusting heartbeat timeout like below is not really representative case of a network failue,
        // because server sends close frame and it gets delivered. To test it for real, comment line below
        // and use `pkill -STOP nakama` (linux) or sysinternal process explorer to suspend nakama process
        // making it unresponsive. You'll need to "unfreeze" it again with `pkill -CONT` mid test, so that
        // it can connect again.
        test.rtClient->setHeartbeatIntervalMs(0);
        test.listener.setDisconnectCallback([&test, &i](const NRtClientDisconnectInfo& info){
            if (info.code != NRtClientDisconnectInfo::Code::HEARTBEAT_FAILURE) {
                std::cout << "Unexpected heartbeat failure disconnect code, wanted=4000 , got=" << info.code << std::endl;
                test.stopTest(false);
            }

            if (i++ > 0) {
                test.stopTest(true);
                return;
            }
            // test that heartbeats continue to work after reconnect
            test.rtClient->connect(test.session, true, test.protocol);
        });
    };
    test.runTest();
}

void test_rt_remote_disconnect()
{
    NRtClientTest test(__func__);
    // test what
    test.setRtStopTestOnDisconnect(false);
    test.setTestTimeoutMs(15000);
    test.onRtConnect = [&test]()
    {
        test.rtClient->createMatch([&test](const Nakama::NMatch& match)
        {
            std::cout << "created match" << std::endl;
            std::cout << "about to sleep" << std::endl;
            test.setRtTickPaused(true);
            sleep(10000);
            test.setRtTickPaused(false);
            std::cout << "done sleeping" << std::endl;

            test.rtClient->leaveMatch(match.matchId, [&test](){
                test.stopTest(true);
            });
        });
    };

    test.runTest();
}


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

        auto successCallback = [&test](NGroup group)
        {
            std::cout << "joined group: " << group.id << std::endl;

            auto joinedChatSucceeded = [&test](NChannelPtr /*channel*/)
            {
                std::cout << "Group chat joined successfully." << std::endl;
                test.stopTest(true);
            };

            auto joinedChatFailed = [&test](const NRtError& err)
            {
                std::cout << "Could not join group chat: " << toString(err) << std::endl;
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

        test.client->createGroup(test.session, "group chat " + test.session->getAuthToken(), "a group for chatting", "", "", false, {}, successCallback);
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
    test_rt_party();
}

void test_realtime()
{
    test_rt_remote_disconnect();
    // These tests are not protocol specific
    test_rt_quickdestroy();
    test_rt_rapiddisconnect();
    // change to 10 iterations to trigger https://github.com/microsoft/libHttpClient/issues/698 bug
    for(int i = 0; i < 1; i++) {
        test_rt_reconnect();
    }
    test_rt_heartbeat();

    run_realtime_tests();

    NRtClientTest::protocol = NRtClientProtocol::Json;
    std::cout << std::endl << "using Json protocol" << std::endl;

    run_realtime_tests();
}

} // namespace Test
} // namespace Nakama
