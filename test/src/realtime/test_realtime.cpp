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

#include <thread>
#include "realtime/RtClientTest.h"
#include "globals.h"

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
            NLOG_INFO("TEST: Connect");
            hadConnectCallback = true;
        });
        test.listener.setDisconnectCallback([&test, &hadConnectCallback](const NRtClientDisconnectInfo& /*info*/) {
            NLOG_INFO("TEST: Disconnect");
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
        NLOG_INFO("TEST: connected");
        test.listener.setDisconnectCallback([&test](const NRtClientDisconnectInfo& /*info*/){
            NLOG_INFO("TEST: disconnected, connecting again");
            test.rtClient->connect(test.session, true, test.protocol);
        });
        test.rtClient->disconnect();
        if(connectedOnce) {
            NLOG_INFO("TEST: Stopping test with success");
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
    test.setRtStopTestOnDisconnect(false);
    test.setTestTimeoutMs(100000);
    int i = 0;
    test.onRtConnect = [&test, &i](){
        NLOG_INFO("CONNECTED");
        // Fail heartbeat immediately on the second tick
        // Adjusting heartbeat timeout like below is not really representative case of a network failue,
        // because server sends close frame and it gets delivered. To test it for real, comment line below
        // and use `pkill -STOP nakama` (linux) or sysinternal process explorer to suspend nakama process
        // making it unresponsive. You'll need to "unfreeze" it again with `pkill -CONT` mid test, so that
        // it can connect again.
        test.rtClient->setHeartbeatIntervalMs(0);
        test.listener.setDisconnectCallback([&test, &i](const NRtClientDisconnectInfo& info){
            if (info.code != NRtClientDisconnectInfo::Code::HEARTBEAT_FAILURE) {
                NLOG_INFO("Unexpected heartbeat failure disconnect code, wanted=4000 , got=" + std::to_string(info.code));
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

// optional test -- pass --socket.pong_wait_ms 5000 to Nakama so that it disconnects rtclient after sleep
// in order to test how it will react.
void test_rt_remote_disconnect()
{
    NRtClientTest test(__func__);
    // we want to test behavior of socket after disconnect.
    test.setRtStopTestOnDisconnect(false);
    test.setTestTimeoutMs(15000);
    test.onRtConnect = [&test]()
    {
        test.rtClient->createMatch([&test](const Nakama::NMatch& match)
        {
            test.setRtTickPaused(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));

            test.setRtTickPaused(false);

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            test.stopTest(true);
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
            NLOG_INFO("ERROR: onRtConnect executed not from main thread!");
            test.stopTest();
            return;
        }

        auto successCallback = [&test, &main_thread_id](NChannelPtr channel)
        {
            NLOG_INFO("joined chat: " + channel->id);

            if (main_thread_id != this_thread::get_id())
            {
                NLOG_INFO("ERROR: successCallback executed not from main thread!");
                test.stopTest();
                return;
            }

            auto ackCallback = [&test](const NChannelMessageAck& ack)
            {
                NLOG_INFO("message sent successfuly. msg id: " + ack.messageId);
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
            NLOG_INFO("ERROR: onRtConnect executed not from main thread!");
            test.stopTest();
            return;
        }

        auto successCallback = [&test](NGroup group)
        {
            NLOG_INFO("joined group: " + group.id);

            auto joinedChatSucceeded = [&test](NChannelPtr /*channel*/)
            {
                NLOG_INFO("Group chat joined successfully.");
                test.stopTest(true);
            };

            auto joinedChatFailed = [&test](const NRtError& err)
            {
                NLOG_INFO("Could not join group chat: " + toString(err));
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
//    test_rt_joinChat();
//    test_rt_joinGroupChat();
    test_rt_match();
//    test_notifications();
//    test_authoritative_match();
//    test_tournament();
//    test_rpc();
//    test_rt_party();
}

void test_realtime()
{
    // These tests are not protocol specific
//    test_rt_quickdestroy();
//    test_rt_rapiddisconnect();
//    //// change to 10 iterations to trigger https://github.com/microsoft/libHttpClient/issues/698 bug
//    for (int i = 0; i < 1; i++) {
//        test_rt_reconnect();
//    }
//    test_rt_heartbeat();

    run_realtime_tests();

    NRtClientTest::protocol = NRtClientProtocol::Json;
    NLOG_INFO("using Json protocol");

//    run_realtime_tests();
}

} // namespace Test
} // namespace Nakama
