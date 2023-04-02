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

#include "nakama-cpp/log/NLogger.h"
#include "NTest.h"
#include "TestGuid.h"

namespace Nakama {
    namespace Test {

        using namespace std;

        void test_rt_create_match()
        {
            const bool threadedTick = true;
            NTest test1(__func__, threadedTick);
            NTest test2(__func__ + std::string("2"), threadedTick);

            test1.runTest();
            test2.runTest();

            NSessionPtr session = test1.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
            bool createStatus = false;
            test1.rtClient->connectAsync(session, createStatus).get();

            NMatch match = test1.rtClient->createMatchAsync().get();
            NLOG_INFO("created match: " + match.matchId);
        }

        void test_rt_matchmaker()
        {
            bool threadedTick = true;
            NTest test1(__func__, threadedTick);
            NTest test2(std::string(__func__) + std::string("2"), threadedTick);

            test1.setTestTimeoutMs(20000);
            test2.setTestTimeoutMs(20000);

            test1.runTest();
            test2.runTest();

            NSessionPtr session = test1.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
            bool createStatus = false;
            test1.rtClient->connectAsync(session, createStatus).get();

            NSessionPtr session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
            test2.rtClient->connectAsync(session2, createStatus).get();

            test1.listener.setMatchDataCallback([&test1, &test2](const NMatchData& data)
            {
                NLOG_INFO("match data: " + data.data);

                test1.stopTest(data.opCode == 1);
                test2.stopTest(data.data == "Nice day today!");
            });

            test1.listener.setMatchmakerMatchedCallback([&test1](NMatchmakerMatchedPtr matched)
            {
                test1.rtClient->joinMatchAsync(matched->matchId, {}).get();
            });

            test2.listener.setMatchmakerMatchedCallback([&test2](NMatchmakerMatchedPtr matched)
            {
                test2.rtClient->joinMatchAsync(matched->matchId, {}).get();
                test2.rtClient->sendMatchDataAsync(matched->matchId, 1, "Nice day today!").get();
            });

            const int minCount = 2;
            const int maxCount = 2;
            const std::string query = "";
            const NStringMap stringProperties = {};
            const NStringDoubleMap numericProperties = {};
            const int countMultiple = 2;
            test1.rtClient->addMatchmakerAsync(minCount, maxCount, query, stringProperties, numericProperties, countMultiple).get();
            test2.rtClient->addMatchmakerAsync(minCount, maxCount, query, stringProperties, numericProperties, countMultiple).get();

            test1.waitUntilStop();
            test2.waitUntilStop();
        }

        void test_rt_match()
        {
            test_rt_create_match();
            test_rt_matchmaker();
        }

    } // namespace Test
} // namespace Nakama
