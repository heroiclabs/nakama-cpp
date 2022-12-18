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

void test_connectError()
{
    NCppTest test(__func__);
    test.setTestTimeoutMs(20000);

    NClientParameters parameters;

    parameters.port = 1111;

    test.createClient(parameters);

    auto successCallback = [&test](NSessionPtr session)
    {
        NLOG_INFO("session token: " + session->getAuthToken());
        test.stopTest();
    };

    auto errorCallback = [&test](const NError& error)
    {
        test.stopTest(error.code == ErrorCode::ConnectionError);
    };

    test.client->authenticateDevice("mytestdevice0001", opt::nullopt, opt::nullopt, {}, successCallback, errorCallback);

    test.runTest();
}

void test_connectErrorAndDestroy()
{
    NCppTest test(__func__);

    NClientParameters parameters;

    parameters.port = 1111;

    test.createClient(parameters);

    auto successCallback = [&test](NSessionPtr session)
    {
        NLOG_INFO("session token: " + session->getAuthToken());
        test.stopTest();
    };

    auto errorCallback = [&test](const NError& error)
    {
        test.stopTest(error.code == ErrorCode::ConnectionError || error.code == ErrorCode::CancelledByUser);
    };

    test.client->authenticateDevice("mytestdevice0001", opt::nullopt, opt::nullopt, {}, successCallback, errorCallback);

    test.client.reset();
}

void test_disconnection()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        NLOG_INFO("session token: " + session->getAuthToken());
        test.stopTest(true);
    };

    auto errorCallback = [&test](const NError& error)
    {
        test.stopTest(error);
    };

    test.client->authenticateDevice("mytestdevice0001", opt::nullopt, opt::nullopt, {}, successCallback, errorCallback);

    test.client->disconnect();

    test.runTest();
}

void test_disconnect()
{
    test_connectError();
    test_connectErrorAndDestroy();
    //test_disconnection(); depending on transport implementation either success or error may be legitimately called. Test is inheritly racy
}

} // namespace Test
} // namespace Nakama
