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

namespace Nakama {
namespace Test {

using namespace std;

void test_getAccount()
{
    NTest test(__func__);

    test.createWorkingClient();

    NSessionPtr my_session;

    auto errorCallback = [&test](const NError& error)
    {
        test.stopTest();
    };

    auto successCallback = [&test, &my_session, errorCallback](NSessionPtr session)
    {
        my_session = session;
        std::cout << "session token: " << session->getAuthToken() << std::endl;

        auto successCallback = [&test](const NAccount& account)
        {
            std::cout << "account user id: " << account.user.id << std::endl;
            test.stopTest(true);
        };

        test.client->getAccount(session, successCallback, errorCallback);
    };

    test.client->authenticateDevice("mytestdevice0000", opt::nullopt, true, successCallback, errorCallback);

    test.runTest();
}

} // namespace Test
} // namespace Nakama
