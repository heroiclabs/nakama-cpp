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

#ifdef BUILD_C_API

#include "wrapper-test.h"

namespace Nakama {
namespace Test {

using namespace std;

void wrapper_test_rt_match_join(WrapperTest& test, const std::string& match_id, const std::string& match_token = "")
{
    std::shared_ptr<std::string> payload(new std::string());
    payload->resize(1024);
    char next = 1;
    for (char& c : *payload)
    {
        c = next;
        next++;
    }

    auto successCallback = [&test, payload](const NMatch& match)
    {
        std::cout << "joined match: " << match.matchId << std::endl;

        test.rtClient->sendMatchData(
            match.matchId,
            1, // op code
            *payload,
            {}
        );
    };

    if (!match_id.empty())
    {
        NStringMap metadata;

        metadata.emplace("key", "value");

        test.rtClient->joinMatch(
            match_id,
            metadata,
            successCallback);
    }
    else
    {
        test.rtClient->joinMatchByToken(
            match_token,
            successCallback);
    }

    test.listener.setMatchDataCallback([&test, payload](const NMatchData& data)
    {
        std::cout << "match data size: " << data.data.size() << std::endl;
        NTEST_ASSERT(*payload == data.data);
        test.stopTest(true);
    });
}

void wrapper_test_rt_create_match()
{
    WrapperTest test1(__func__);
    WrapperTest test2("test_rt_match_join");

    test1.connect([&test1, &test2]()
    {
        auto successCallback = [&test1, &test2](const NMatch& match)
        {
            std::cout << "created match: " << match.matchId << std::endl;

            test2.connect([&test2, match]()
            {
                wrapper_test_rt_match_join(test2, match.matchId);
            });

            test2.runTest();
        };

        test1.rtClient->createMatch(
            successCallback);
    });

    test1.listener.setMatchDataCallback([&test1](const NMatchData& data)
    {
        std::cout << "match data size: " << data.data.size() << std::endl;

        test1.rtClient->sendMatchData(
            data.matchId,
            1, // op code
            data.data,
            {}
        );

        test1.stopTest(true);
    });

    test1.runTest();
}

void wrapper_test_rt_matchmaker2(WrapperTest& test2)
{
    test2.connect([&test2]()
    {
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
            successCallback);
    });

    test2.listener.setMatchmakerMatchedCallback([&test2](NMatchmakerMatchedPtr matched)
    {
        std::cout << "matched token: " << matched->token << std::endl;

        wrapper_test_rt_match_join(test2, "", matched->token);
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

void wrapper_test_rt_matchmaker()
{
    WrapperTest test1(__func__);
    WrapperTest test2("test_rt_matchmake2");

    test1.connect([&test1, &test2]()
    {
        // run second test
        wrapper_test_rt_matchmaker2(test2);

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
            successCallback);
    });

    test1.listener.setMatchmakerMatchedCallback([&test1](NMatchmakerMatchedPtr matched)
    {
        std::cout << "matched token: " << matched->token << std::endl;

        wrapper_test_rt_match_join(test1, "", matched->token);
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

void wrapper_test_rt_match()
{
    wrapper_test_rt_create_match();
    wrapper_test_rt_matchmaker();
}

} // namespace Test
} // namespace Nakama

#endif // BUILD_C_API
