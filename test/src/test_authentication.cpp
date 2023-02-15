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

void test_authenticateEmail1()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        test.stopTest(!session->getAuthToken().empty());
    };

    test.client->authenticateEmail("test@mail.com", "12345678", "", true, {}, successCallback);
    test.runTest();
}

void test_authenticateEmail2()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
         // ensure that username is encoded properly
        NLOG_INFO("session token: " + session->getAuthToken());
        NLOG_INFO("returning username: " + session->getUsername());
        test.stopTest(!session->getAuthToken().empty() && u8"βσκαταη3" == session->getUsername());
    };

    test.client->authenticateEmail("test2@mail.com", "12345678", u8"βσκαταη3", true, {}, successCallback);

    test.runTest();
}

void test_authenticateDevice()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        NLOG_INFO("session token: " + session->getAuthToken());
        test.stopTest(session->getAuthToken().empty() == false);
    };

    test.client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

    test.runTest();
}

void test_authenticateDevice2()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        NLOG_INFO("session token: " + session->getAuthToken());
        test.stopTest(session->getAuthToken().empty() == false);
        NTEST_ASSERT(session->getVariable("param1") == "test value");
    };

    NStringMap vars;

    vars.emplace("param1", "test value");

    test.client->authenticateDevice("mytestdevice0001", opt::nullopt, opt::nullopt, vars, successCallback);

    test.runTest();
}

void test_authenticateRefresh()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session1)
    {
        auto successCallback2 = [&test, session1](NSessionPtr session2)
        {
            test.stopTest(session2->getAuthToken().empty() == false && session2->getAuthToken() == session1->getAuthToken());
        };

        auto errorCallback2 = [&test](const NError& error)
        {
            test.stopTest(error);
        };

        test.client->authenticateRefresh(session1, successCallback2, errorCallback2);
    };

    NStringMap vars;

    vars.emplace("param1", "test value");

    test.client->authenticateDevice("mytestdevice0001", opt::nullopt, true, vars, successCallback);

    test.runTest();
}

void test_authentication()
{
    test_authenticateEmail1();
#if !defined(__UNREAL__)
// No matter what I do, it still fails when running from within Unreal on Windows, because
// u8"...." utf-8 string constant is interpreted  incorrectly. When compiling outside
// Unreal we pass /utf-8 on MSVC, but there seems to be no way to pass arbitrary compiler flags
// when compiling Unreal module.
// According to Unreal's Engine/Source/Programs/UnrealBuildTool/Platform/Windows/VCToolChain.cs
// it passes '/source-charset:utf-8' and '/execution-charset:utf-8' which according to MSVC docs
// is exactly what `/utf-8` flag expands to, but either it doesn't really pass it (and annoyingly
// there is no way to see exact compiler flags used by UnrealBuildTool or at least I didn't find one)
// or it doesn't have exactly same effect as `/utf-8`.
// According to MSVC docs, another way to tell compiler that source is UTF-8 encoded is to add BOM,
// which this file has now, but it changed nothing.
    test_authenticateEmail2();
#endif

    test_authenticateDevice();
    test_authenticateDevice2();
    //test_authenticateRefresh();
}

} // namespace Test
} // namespace Nakama
