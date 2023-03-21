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

#include "NCppTest.h"
#include "TaskExecutor.h"

namespace Nakama {
    namespace Test {
        NCppTest::NCppTest(const char* name, bool threadedTick) : NTest(name, threadedTick)
        {
        }

        void NCppTest::createWorkingClient()
        {
            NClientParameters parameters;
            setWorkingClientParameters(parameters);
            createClient(parameters);
        }

        NClientPtr NCppTest::createClient(const NClientParameters& parameters)
        {
            client = createDefaultClient(parameters);

            if (client)
            {
                client->setErrorCallback([this](const NError& error) { stopTest(error); });
            }
            return client;
        }

        void NCppTest::tick()
        {
            client->tick();
            TaskExecutor::instance().tick();
        }
    }
}