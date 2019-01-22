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
#include "nakama-cpp/realtime/NRtDefaultClientListener.h"

namespace Nakama {
namespace Test {

class NRtClientTest : public NTest
{
public:
    NRtClientTest(const char* name) : NTest(name) {}

    std::function<void()> onRtConnect;

    NRtDefaultClientListener listener;
    NRtClientPtr rtClient;

    void runTest() override
    {
        createWorkingClient();

        listener.setConnectCallback([this]()
        {
            std::cout << "connected." << std::endl;

            if (onRtConnect)
                onRtConnect();
            else
                stopTest();
        });

        listener.setErrorCallback([this](const NRtError& error)
        {
            std::cout << "connect error: " << error.message << std::endl;
            stopTest();
        });

        auto successCallback = [this](NSessionPtr session)
        {
            std::cout << "session token: " << session->getAuthToken() << std::endl;

            rtClient = client->createRtClient();

            rtClient->setListener(&listener);

            rtClient->connect(session, true);
        };

        auto errorCallback = [this](const NError& error)
        {
            std::cout << "error: " << error.message << std::endl;
            stopTest();
        };

        client->authenticateDevice("mytestdevice0000", opt::nullopt, true, successCallback, errorCallback);

        NTest::runTest();
    }

    void tick() override
    {
        NTest::tick();

        if (rtClient)
        {
            rtClient->tick();
        }
    }
};

} // namespace Test
} // namespace Nakama
