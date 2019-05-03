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

void test_authenticateDevice()
{
    NTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        std::cout << "session token: " << session->getAuthToken() << std::endl;
        test.stopTest(true);
    };

    test.client->authenticateDevice("mytestdevice0000", opt::nullopt, true, successCallback);

    test.runTest();
}

void test_authenticateDevice2()
{
    NTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        std::cout << "session token: " << session->getAuthToken() << std::endl;
        test.stopTest(true);
    };

    test.client->authenticateDevice("mytestdevice0001", opt::nullopt, opt::nullopt, successCallback);

    test.runTest();
}

void test_authentication()
{
    test_authenticateDevice();
    test_authenticateDevice2();
}

} // namespace Test
} // namespace Nakama
