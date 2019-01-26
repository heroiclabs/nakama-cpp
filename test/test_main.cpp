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

void test_getAccount();
void test_authentication();
void test_errors();
void test_disconnect();
void test_restoreSession();

void test_rt_joinChat();

void setWorkingClientParameters(DefaultClientParameters& parameters)
{
}

// *************************************************************

// stats
uint32_t g_runTestsCount = 0;
uint32_t g_failedTestsCount = 0;

NTest::NTest(const char * name)
{
    if (g_runTestsCount > 0)
        cout << endl << endl;

    cout << "*************************************" << endl;
    cout << "Running " << name << endl;
    cout << "*************************************" << endl;

    ++g_runTestsCount;
}

void NTest::createWorkingClient()
{
    DefaultClientParameters parameters;

    setWorkingClientParameters(parameters);

    client = createDefaultClient(parameters);
}

void NTest::createClientWithParameters(const DefaultClientParameters & parameters)
{
    client = createDefaultClient(parameters);
}

void NTest::runTest()
{
    std::chrono::milliseconds sleep_period(15);

    while (_continue_loop)
    {
        tick();

        std::this_thread::sleep_for(sleep_period);
    }

    cout << "Result: ";

    if (_testSucceeded)
    {
        cout << "OK" << endl;
    }
    else
    {
        cout << "Failed!" << endl;
        ++g_failedTestsCount;
    }
}

void NTest::stopTest(bool succeeded)
{
    _testSucceeded = succeeded;
    _continue_loop = false;
}

void NTest::tick()
{
    client->tick();
}

// *************************************************************

ostream& printPercent(ostream& os, uint32_t totalCount, uint32_t count)
{
    if (totalCount > 0)
    {
        os << count * 100 / totalCount << "%";
    }
    else
    {
        os << "0%";
    }

    return os;
}

int runAllTests()
{
    NLogger::initWithConsoleSink(NLogLevel::Debug);

    test_authentication();
    test_getAccount();
    test_disconnect();
    test_errors();
    test_restoreSession();

    test_rt_joinChat();

    // total stats
    uint32_t testsPassed = (g_runTestsCount - g_failedTestsCount);

    cout << endl << endl;
    cout << "Total tests : " << g_runTestsCount << endl;
    cout << "Tests passed: " << testsPassed << " ("; printPercent(cout, g_runTestsCount, testsPassed) << ")" << endl;
    cout << "Tests failed: " << g_failedTestsCount << " ("; printPercent(cout, g_runTestsCount, g_failedTestsCount) << ")" << endl;
    return 0;
}

} // namespace Test
} // namespace Nakama

int main()
{
    return Nakama::Test::runAllTests();
}
