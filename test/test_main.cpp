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
#include "test_server_config.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_getAccount();
void test_authentication();
void test_errors();
void test_disconnect();
void test_restoreSession();
void test_storage();
void test_realtime();

// currently running tests
std::vector<NTest*> g_running_tests;

// stats
uint32_t g_runTestsCount = 0;
uint32_t g_failedTestsCount = 0;

void setWorkingClientParameters(DefaultClientParameters& parameters)
{
    parameters.host      = SERVER_HOST;
    parameters.port      = SERVER_GRPC_PORT;
    parameters.serverKey = SERVER_KEY;
}

void addRunningTest(NTest* test)
{
    g_running_tests.push_back(test);
}

void removeRunningTest(NTest* test)
{
    for (auto it = g_running_tests.begin(); it != g_running_tests.end(); ++it)
    {
        if (*it == test)
        {
            g_running_tests.erase(it);
            break;
        }
    }
}

void runTestsLoop()
{
    std::chrono::milliseconds sleep_period(15);

    while (!g_running_tests.empty())
    {
        auto running_tests = g_running_tests;

        for (auto test : running_tests)
        {
            if (!test->isDone())
            {
                test->tick();
            }
        }

        std::this_thread::sleep_for(sleep_period);
    }
}

// *************************************************************

NTest::NTest(const char * name)
{
    _name = name;
}

NTest::~NTest()
{
    removeRunningTest(this);
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
    if (g_runTestsCount > 0)
        cout << endl << endl;

    ++g_runTestsCount;

    addRunningTest(this);

    printTestName("Running");

    if (g_running_tests.size() == 1)
    {
        runTestsLoop();
    }
}

void NTest::stopTest(bool succeeded)
{
    removeRunningTest(this);

    _testSucceeded = succeeded;
    _continue_loop = false;

    if (succeeded)
    {
        printTestName("Succeeded");
    }
    else
    {
        ++g_failedTestsCount;
        printTestName("Failed");
    }

    cout << endl << endl;
}

void NTest::tick()
{
    client->tick();
}

void NTest::printTestName(const char* event)
{
    cout << "*************************************" << endl;
    cout << event << " " << _name << endl;
    cout << "*************************************" << endl;
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
    test_storage();
    test_realtime();

    // total stats
    uint32_t testsPassed = (g_runTestsCount - g_failedTestsCount);

    cout << endl << endl;
    cout << "Total tests : " << g_runTestsCount << endl;
    cout << "Tests passed: " << testsPassed << " ("; printPercent(cout, g_runTestsCount, testsPassed) << ")" << endl;
    cout << "Tests failed: " << g_failedTestsCount << " ("; printPercent(cout, g_runTestsCount, g_failedTestsCount) << ")" << endl;

    return g_failedTestsCount == 0 ? 0 : -1;
}

} // namespace Test
} // namespace Nakama

int main()
{
    return Nakama::Test::runAllTests();
}
