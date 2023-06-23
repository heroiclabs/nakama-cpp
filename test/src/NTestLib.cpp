/*
 * Copyright 2023 The Nakama Authors
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

#include <stdint.h>
#include "NTestLib.h"
#include "globals.h"

namespace Nakama {
namespace Test {

    void test_getAccount();
    void test_authentication();
    void test_errors();
    void test_disconnect();
    void test_restoreSession();
    void test_storage();
    void test_groups();
    void test_friends();
    void test_listMatches();
    void test_realtime();
    void test_internals();

    std::ostream& printPercent(std::ostream& os, std::uint32_t totalCount, std::uint32_t count)
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


    int runAllTests(std::function<NClientPtr(Nakama::NClientParameters)> clientFactory, std::function<NRtClientPtr(Nakama::NClientPtr client)> rtClientFactory, NClientParameters parameters, std::string serverHttpKey)
    {
        NTest::ClientFactory = clientFactory;
        NTest::RtClientFactory = rtClientFactory;
        NTest::ServerHttpKey = serverHttpKey;
        NTest::NClientParameters = parameters;

        int res = 0;

        Nakama::NLogger::initWithConsoleSink(Nakama::NLogLevel::Debug);

        test_internals();

        test_authentication();
        test_getAccount();
        test_disconnect();
        test_errors();
        test_restoreSession();
        test_storage();
        test_groups();
        test_friends();
        // TODO flaky test_listMatches();
        test_realtime();

        // total stats
        std::uint32_t testsPassed = (g_runTestsCount - g_failedTestsCount);

        NLOG_INFO("Total tests : " + std::to_string(g_runTestsCount));
        NLOG_INFO("Tests passed: " + std::to_string(testsPassed) +" (");
        printPercent(std::cout, g_runTestsCount, testsPassed);
        NLOG_INFO("Tests failed: " + std::to_string(g_failedTestsCount) + " (");
        printPercent(std::cout, g_runTestsCount, g_failedTestsCount);

        return g_failedTestsCount == 0 ? 0 : -1;
    }
}
}
