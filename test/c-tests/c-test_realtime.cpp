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

#include "c-test.h"

namespace Nakama {
namespace Test {

using namespace std;

static void connectCallback(NRtClient client)
{
    std::cout << "connected" << std::endl;

    stopCTest(getCurCTest()->client, true);
}

static void successCallback(NClient client, NClientReqData reqData, NSession session)
{
    const char* token = NSession_getAuthToken(session);

    std::cout << "session token: " << token << std::endl;

    NRtClient rtClient = NClient_createRtClient(client, NDEFAULT_PORT);
    getCurCTest()->rtClient = rtClient;

    NRtClient_setConnectCallback(rtClient, connectCallback);

    NRtClient_connect(rtClient, session, true, NRtClientProtocol_Protobuf);

    NSession_destroy(session);
}

void ctest_realtime_connect()
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
    
    NRtClient_destroy(getCurCTest()->rtClient);
    destroyNakamaClient(getCurCTest()->client);
}

void ctest_realtime()
{
    ctest_realtime_connect();
}

} // namespace Test
} // namespace Nakama
