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

#include "test_base.h"
#include "nakama-cpp-c-wrapper/NakamaWrapper.h"

namespace Nakama {
namespace Test {

    using namespace NAKAMA_NAMESPACE;

    class WrapperTest : public NTest
    {
    public:
        WrapperTest(const char* name) : NTest(name) {}
        ~WrapperTest();

        static void setWorkingClientParameters(NClientParameters& parameters);
        void createWorkingClient() override;
        void createClient(const NClientParameters& parameters);
        void authenticate(std::function<void()> callback);
        void connect(std::function<void()> callback);
        void tick() override;

        NClientPtr client;
        NRtClientPtr rtClient;
        NRtDefaultClientListener listener;
        NSessionPtr session;
    };

} // namespace Test
} // namespace Nakama
