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
#include "test_serverConfig.h"

namespace Nakama {
namespace Test {

    CTest::~CTest()
    {
        destroyNakamaClient(client);
    }

    static void TestErrorCallback(NClient client, const sNError*)
    {
        stopCTest(client);
    }

    void CTest::createWorkingClient()
    {
        tNClientParameters cParameters;

        cParameters.host = SERVER_HOST;
        cParameters.port = SERVER_HTTP_PORT;
        cParameters.serverKey = SERVER_KEY;
        cParameters.ssl = SERVER_SSL;

        client = createDefaultNakamaClient(&cParameters);

        NClient_setUserData(client, this);
        NClient_setErrorCallback(client, TestErrorCallback);
    }

    void CTest::tick()
    {
        NClient_tick(client);
        if (rtClient)
            NRtClient_tick(rtClient);
    }

    CTest* getCurCTest()
    {
        return (CTest*)getCurTest();
    }

    void stopCTest(NClient client, bool succeeded)
    {
        CTest* test = (CTest*)NClient_getUserData(client);

        test->stopTest(succeeded);
    }

} // namespace Test
} // namespace Nakama
