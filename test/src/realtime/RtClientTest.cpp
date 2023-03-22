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

#include "nakama-cpp/log/NLogger.h"
#include "realtime/RtClientTest.h"
#include "test_serverConfig.h"

namespace Nakama {
namespace Test {

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
            NLOG_INFO("Joined User ID: " + presence.userId + " Username: " + presence.username + " Status: " + presence.status);
        }

        for (auto& presence : event.leaves)
        {
            NLOG_INFO("Left User ID: " + presence.userId + " Username: " + presence.username + " Status: " + presence.status);
        }
    });

    listener.setChannelMessageCallback([](const NChannelMessage& message)
    {
        NLOG_INFO("Received a message on channel " + message.channelId);
        NLOG_INFO("Message content: " + message.content);
    });

    auto successCallback = [this](NSessionPtr sess)
    {
        this->session = sess;

        NLOG_INFO("session token: " + session->getAuthToken());

        rtClient = client->createRtClient();

        rtClient->setListener(&listener);

        rtClient->connect(sess, true, protocol);
    };

    client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

    NTest::runTest();
}

void NRtClientTest::runTest(std::function<void()> func) {
    createWorkingClient();
    auto successCallback = [this, func](NSessionPtr sess) {
        this->session = sess;
        this->rtClient = client->createRtClient();
        this->rtClient->setListener(&listener);
        func();
    };
    client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);
    NTest::runTest();
}

void NRtClientTest::tick()
{
    NTest::tick();

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
