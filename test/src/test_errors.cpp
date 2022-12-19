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

void test_error_NotFound()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        NLOG_INFO("session token: " + session->getAuthToken());
        test.stopTest(false);
    };

    auto errorCallback = [&test](const NError& error)
    {
        test.stopTest(error.code == ErrorCode::NotFound);
    };

    test.client->authenticateDevice("_not_existing_device_id_", opt::nullopt, false, {}, successCallback, errorCallback);

    test.runTest();
}

void test_error_InvalidArgument()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        NLOG_INFO("session token: " + session->getAuthToken());
        test.stopTest();
    };

    auto errorCallback = [&test](const NError& error)
    {
        test.stopTest(error.code == ErrorCode::InvalidArgument);
    };

    test.client->authenticateDevice("", opt::nullopt, false, {}, successCallback, errorCallback);

    test.runTest();
}

void test_error_InvalidArgument2()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        NLOG_INFO("session token: " + session->getAuthToken());
        test.stopTest();
    };

    auto errorCallback = [&test](const NError& error)
    {
        test.stopTest(error.code == ErrorCode::InvalidArgument);
    };

    test.client->authenticateDevice("1", opt::nullopt, false, {}, successCallback, errorCallback);

    test.runTest();
}

void test_error_Unauthenticated()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto errorCallback = [&test](const NError& error)
    {
        test.stopTest(error.code == ErrorCode::Unauthenticated);
    };

    auto session = restoreSession("dfgdfgdfg.dfgdfgdfg.dfgdfgdfg", "dfgdfgdfg.dfgdfgdfg.dfgdfgdfg");

    test.client->getAccount(session, nullptr, errorCallback);

    test.runTest();
}

void test_errors()
{
    test_error_NotFound();
    test_error_InvalidArgument();
    test_error_InvalidArgument2();
    test_error_Unauthenticated();
}

} // namespace Test
} // namespace Nakama
