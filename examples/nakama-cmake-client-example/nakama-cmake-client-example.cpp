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
#include <iostream>
#include <thread>

using namespace Nakama;
using namespace std;

class NakamaSessionManager
{
public:
    NakamaSessionManager()
    {
        DefaultClientParameters parameters;

        // set server end point
        parameters.serverKey = "defaultkey";
        parameters.host      = "127.0.0.1";
        parameters.port      = 7349;

        _client = createDefaultClient(parameters);
        _rtClient = _client->createRtClient();
        
        NStringMap smap;
        NStringDoubleMap dmap;
        
        smap.emplace("key", "value");
        dmap.emplace("key", 2.0);
        
        _rtClient->addMatchmaker({}, {}, {}, smap, dmap);
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
                cout << "restored session with token: " << session->getAuthToken() << endl;
                onAuthenticated();
                return;
            }
        }

        auto successCallback = [this](NSessionPtr session)
        {
            // to do: save session token in your storage
            _session = session;
            cout << "session token: " << session->getAuthToken() << endl;
            onAuthenticated();
        };

        auto errorCallback = [](const NError& error)
        {
        };

        _client->authenticateDevice(deviceId, opt::nullopt, opt::nullopt, successCallback, errorCallback);
    }

    void tick()
    {
        _client->tick();
    }
    
    void onAuthenticated()
    {
        auto successCallback = [this](const NAccount& account)
        {
            cout << "account user id: " << account.user.id << endl;
            cout << "account user created at: " << account.user.createdAt << endl;
        };
        
        auto errorCallback = [](const NError& error)
        {
        };

        _client->getAccount(_session, successCallback, errorCallback);
    }

protected:
    NClientPtr _client;
    NRtClientPtr _rtClient;
    NSessionPtr _session;
};

int main()
{
    // enable debug logs to console
    NLogger::initWithConsoleSink(NLogLevel::Debug);

    NakamaSessionManager sessionManager;

    sessionManager.start("mytestdevice0001");

    chrono::milliseconds sleep_period(50);

    // run main loop
    while (true)
    {
        sessionManager.tick();

        this_thread::sleep_for(sleep_period);
    }
}
