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

#include "nakama-c/ClientFactory.h"
#include "test_serverConfig.h"
#include <stdio.h>

static bool stopTest = false;

static void TestErrorCallback(NClient client, const sNError* error)
{
    // stop test
    printf("Failed\n");
    stopTest = true;
}

static void SessionSuccessCallback(NClient client, NClientReqData reqData, NSession session)
{
    const char* token = NSession_getAuthToken(session);

    printf("session token: %s\n", token);

    // stop test

    destroyNakamaSession(session);
    stopTest = true;
}

void c_test_pure()
{
    NClient client;
    tNClientParameters cParameters;

    printf("running pure C test\n\n");

    cParameters.host = SERVER_HOST;
    cParameters.port = SERVER_HTTP_PORT;
    cParameters.serverKey = SERVER_KEY;
    cParameters.ssl = SERVER_SSL;

    client = createDefaultNakamaClient(&cParameters);

    NClient_setErrorCallback(client, TestErrorCallback);

    NClient_authenticateDevice(client,
        "mytestdevice0000",
        NULL, // username
        true,    // create
        NULL,
        NULL,
        SessionSuccessCallback,
        NULL
    );

    while (!stopTest)
    {
        NClient_tick(client);
    }

    destroyNakamaClient(client);
}
