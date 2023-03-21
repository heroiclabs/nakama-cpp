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

#include "NCppTest.h"
#include "globals.h"

namespace Nakama {
namespace Test {

using namespace std;

class NMatchListTest : public NCppTest
{
    NSessionPtr session;

public:
    explicit NMatchListTest(const char* name) : NCppTest(name) {}

    void runTest() override
    {
        createWorkingClient();

        auto successCallback = [this](NSessionPtr sess)
        {
            this->session = sess;

            NLOG_INFO("session token: " + sess->getAuthToken());

            listMatches();
        };

        client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

        NCppTest::runTest();
    }

    void listMatches()
    {
        auto rpcSuccessCallback = [this](const NRpc& rpc)
        {
            auto successCallback = [this](NMatchListPtr matchList)
            {
                NLOG_INFO("Expecting match count to be 2. Actual count: " + std::to_string(matchList->matches.size()));
                NTEST_ASSERT(matchList->matches.size() == 2);
                stopTest(true);
            };

            int minPlayers = 0;
            int maxPlayers = 10;
            int limit = 10;
            bool authoritative = true;
            string label = "";
            string query = "+label.type:freeforall +label.difficulty:>1";
            client->listMatches(session, minPlayers, maxPlayers, limit, label, query, authoritative, successCallback);
        };

        client->rpc(session, "create_matches", "", rpcSuccessCallback);
    }
};

void test_listMatches()
{
    NMatchListTest test(__func__);
    test.runTest();
}

} // namespace Test
} // namespace Nakama
