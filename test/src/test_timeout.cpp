/*
 * Copyright 2025 The Nakama Authors
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

#include "NTest.h"
#include "nakama-cpp/log/NLogger.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_connectTimeout()
{
    NClientParameters parameters;
    parameters.port = 1111;
    parameters.timeout = 1;

    NTest test(__func__, parameters);
    test.setTestTimeoutMs(20000);

    auto successCallback = [&test](NSessionPtr session)
    {
        test.stopTest();
    };

    auto errorCallback = [&test](const NError& error)
    {
        NLOG_INFO("connect error " + std::to_string((int) error.code));

        test.stopTest(error.code == ErrorCode::ConnectionError || error.code == ErrorCode::CancelledByUser);
    };

    test.client->authenticateDevice("mytestdevice0007", opt::nullopt, opt::nullopt, {}, successCallback, errorCallback);

    test.runTest();
}

void test_connectTimeoutAmple()
{
    NClientParameters parameters;
    parameters.timeout = 60;

    NTest test(__func__, parameters);
    test.setTestTimeoutMs(20000);

    auto successCallback = [&test](NSessionPtr session)
    {
        test.stopTest(true);
    };

    auto errorCallback = [&test](const NError& error)
    {
        NLOG_INFO("connect error " + std::to_string((int) error.code));

        test.stopTest();
    };

    test.client->authenticateDevice("mytestdevice0007", opt::nullopt, opt::nullopt, {}, successCallback, errorCallback);

    test.runTest();
}

void test_timeouts()
{
    test_connectTimeout();
}

} // namespace Test
} // namespace Nakama
