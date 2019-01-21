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

namespace Nakama {
namespace Test {

using namespace std;

void test_rt_joinChat()
{
    NRtClientTest test(__func__);

    test.onRtConnect = [&test]()
    {
        auto successCallback = [&test](NChannelPtr channel)
        {
            std::cout << "joined chat: " << channel->id << std::endl;

            test.stopTest(true);
        };

        auto errorCallback = [&test](const NRtError& error)
        {
            std::cout << "error: " << error.message << std::endl;
            test.stopTest();
        };

        test.rtClient->joinChat(
            "chat",
            NChannelType::ROOM,
            {},
            {},
            successCallback,
            errorCallback);
    };

    test.runTest();
}

} // namespace Test
} // namespace Nakama
