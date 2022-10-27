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

#include "realtime/RtClientTestBase.h"
#include "test_serverConfig.h"

#if defined(__UNREAL__)
#include "NakamaUnreal.h"
#endif

namespace Nakama {
namespace Test {

NRtClientPtr createRtClient(NClientPtr client, int32_t port)
{
#if defined(__UNREAL__)
    return Nakama::Unreal::createNakamaRtClient(client, port);
#else
    return client->createRtClient(port);
#endif
}

NRtClientProtocol NRtClientTest::protocol = NRtClientProtocol::Protobuf;

void NRtClientTest::runTest()
{
    createWorkingClient();

    listener.setConnectCallback([this]()
    {
        if (onRtConnect)
            onRtConnect();
        else
            stopTest();
    });

    listener.setDisconnectCallback([this](const NRtClientDisconnectInfo &/*info*/)
    {
        if (!isDone() && this->_stopTestOnDisconnect)
        {
            stopTest();
        }
    });

    listener.setErrorCallback([this](const NRtError& /*error*/)
    {
        stopTest();
    });

    listener.setStatusPresenceCallback([](const NStatusPresenceEvent& event)
    {
        for (auto& presence : event.joins)
        {
            std::cout << "Joined User ID: " << presence.userId << " Username: " << presence.username << " Status: " << presence.status << std::endl;
        }

        for (auto& presence : event.leaves)
        {
            std::cout << "Left User ID: " << presence.userId << " Username: " << presence.username << " Status: " << presence.status << std::endl;
        }
    });

    listener.setChannelMessageCallback([](const NChannelMessage& message)
    {
        std::cout << "Received a message on channel " << message.channelId << std::endl;
        std::cout << "Message content: " << message.content << std::endl;
    });

    auto successCallback = [this](NSessionPtr sess)
    {
        this->session = sess;

        std::cout << "session token: " << session->getAuthToken() << std::endl;

        rtClient = createRtClient(client, SERVER_HTTP_PORT);

        rtClient->setListener(&listener);

        rtClient->connect(sess, true, protocol);
    };

    client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

    NCppTest::runTest();
}

void NRtClientTest::runTest(std::function<void()> func) {
    createWorkingClient();
    auto successCallback = [this, func](NSessionPtr sess) {
        this->session = sess;
        this->rtClient = createRtClient(client, SERVER_HTTP_PORT);
        this->rtClient->setListener(&listener);
        func();
    };
    client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);
    NCppTest::runTest();
}

void NRtClientTest::tick()
{
    NCppTest::tick();

    if (rtClient && !this->_rtTickPaused)
    {
        rtClient->tick();
    }
}

void NRtClientTest::setRtTickPaused(bool paused)
{
    this->_rtTickPaused = paused;
}

void NRtClientTest::setRtStopTestOnDisconnect(bool stopTest)
{
    this->_stopTestOnDisconnect = stopTest;
}

} // namespace Test
} // namespace Nakama
