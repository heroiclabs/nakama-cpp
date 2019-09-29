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

#include "wrapper-test.h"

namespace Nakama {
namespace Test {

using namespace std;

void wrapper_test_getAccount()
{
    WrapperTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        std::cout << "session token: " << session->getAuthToken() << std::endl;

        test.client->getAccount(session,
            [&](const NAccount& account)
            {
                NTEST_ASSERT(account.devices.size() > 0);
                NTEST_ASSERT(account.devices[0].id == "mytestdevice0001");
                test.stopTest(true);
            });
    };

    test.client->authenticateDevice("mytestdevice0001", opt::nullopt, opt::nullopt, {}, successCallback);

    test.runTest();
}

void wrapper_test_account()
{
    wrapper_test_getAccount();
}

} // namespace Test
} // namespace Nakama
