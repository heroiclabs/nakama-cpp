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

#include "wrapper-test.h"
#include "test_serverConfig.h"
#include "nakama-cpp-c-wrapper/NakamaWrapperImpl.h"

namespace Nakama {
namespace Test {

    WrapperTest::~WrapperTest()
    {
    }

    void WrapperTest::setWorkingClientParameters(NAKAMA_NAMESPACE::NClientParameters& parameters)
    {
        parameters.host = SERVER_HOST;
        parameters.port = SERVER_GRPC_PORT;
        parameters.serverKey = SERVER_KEY;
        parameters.ssl = SERVER_SSL;
    }

    void WrapperTest::createWorkingClient()
    {
        NAKAMA_NAMESPACE::NClientParameters parameters;

        setWorkingClientParameters(parameters);

        createClient(parameters);
    }

    void WrapperTest::createClient(const NAKAMA_NAMESPACE::NClientParameters& parameters)
    {
        client = createDefaultClient(parameters);

        client->setErrorCallback([this](const NAKAMA_NAMESPACE::NError& error)
        {
            stopTest();
        });
    }

    void WrapperTest::tick()
    {
        if (client)
            client->tick();
    }

} // namespace Test
} // namespace Nakama
