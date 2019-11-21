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

void wrapper_test_authenticateEmail()
{
    WrapperTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        std::cout << "session token: " << session->getAuthToken() << std::endl;
        NTEST_ASSERT(!session->getAuthToken().empty());
        NTEST_ASSERT(!session->getUserId().empty());
        NTEST_ASSERT(!session->getUsername().empty());
        NTEST_ASSERT(session->getVariable("param1") == "test value");
        NTEST_ASSERT(session->getVariable("param2") == "");
        test.stopTest(true);
    };

    NStringMap vars;

    vars.emplace("param1", "test value");
    vars.emplace("param2", "");

    test.client->authenticateEmail("test@mail.com", "12345678", "", true, vars, successCallback);

    test.runTest();
}

void wrapper_test_authenticateDevice()
{
    WrapperTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        std::cout << "session token: " << session->getAuthToken() << std::endl;
        test.stopTest(session->getAuthToken().empty() == false);
    };

    test.client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

    test.runTest();
}

void wrapper_test_authenticateDevice2()
{
    WrapperTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        std::cout << "session token: " << session->getAuthToken() << std::endl;
        test.stopTest(session->getAuthToken().empty() == false);
        NTEST_ASSERT(session->getVariable("param1") == "test value");
    };

    NStringMap vars;

    vars.emplace("param1", "test value");

    test.client->authenticateDevice("mytestdevice0001", opt::nullopt, opt::nullopt, vars, successCallback);

    test.runTest();
}

class MyLogger : public NLogSinkInterface
{
public:
    void log(NLogLevel level, const std::string& message, const char* func = nullptr) override
    {
        std::cout << func << ": " << message << std::endl;
    }

    void flush() override {}
};

void wrapper_test_authentication()
{
    NLogger::init(NLogSinkPtr(new MyLogger()), NLogLevel::Debug);

    wrapper_test_authenticateEmail();
    wrapper_test_authenticateDevice();
    wrapper_test_authenticateDevice2();
}

} // namespace Test
} // namespace Nakama
