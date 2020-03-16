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

#ifdef BUILD_C_API

#include "wrapper-test.h"

namespace Nakama {
namespace Test {

using namespace std;

void wrapper_test_rt_match();

void wrapper_test_realtime_joinChat()
{
    WrapperTest test(__func__);

    test.createWorkingClient();

    // data must be JSON
    std::string json_data = "{\"msg\":\"Hello there!\"}";

    test.listener.setChannelMessageCallback([&](const NChannelMessage& msg)
    {
        test.stopTest(msg.content == json_data);
    });

    auto successCallback = [&](NSessionPtr session)
    {
        test.session = session;

        std::cout << "session token: " << session->getAuthToken() << std::endl;

        test.connect([&]()
        {
            auto successCallback = [&](NChannelPtr channel)
            {
                std::cout << "joined chat: " << channel->id << std::endl;

                auto ackCallback = [&test](const NChannelMessageAck& ack)
                {
                    std::cout << "message sent successfuly. msg id: " << ack.messageId << std::endl;
                };

                test.rtClient->writeChatMessage(
                    channel->id,
                    json_data,
                    ackCallback
                );
            };

            test.rtClient->joinChat(
                "chat",
                NChannelType::ROOM,
                {},
                {},
                successCallback);
        });
    };

    test.client->authenticateEmail("test@mail.com", "12345678", "", true, {}, successCallback);

    test.runTest();
}

void wrapper_test_realtime_buffered_sends()
{
    WrapperTest test(__func__);

    test.createWorkingClient();

    // data must be JSON
    std::string json_data = "{\"msg\":\"Hey there!\"}";
    NTimestamp sendDelay = 500; // ms
    NTimestamp sentTime = 0;
    bool first = true;

    test.listener.setChannelMessageCallback([&](const NChannelMessage& msg)
        {
            NTEST_ASSERT(msg.content == json_data);
            NTEST_ASSERT(test.rtClient->isEnabledBufferedSends() == first);
            if (first)
            {
                NTEST_ASSERT(getUnixTimestampMs() - sentTime >= sendDelay);
                test.rtClient->disableBufferedSends();
                test.rtClient->writeChatMessage(
                    msg.channelId,
                    json_data
                );
                first = false;
            }
            else
            {
                test.stopTest(true);
            }
        });

    auto successCallback = [&](NSessionPtr session)
    {
        test.session = session;

        std::cout << "session token: " << session->getAuthToken() << std::endl;

        test.connect([&]()
            {
                test.rtClient->enableBufferedSends({ 1024, sendDelay });

                auto successCallback = [&](NChannelPtr channel)
                {
                    std::cout << "joined chat: " << channel->id << std::endl;

                    auto ackCallback = [&test](const NChannelMessageAck& ack)
                    {
                        std::cout << "message sent successfuly. msg id: " << ack.messageId << std::endl;
                    };

                    sentTime = getUnixTimestampMs();
                    test.rtClient->writeChatMessage(
                        channel->id,
                        json_data,
                        ackCallback
                    );
                };

                test.rtClient->joinChat(
                    "chat",
                    NChannelType::ROOM,
                    {},
                    {},
                    successCallback);
            });
    };

    test.client->authenticateEmail("test@mail.com", "12345678", "", true, {}, successCallback);

    test.runTest();
}

void wrapper_test_realtime()
{
    wrapper_test_realtime_joinChat();
    wrapper_test_rt_match();
    wrapper_test_realtime_buffered_sends();
}

} // namespace Test
} // namespace Nakama

#endif // BUILD_C_API
