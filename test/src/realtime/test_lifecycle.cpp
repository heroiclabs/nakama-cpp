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

        void test_rt_rapiddisconnect()
        {
            bool threadedTick = true;
            NTest test(__func__, threadedTick);
            test.setTestTimeoutMs(5000);
            test.runTest();

            bool hadConnectCallback = false;
            bool hadDisconnectCallback = false;

            NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
            bool createStatus = false;

            test.listener.setConnectCallback([&hadConnectCallback]() {
                NLOG_INFO("TEST: Connect");
                hadConnectCallback = true;
            });


            test.listener.setDisconnectCallback([&hadDisconnectCallback](const NRtClientDisconnectInfo& /*info*/) {
                NLOG_INFO("TEST: Disconnect");
                hadDisconnectCallback = true;
            });

            test.rtClient->connect(session, createStatus, NTest::RtProtocol);
            test.rtClient->disconnect();
            // neither callback should have fired
            test.stopTest(!hadConnectCallback && !hadDisconnectCallback);
        }

        void test_rt_reconnect()
        {
            bool threadedTick = true;
            NTest test(__func__, threadedTick);
            test.setTestTimeoutMs(2000);
            bool hadConnectCallback = false;
            test.runTest();

            NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
            bool createStatus = false;
            test.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();

            test.rtClient->disconnect();
            test.rtClient->connectAsync(session, true).get();
            test.stopTest(true); // connect after disconnect succeeded
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

            NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
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
