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

#include "test_main.h"
#include "nakama-cpp/NUtils.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_restoreSession()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    NSessionPtr my_session;

    {
        string token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE1MTY5MTA5NzMsInVpZCI6ImY0MTU4ZjJiLTgwZjMtNDkyNi05NDZiLWE4Y2NmYzE2NTQ5MCIsInVzbiI6InZUR2RHSHl4dmwifQ.gzLaMQPaj5wEKoskOSALIeJLOYXEVFoPx3KY0Jm1EVU";
        my_session = restoreSession(token);
        NTEST_ASSERT(my_session->getAuthToken() == token);
        NTEST_ASSERT(my_session->getVariables().empty());
    }

    {
        string token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE1MTY5MTA5NzMsInVpZCI6ImY0MTU4ZjJiLTgwZjMtNDkyNi05NDZiLWE4Y2NmYzE2NTQ5MCIsInVzbiI6InZUR2RHSHl4dmwiLCJ2cnMiOnsiazEiOiJ2MSIsImsyIjoidjIifX0.Hs9ltsNmtrTJXi2U21jjuXcd-3DMsyv4W6u1vyDBMTo";
        my_session = restoreSession(token);
        NTEST_ASSERT(my_session->getAuthToken() == token);
        NTEST_ASSERT(my_session->getUsername() == "vTGdGHyxvl");
        NTEST_ASSERT(my_session->getUserId() == "f4158f2b-80f3-4926-946b-a8ccfc165490");
        NTEST_ASSERT(my_session->getVariable("k1") == "v1");
        NTEST_ASSERT(my_session->getVariable("k2") == "v2");
    }

    my_session.reset();

    auto successCallback = [&test, &my_session](NSessionPtr session)
    {
        const int expirePeriodMinutes = 120;
        my_session = restoreSession(session->getAuthToken());

        std::cout << "session token: " << my_session->getAuthToken() << std::endl;
        std::cout << "isExpired: " << my_session->isExpired() << std::endl;

        if (session->isExpired())
        {
            std::cout << "original session must not be expired" << std::endl;
            test.stopTest();
        }
        else if (my_session->isExpired())
        {
            std::cout << "restored session must not be expired" << std::endl;
            test.stopTest();
        }
        else if (!my_session->isExpired(getUnixTimestampMs() + expirePeriodMinutes * 60 * 1000))
        {
            std::cout << "restored session must expired after " << expirePeriodMinutes << " minutes" << std::endl;
            test.stopTest();
        }
        else
        {
            auto successCallback = [&test](const NAccount& account)
            {
                std::cout << "account user id: " << account.user.id << std::endl;
                test.stopTest(true);
            };

            test.client->getAccount(my_session, successCallback);
        }
    };

    test.client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

    test.runTest();
}

} // namespace Test
} // namespace Nakama
