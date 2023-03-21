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

#pragma once

#include "NTest.h"


namespace Nakama {
    namespace Test {

        #define NTEST_ASSERT(cond)  if (!(cond)) { abortCurrentTest(__FILE__, __LINE__); }

        // currently running tests
        extern std::vector<NTest*> g_running_tests;
        extern NTest* g_cur_test;

        // stats
        extern uint32_t g_runTestsCount;
        extern uint32_t g_failedTestsCount;

        void sleep(uint32_t ms);
        int getFailedCount();
        void abortCurrentTest(const char* file, int lineno);
        void removeRunningTest(NTest* test);
        void addRunningTest(NTest* test);
        void runTestsLoop();
    }
}
