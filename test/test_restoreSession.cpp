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
    NTest test(__func__);

    test.createWorkingClient();

    NSessionPtr my_session;

    auto errorCallback = [&test](const NError& error)
    {
        std::cout << "error: " << error.GetErrorMessage() << std::endl;
        test.stopTest();
    };

    auto successCallback = [&test, &my_session, errorCallback](NSessionPtr session)
    {
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
        else if (!my_session->isExpired(getUnixTimestampMs() + 2 * 60 * 1000))
        {
            std::cout << "restored session must expired after 2 minutes" << std::endl;
            test.stopTest();
        }
        else
        {
            auto successCallback = [&test](const NAccount& account)
            {
                std::cout << "account user id: " << account.user.id << std::endl;
                test.stopTest(true);
            };

            test.client->getAccount(my_session, successCallback, errorCallback);
        }
    };

    test.client->authenticateDevice("mytestdevice0000", opt::nullopt, true, successCallback, errorCallback);

    test.runTest();
}

} // namespace Test
} // namespace Nakama
