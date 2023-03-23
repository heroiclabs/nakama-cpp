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

namespace Nakama {
namespace Test {

using namespace std;

void test_rt_party_join(NTest& test, const std::string& party_id)
{
    auto successCallback = [&test](const NParty& party)
    {
        NLOG_INFO("joined party: " + party.id);

        std::string payload = "How are you?";

        test.rtClient->sendPartyData(
            party.id,
            1, // op code
            payload
        );
    };

    NStringMap metadata;

    metadata.emplace("key", "value");

    test.rtClient->joinParty(party_id);

    test.listener.setPartyCallback(successCallback);

    test.listener.setPartyDataCallback([&test](const NPartyData& data)
    {
        NLOG_INFO("party data: " + data.data);
        test.stopTest(true);
    });
}

void test_rt_create_party()
{
    NTest test1(__func__);
    NTest test2("test_rt_party_join");

    test1.onRtConnect = [&test1, &test2]()
    {
        auto successCallback = [&test2](const NParty& party)
        {
            NLOG_INFO("created match: " + party.id);

            test2.onRtConnect = [&test2, party]()
            {
                test_rt_party_join(test2, party.id);
            };

            test2.runTest();
        };

        test1.rtClient->createParty(true, 2, successCallback);
    };

    test1.listener.setPartyDataCallback([&test1](const NPartyData& data)
    {
        NLOG_INFO("party data: " + data.data);

        std::string payload = "I'm fine";

        test1.rtClient->sendPartyData(
            data.partyId,
            1, // op code
            payload
        );

        test1.stopTest(true);
    });

    test1.runTest();
}

void test_rt_party_matchmaker()
{
    NTest test1(__func__, true);
    NTest test2("test_rt_party_matchmaker2", true);

    test1.onRtConnect = [&test1]()
    {
        auto party = test1.rtClient->createPartyAsync(false, 1).get();
        auto ticket = test1.rtClient->addMatchmakerPartyAsync(party.id, "*", 0, 2).get();
        test1.stopTest(ticket.ticket != "");
    };

    test2.onRtConnect = [&test2]()
    {
        auto party = test2.rtClient->createPartyAsync(false, 1).get();
        auto ticket = test2.rtClient->addMatchmakerPartyAsync(party.id, "*", 0, 2).get();
        test2.stopTest(ticket.ticket != "");
    };

    test1.runTest();
    test2.runTest();
}

void test_rt_party()
{
    test_rt_create_party();
    test_rt_party_matchmaker();
}

} // namespace Test
} // namespace Nakama
