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

#include "c-test.h"

namespace Nakama {
namespace Test {

using namespace std;

static void successCallback(NClient client, NClientReqData reqData, NSession session)
{
    const char* token = NSession_getAuthToken(session);

    std::cout << "session token: " << token << std::endl;

    stopCTest(client, token[0] != 0);

    NSession_destroy(session);
}

void ctest_authenticateDevice()
{
    CTest test(__func__);

    test.createWorkingClient();

    NClient_authenticateDevice(test.client,
        "mytestdevice0000",
        nullptr, // username
        true,    // create
        nullptr,
        nullptr,
        successCallback,
        nullptr
    );

    test.runTest();
}

void ctest_authenticateDevice2()
{
    CTest test(__func__);

    test.createWorkingClient();

    NClient_authenticateDevice(test.client,
        "mytestdevice0001",
        "testuser", // username
        true,       // create
        nullptr,
        nullptr,
        successCallback,
        nullptr
    );

    test.runTest();
}

void ctest_authentication()
{
    ctest_authenticateDevice();
    ctest_authenticateDevice2();
}

} // namespace Test
} // namespace Nakama

#endif // BUILD_C_API
