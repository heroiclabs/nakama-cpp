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

void test_rt_create_party()
{
    bool threadedTick = true;
    NTest test1(__func__, threadedTick);
    NTest test2(std::string(__func__) + std::string("2"), threadedTick);

    test1.runTest();
    test2.runTest();

    NSessionPtr session = test1.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
    NSessionPtr session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();

    bool createStatus = false;
    test1.rtClient->connectAsync(session, createStatus, NRtClientProtocol::Json).get();
    test2.rtClient->connectAsync(session2, createStatus, NRtClientProtocol::Json).get();

    const NParty& party = test1.rtClient->createPartyAsync(true, 2).get();

    std::promise<void> partyJoinPromise;
    std::promise<void> partyDataPromise;

    auto partyJoinCallback = [&partyJoinPromise](const NParty& party)
    {
        partyJoinPromise.set_value();
    };

    test2.listener.setPartyCallback(partyJoinCallback);

    test1.listener.setPartyDataCallback([&partyDataPromise](const NPartyData& data)
    {
        partyDataPromise.set_value();
    });

    test2.rtClient->joinPartyAsync(party.id).get();

    test1.stopTest(true);
    test2.stopTest(true);
}

void test_rt_party_matchmaker()
{
    NTest test1(__func__, true);
    NTest test2("test_rt_party_matchmaker2", true);

    test1.runTest();
    test2.runTest();
    NSessionPtr session = test1.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
    NSessionPtr session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();

    bool createStatus = false;
    test1.rtClient->connectAsync(session, createStatus, NRtClientProtocol::Json).get();
    test2.rtClient->connectAsync(session2, createStatus, NRtClientProtocol::Json).get();

    auto party1 = test1.rtClient->createPartyAsync(false, 1).get();
    auto ticket1 = test1.rtClient->addMatchmakerPartyAsync(party1.id, "*", 2, 2).get();
    test1.stopTest(ticket1.ticket != "");

    auto party2 = test2.rtClient->createPartyAsync(false, 1).get();
    auto ticket2 = test2.rtClient->addMatchmakerPartyAsync(party2.id, "*", 2, 2).get();
    test2.stopTest(ticket2.ticket != "");

}

void test_rt_party()
{
    test_rt_create_party();
    test_rt_party_matchmaker();
}

} // namespace Test
} // namespace Nakama
