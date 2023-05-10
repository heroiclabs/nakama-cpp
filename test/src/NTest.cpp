
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
#include <mutex>
#include <condition_variable>
#include "NTest.h"
#include "nakama-cpp/Nakama.h"
#include "globals.h"
#include "test_serverConfig.h"

namespace Nakama {
    namespace Test {

        NRtClientProtocol NTest::RtProtocol = NRtClientProtocol::Json;

        NTest::NTest(std::string name, bool threadedTick)
                : _name(name), _threadedTick(threadedTick), _rtTickPaused(false),
                client(createDefaultClient({SERVER_KEY, SERVER_HOST, SERVER_PORT, SERVER_SSL})),
                rtClient(client->createRtClient())
        {
            client->setErrorCallback([this](const NError& error) { stopTest(error); });
            rtClient->setListener(&listener);
            _isDone.store(false);
        }

        NTest::NTest(std::string name, NClientParameters parameters)
                : _name(name),
                client(createDefaultClient(parameters)),
                rtClient(client->createRtClient())
        {
            client->setErrorCallback([this](const NError& error) { stopTest(error); });
            rtClient->setListener(&listener);
        }

        NTest::NTest(const char* name, bool threadedTick) : NTest(std::string(name), threadedTick)
        {
        }

        NTest::~NTest()
        {
            if (_threadedTick)
            {
                _tickThread.join();

            }
        }

        void NTest::runTest()
        {

            if (_threadedTick)
            {
                _tickThread = std::thread(&NTest::runTestInternal, this);
            }
            else
            {
                runTestInternal();
            }
        }

        void NTest::runTestInternal()
        {
            if (g_runTestsCount > 0)
            {
                std::cout << std::endl << std::endl;
            }

            ++g_runTestsCount;

            printTestName("Running");

            while (!isDone())
            {
                if (!checkTimeout(50)) {
                    NLOG_INFO("Test timeout");
                    stopTest(isSucceeded());
                }

                tick();

                std::chrono::milliseconds sleep_period(50);
                std::this_thread::sleep_for(sleep_period);
            }

            NLOG_INFO("done running tst internal");

        }

        void NTest::stopTest(bool succeeded)
        {
            _testSucceeded.store(succeeded);
            _isDone.store(true);

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

        void NTest::tick()
        {
            client->tick();

            if (!_rtTickPaused)
            {
                rtClient->tick();
            }
        }
    }
}