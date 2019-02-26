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

#include "nakama-cpp/Nakama.h"
#include "test_server_config.h"
#include <chrono>
#include <thread>
#include <iostream>

namespace Nakama {
namespace Test {

using namespace std;

class NakamaSessionManager
{
public:
    NakamaSessionManager()
    {
        DefaultClientParameters parameters;

        parameters.host = SERVER_HOST;
        parameters.port = SERVER_GRPC_PORT;
        parameters.serverKey = SERVER_KEY;

        _client = createDefaultClient(parameters);
    }

    void start(const string& deviceId)
    {
        // to do: read session token from your storage
        string sessionToken;

        if (!sessionToken.empty())
        {
            // Lets check if we can restore a cached session.
            auto session = restoreSession(sessionToken);

            if (!session->isExpired())
            {
                // Session was valid and is restored now.
                _session = session;
                return;
            }
        }

        auto successCallback = [this](NSessionPtr session)
        {
            // to do: save session token in your storage
            std::cout << "session token: " << session->getAuthToken() << std::endl;
            _done = true;
        };

        auto errorCallback = [this](const NError& error)
        {
            _done = true;
        };

        _client->authenticateDevice(deviceId, opt::nullopt, opt::nullopt, successCallback, errorCallback);
    }

    void tick()
    {
        _client->tick();
    }

    bool isDone() const { return _done; }

protected:
    NClientPtr _client;
    NSessionPtr _session;
    bool _done = false;
};

void full_example()
{
    std::cout << "running full example..." << std::endl;

    NakamaSessionManager sessionManager;

    sessionManager.start("mytestdevice0001");

    std::chrono::milliseconds sleep_period(15);

    while (!sessionManager.isDone())
    {
        sessionManager.tick();

        std::this_thread::sleep_for(sleep_period);
    }
}

} // namespace Test
} // namespace Nakama
