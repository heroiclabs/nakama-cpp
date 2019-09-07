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

class NRtClientTest : public NCppTest
{
public:
    NRtClientTest(const char* name) : NCppTest(name) {}

    std::function<void()> onRtConnect;

    NRtDefaultClientListener listener;
    NRtClientPtr rtClient;
    NSessionPtr session;
    static NRtClientProtocol protocol;

    void runTest() override;

    void tick() override;
};

} // namespace Test
} // namespace Nakama
