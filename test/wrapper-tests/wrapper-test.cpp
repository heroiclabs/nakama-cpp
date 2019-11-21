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
        parameters.port = SERVER_PORT;
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
        if (getClientType() == ClientType_Grpc)
            client = createGrpcClient(parameters);
        else
            client = createRestClient(parameters);

        client->setErrorCallback([this](const NAKAMA_NAMESPACE::NError& error)
        {
            stopTest();
        });
    }

    void WrapperTest::authenticate(std::function<void()> callback)
    {
        auto successCallback = [this, callback](NSessionPtr session)
        {
            this->session = session;

            std::cout << "session token: " << session->getAuthToken() << std::endl;

            callback();
        };

        client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);
    }

    void WrapperTest::connect(std::function<void()> callback)
    {
        if (!client)
        {
            createWorkingClient();
            authenticate([this, callback]()
            {
                connect(callback);
            });
            return;
        }

        listener.setConnectCallback([=]()
        {
            std::cout << "connected" << std::endl;
            callback();
        });

        rtClient = client->createRtClient(SERVER_HTTP_PORT);

        rtClient->setListener(&listener);

        rtClient->connect(session, true);
    }

    void WrapperTest::tick()
    {
        if (client)
            client->tick();
        if (rtClient)
            rtClient->tick();
    }

} // namespace Test
} // namespace Nakama

#endif // BUILD_C_API
