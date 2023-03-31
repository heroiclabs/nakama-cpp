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

#include <iostream>
#include <string>
#include <thread>
#include <nakama-cpp/NError.h>
#include "nakama-cpp/NClientInterface.h"
#include "nakama-cpp/ClientFactory.h"
#include "nakama-cpp/realtime/NRtDefaultClientListener.h"

namespace Nakama {
namespace Test {

    class NTest
    {
    public:
        NTest(const char* name, bool threadedTick = false);
        NTest(std::string name, bool threadedTick = false);
        NTest(std::string name, NClientParameters parameters);

        virtual void runTest();

        void stopTest(bool succeeded = false);
        void stopTest(const NError& error);
        void onTimeout() {};

        void setRtTickPaused(bool paused) {
            _rtTickPaused = true;
        }

        void waitUntilStop();

        void tick();
        bool checkTimeout(int timePassedMs) {
            timeoutMs -= timePassedMs;
            return timeoutMs >= 0;
        }
        bool isDone() const { return !_continue_loop; }
        bool isSucceeded() const { return _testSucceeded; }

        void setTestTimeoutMs(int ms) {
            timeoutMs = ms;
        }

        const NClientPtr client;
        const NRtClientPtr rtClient;
        NRtDefaultClientListener listener;

    protected:
        void printTestName(const char* event);

    protected:
        bool _continue_loop = true;
        bool _testSucceeded = false;
        bool _threadedTick = false;
        std::string _name;
        std::thread _tickThread;
        int timeoutMs = 60*1000;
        bool _rtTickPaused;

    private:
        void runTestInternal();
        std::mutex _mtx;
        std::condition_variable _cv;
    };

} // namespace Test
} // namespace Nakama
