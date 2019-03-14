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

namespace Nakama {
namespace Test {

using namespace std;

void test_rt_match_join(NRtClientTest& test, const std::string& match_id, const std::string& match_token = "")
{
    auto errorCallback = [&test](const NRtError& error)
    {
        test.stopTest();
    };

    auto successCallback = [&test](const NMatch& match)
    {
        std::cout << "joined match: " << match.matchId << std::endl;

        std::string json_data = "How are you?";

        test.rtClient->sendMatchData(
            match.matchId,
            1, // op code
            json_data,
            {}
        );
    };

    if (!match_id.empty())
    {
        test.rtClient->joinMatch(
            match_id,
            successCallback,
            errorCallback);
    }
    else
    {
        test.rtClient->joinMatchByToken(
            match_token,
            successCallback,
            errorCallback);
    }

    test.listener.setMatchDataCallback([&test](const NMatchData& data)
    {
        std::cout << "match data: " << data.data << std::endl;
        test.stopTest(true);
    });
}

void test_rt_create_match()
{
    NRtClientTest test1(__func__);
    NRtClientTest test2("test_rt_match_join");

    test1.onRtConnect = [&test1, &test2]()
    {
        auto errorCallback = [&test1, &test2](const NRtError& error)
        {
            test1.stopTest();
        };

        auto successCallback = [&test1, &test2](const NMatch& match)
        {
            std::cout << "created match: " << match.matchId << std::endl;

            test2.onRtConnect = [&test2, match]()
            {
                test_rt_match_join(test2, match.matchId);
            };

            test2.runTest();
        };

        test1.rtClient->createMatch(
            successCallback,
            errorCallback);
    };

    test1.listener.setMatchDataCallback([&test1](const NMatchData& data)
    {
        std::cout << "match data: " << data.data << std::endl;

        std::string payload = "I'm fine";

        test1.rtClient->sendMatchData(
            data.matchId,
            1, // op code
            payload,
            {}
        );

        test1.stopTest(true);
    });

    test1.runTest();
}

void test_rt_matchmaker2(NRtClientTest& test2)
{
    test2.onRtConnect = [&test2]()
    {
        auto errorCallback = [&test2](const NRtError& error)
        {
            test2.stopTest();
        };

        auto successCallback = [&test2](const NMatchmakerTicket& ticket)
        {
            std::cout << "matchmaker ticket: " << ticket.ticket << std::endl;
            // waiting for MatchmakerMatchedCallback
        };

        test2.rtClient->addMatchmaker(
            2,
            2,
            opt::nullopt,
            {},
            {},
            successCallback,
            errorCallback);
    };

    test2.listener.setMatchmakerMatchedCallback([&test2](NMatchmakerMatchedPtr matched)
    {
        std::cout << "matched token: " << matched->token << std::endl;

        test_rt_match_join(test2, "", matched->token);
    });

    test2.listener.setMatchDataCallback([&test2](const NMatchData& data)
    {
        std::cout << "match data: " << data.data << std::endl;

        std::string payload = "Nice day today!";

        test2.rtClient->sendMatchData(
            data.matchId,
            1, // op code
            payload,
            {}
        );

        test2.stopTest(true);
    });

    test2.runTest();
}

void test_rt_matchmaker()
{
    NRtClientTest test1(__func__);
    NRtClientTest test2("test_rt_matchmake2");

    test1.onRtConnect = [&test1, &test2]()
    {
        // run second test
        test_rt_matchmaker2(test2);

        auto errorCallback = [&test1, &test2](const NRtError& error)
        {
            test1.stopTest();
        };

        auto successCallback = [&test1, &test2](const NMatchmakerTicket& ticket)
        {
            std::cout << "matchmaker ticket: " << ticket.ticket << std::endl;
            // waiting for MatchmakerMatchedCallback
        };

        test1.rtClient->addMatchmaker(
            2,
            2,
            opt::nullopt,
            {},
            {},
            successCallback,
            errorCallback);
    };

    test1.listener.setMatchmakerMatchedCallback([&test1](NMatchmakerMatchedPtr matched)
    {
        std::cout << "matched token: " << matched->token << std::endl;

        test_rt_match_join(test1, "", matched->token);
    });

    test1.listener.setMatchDataCallback([&test1](const NMatchData& data)
    {
        std::cout << "match data: " << data.data << std::endl;

        std::string payload = "Nice day today!";

        test1.rtClient->sendMatchData(
            data.matchId,
            1, // op code
            payload,
            {}
        );

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
