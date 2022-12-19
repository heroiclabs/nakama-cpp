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

#include "test_base.h"
#include <chrono>
#include <thread>
#include <vector>
#include <iostream>
#include <nakama-cpp/NError.h>
#include "nakama-cpp/log/NLogger.h"

namespace Nakama {
namespace Test {

using namespace std;

// currently running tests
std::vector<NTest*> g_running_tests;
NTest* g_cur_test = nullptr;

// stats
uint32_t g_runTestsCount = 0;
uint32_t g_failedTestsCount = 0;

void(*g_on_tick)(int ms) = nullptr;

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
    while (!g_running_tests.empty())
    {
        auto running_tests = g_running_tests;
        int step = 15;

        for (auto test : running_tests)
        {
            if (!test->isDone())
            {
                if (!test->checkTimeout(step)) {
                    test->onTimeout();
                    NLOG_INFO("Test timeout");
                    test->stopTest(test->isSucceeded());
                }
                test->tick();
                if (g_on_tick != nullptr) g_on_tick(15);
            }
        }

        sleep(step);
    }
}

void abortCurrentTest(const char* file, int lineno)
{
    if (g_running_tests.size() > 1)
    {
        NLOG_INFO(std::to_string(g_running_tests.size()) + " tests are running, aborting one...");
    }

    NLOG_INFO("TEST ASSERT FAILED!");
    NLOG_INFO(std::string(file) + ":" + std::to_string(lineno));
    g_cur_test->stopTest();
}

NTest* getCurTest()
{
    return g_cur_test;
}

void sleep(uint32_t ms)
{
    std::chrono::milliseconds sleep_period(ms);
    std::this_thread::sleep_for(sleep_period);
}

// *************************************************************

NTest::NTest(const char * name)
        : _name(name)
{
    g_cur_test = this;
}

NTest::~NTest()
{
    removeRunningTest(this);
    g_cur_test = nullptr;
}

void NTest::runTest()
{
    if (!_continue_loop)
        return;

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
        abort();
    }
}

void NTest::stopTest(const NError& error) {
    NLOG_ERROR("Stopping test with error: " + toString(error));
    stopTest(false);
}

void NTest::printTestName(const char* event)
{
    NLOG_INFO("*************************************");
    NLOG_INFO(std::string(event) + " " + _name);
    NLOG_INFO("*************************************");
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

void printTotalStats()
{
    // total stats
    uint32_t testsPassed = (g_runTestsCount - g_failedTestsCount);

    NLOG_INFO("Total tests : " + std::to_string(g_runTestsCount));
    NLOG_INFO("Tests passed: " + std::to_string(testsPassed) +" (");
    printPercent(cout, g_runTestsCount, testsPassed);
    NLOG_INFO("Tests failed: " + std::to_string(g_failedTestsCount) + " (");
    printPercent(cout, g_runTestsCount, g_failedTestsCount);
}

int getFailedCount()
{
    return g_failedTestsCount;
}

} // namespace Test
} // namespace Nakama
