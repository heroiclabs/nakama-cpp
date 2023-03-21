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

#include "nakama-cpp/log/NLogger.h"
#include "globals.h"

namespace Nakama {
    namespace Test {

        std::vector<NTest*> g_running_tests;
        NTest* g_cur_test = nullptr;

        // stats
        uint32_t g_runTestsCount = 0;
        uint32_t g_failedTestsCount = 0;

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

        void sleep(uint32_t ms)
        {
            std::chrono::milliseconds sleep_period(ms);
            std::this_thread::sleep_for(sleep_period);
        }

        int getFailedCount()
        {
            return g_failedTestsCount;
        }
    }
}