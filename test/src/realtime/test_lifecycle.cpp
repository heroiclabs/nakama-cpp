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
#include "NTest.h"
#include "globals.h"
#include "TestGuid.h"

namespace Nakama {
    namespace Test {

        void test_rt_quickdestroy()
        {
            bool threadedTick = true;
            NTest test(__func__, threadedTick);

            test.listener.setErrorCallback([&test](const NRtError&) {
                test.stopTest(false);
            });

            test.listener.setConnectCallback([&test]() {
                test.rtClient->disconnect();
                test.stopTest(true);
            });

            NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid()).get();
            bool createStatus = false;

            test.rtClient->connectAsync(session, createStatus).get();

            test.waitUntilStop();
        }

        void test_rt_rapiddisconnect()
        {
            bool threadedTick = true;
            NTest test(__func__, threadedTick);
            test.setTestTimeoutMs(1000);
            bool hadConnectCallback = false;

            NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid()).get();
            bool createStatus = false;

            test.rtClient->connectAsync(session, createStatus).get();
            test.rtClient->disconnect();

            test.listener.setConnectCallback([&hadConnectCallback]() {
                NLOG_INFO("TEST: Connect");
                hadConnectCallback = true;
            });
            test.listener.setDisconnectCallback([&test, &hadConnectCallback](const NRtClientDisconnectInfo& /*info*/) {
                NLOG_INFO("TEST: Disconnect");
                // both connect and disconnect callback fired and disconnect fired after connect
                test.stopTest(hadConnectCallback);
            });

            test.rtClient->connect(session, true);

            test.waitUntilStop();
        }

        void test_rt_reconnect()
        {
            bool threadedTick = true;
            NTest test(__func__, threadedTick);
            test.setTestTimeoutMs(1000);
            bool hadConnectCallback = false;
            test.runTest();

            NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid()).get();
            bool createStatus = false;

            test.rtClient->connectAsync(session, createStatus).get();

            bool connectedOnce = false;
            test.listener.setDisconnectCallback([&test, &session](const NRtClientDisconnectInfo& /*info*/){
                NLOG_INFO("TEST: disconnected, connecting again");
                test.rtClient->connect(session, true);
            });

            test.rtClient->disconnect();

            if (connectedOnce) {
                NLOG_INFO("TEST: Stopping test with success");
                test.stopTest(true); // connect after disconnect succeeded
            } else {
                connectedOnce = true;
            }

            test.waitUntilStop();
        }

        void test_rt_heartbeat()
        {
            bool threadedTick = true;
            NTest test(__func__, threadedTick);
            test.setTestTimeoutMs(1000);
            bool hadConnectCallback = false;
            test.runTest();

            NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid()).get();
            bool createStatus = false;

            test.setTestTimeoutMs(100000);
            int i = 0;

            // Fail heartbeat immediately on the second tick
            // Adjusting heartbeat timeout like below is not really representative case of a network failue,
            // because server sends close frame and it gets delivered. To test it for real, comment line below
            // and use `pkill -STOP nakama` (linux) or sysinternal process explorer to suspend nakama process
            // making it unresponsive. You'll need to "unfreeze" it again with `pkill -CONT` mid test, so that
            // it can connect again.
            test.rtClient->setHeartbeatIntervalMs(0);
            test.listener.setDisconnectCallback([&test, &i, &session](const NRtClientDisconnectInfo& info){
                if (info.code != NRtClientDisconnectInfo::Code::HEARTBEAT_FAILURE) {
                    NLOG_INFO("Unexpected heartbeat failure disconnect code, wanted=4000 , got=" + std::to_string(info.code));
                    test.stopTest(false);
                }

                if (i++ > 0) {
                    test.stopTest(true);
                    return;
                }
                // test that heartbeats continue to work after reconnect
                test.rtClient->connect(session, true);
            });

            test.waitUntilStop();
        }

        // optional test -- pass --socket.pong_wait_ms 5000 to Nakama so that it disconnects rtclient after sleep
        // in order to test how it will react.
        void test_rt_remote_disconnect()
        {
            bool threadedTick = true;
            NTest test(__func__, threadedTick);
            test.setTestTimeoutMs(15000);
            bool hadConnectCallback = false;
            test.runTest();

            NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid()).get();
            bool createStatus = false;

            test.rtClient->createMatchAsync().get();
            test.setRtTickPaused(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));

            test.setRtTickPaused(false);

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            test.stopTest(true);
        }
    }
}
