
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

#include <iostream>
#include "NTest.h"
#include "nakama-cpp/Nakama.h"
#include "globals.h"
#include "test_serverConfig.h"

namespace Nakama {
    namespace Test {

        NTest::NTest(const char * name, bool threadedTick)
                : _name(name), _threadedTick(threadedTick)
        {
        }

        void NTest::runTest()
        {
            if (_threadedTick)
            {
                _tickThread = std::thread(&NTest::runTestInternal, this);
                _tickThread.join();
            }
            else
            {
                runTestInternal();
            }
        }


        void NTest::runTestInternal()
        {
            if (!_continue_loop)
                return;

            if (g_runTestsCount > 0)
                std::cout << std::endl << std::endl;

            ++g_runTestsCount;

            printTestName("Running");

            while (!isDone())
            {
                if (!checkTimeout(50)) {
                    onTimeout();
                    NLOG_INFO("Test timeout");
                    stopTest(isSucceeded());
                }

                tick();

                std::chrono::milliseconds sleep_period(50);
                std::this_thread::sleep_for(sleep_period);
            }
        }

        void NTest::stopTest(bool succeeded)
        {
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

        void NTest::createWorkingClient()
        {
            NClientParameters parameters;
            parameters.host      = SERVER_HOST;
            parameters.port      = SERVER_PORT;
            parameters.serverKey = SERVER_KEY;
            parameters.ssl       = SERVER_SSL;
            createClient(parameters);
        }

        void NTest::createClient(const NClientParameters& parameters)
        {
            client = createDefaultClient(parameters);
            client->setErrorCallback([this](const NError& error) { stopTest(error); });
            rtClient = client->createRtClient();
            rtClient->
        }

        void NTest::tick()
        {
            client->tick();
            rtClient->tick();
        }
    }
}