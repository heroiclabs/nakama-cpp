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
#include "nakama-c/ClientFactory.h"

namespace Nakama {
namespace Test {

    class CTest : public NTest
    {
    public:
        CTest(const char* name) : NTest(name), client(nullptr) {}
        ~CTest();

        void createWorkingClient() override;
        void tick() override;

        NClient client;
    };

    void stopCTest(NClient client, bool succeeded = false);

} // namespace Test
} // namespace Nakama
