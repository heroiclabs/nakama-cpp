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
#include "realtime/RtClientTest.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_rt_match_join(NRtClientTest& test, const std::string& match_id, const std::string& match_token = "")
{
    NMatch match;

    if (!match_id.empty())
    {
        NStringMap metadata;
        metadata.emplace("key", "value");
        match = test.rtClient->joinMatchAsync(match_id, metadata).get();
    }
    else
    {
        test.rtClient->joinMatchByTokenAsync(match_token).get();
    }

    NLOG_INFO("joined match: " + match.matchId);

    const std::string payload = "How are you?";

    const int opcode = 1;
    test.rtClient->sendMatchDataAsync(match.matchId, opcode, payload).get();

    test.listener.setMatchDataCallback([&test](const NMatchData& data)
    {
        NLOG_INFO("match data: " + data.data);
        test.stopTest(true);
    });
}

void test_rt_create_match()
{
    const bool threadedTick = true;
    NRtClientTest test1(__func__, threadedTick);
    NRtClientTest test2("test_rt_match_join", threadedTick);

    test1.onRtConnect = [&test1, &test2]()
    {
        test2.runTest();
        NMatch match = test1.rtClient->createMatchAsync().get();
        NLOG_INFO("created match: " + match.matchId);
        test_rt_match_join(test2, match.matchId);
    };

    test1.listener.setMatchDataCallback([&test1](const NMatchData& data)
    {
        NLOG_INFO("match data: " + data.data);
        const std::string payload = "I'm fine";
        const int opcode = 1;
        test1.rtClient->sendMatchData(data.matchId, opcode, payload);
        test1.stopTest(true);
    });


    test1.runTest();
}

void test_rt_matchmaker2(NRtClientTest& test2)
{
    test2.onRtConnect = [&test2]()
    {
        const int minCount = 2;
        const int maxCount = 2;
        const std::string query = "";
        const NStringMap stringProperties = {};
        const NStringDoubleMap numericProperties = {};
        const int countMultiple = 2;
        test2.rtClient->addMatchmakerAsync(minCount, maxCount, query, stringProperties, numericProperties, countMultiple).get();
    };

    test2.listener.setMatchmakerMatchedCallback([&test2](NMatchmakerMatchedPtr matched)
    {
        NLOG_INFO("matched token: " + matched->token);
        test_rt_match_join(test2, "", matched->token);
    });

    test2.listener.setMatchDataCallback([&test2](const NMatchData& data)
    {
        NLOG_INFO("match data: " + data.data);
        const std::string payload = "Nice day today!";
        const int opcode = 1;
        test2.rtClient->sendMatchDataAsync(data.matchId, opcode, payload).get();
        test2.stopTest(true);
    });

    test2.runTest();
}

void test_rt_matchmaker()
{
    NRtClientTest test1(__func__);
    test1.setTestTimeoutMs(20000);
    NRtClientTest test2("test_rt_matchmake2");
    test2.setTestTimeoutMs(20000);

    test1.onRtConnect = [&test1, &test2]()
    {
        // run second test
        test_rt_matchmaker2(test2);

        auto successCallback = [](const NMatchmakerTicket& ticket)
        {
            NLOG_INFO("matchmaker ticket: " + ticket.ticket);
            // waiting for MatchmakerMatchedCallback
        };

        test1.rtClient->addMatchmaker(
            2,
            2,
            opt::nullopt,
            {},
            {},
            2,
            successCallback);
    };

    test1.listener.setMatchmakerMatchedCallback([&test1](NMatchmakerMatchedPtr matched)
    {
        NLOG_INFO("matched token: " + matched->token);

        test_rt_match_join(test1, "", matched->token);
    });

    test1.listener.setMatchDataCallback([&test1](const NMatchData& data)
    {
        NLOG_INFO("match data: " + data.data);
        std::string payload = "Nice day today!";
        int opcode = 1;
        test1.rtClient->sendMatchDataAsync(data.matchId, opcode, payload).get();
        test1.stopTest(true);
    });

    test1.runTest();
}

void test_rt_match()
{
    test_rt_create_match();
    test_rt_matchmaker();
}

} // namespace Test
} // namespace Nakama
