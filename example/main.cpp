/*
 * Copyright 2022 The Nakama Authors
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
#include <nakama-cpp/Nakama.h>
#include "nakama-cpp/realtime/NRtDefaultClientListener.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <optional>

#if __ANDROID__
    #include <android_native_app_glue.h>
    #include <jni.h>
#endif


#if __ANDROID__

extern "C"
{
    void android_main(struct android_app* app) {
        mainHelper();
    }
}

#else
int main() {
    mainHelper();
}

#endif

int mainHelper() {
    Nakama::NLogger::initWithConsoleSink(Nakama::NLogLevel::Debug);
    Nakama::NClientParameters params;
    params.serverKey = "defaultkey";
    params.host = "127.0.0.1";
    params.port = Nakama::DEFAULT_PORT;
    auto client = Nakama::createDefaultClient(params);
    Nakama::NRtClientPtr rtClient = nullptr;
    bool done = false;
    auto loginFailedCallback = [&done](const Nakama::NError &error) {
        NLOG_INFO("Failed to login");
        NLOG_INFO(error.message);
        done = true;
    };

    auto connectSucceededCallback = [&done]() {
        NLOG_INFO("Done connecting socket");
        done = true;
    };

    auto rtErrorCallback = [&done](const Nakama::NRtError& error) {
        NLOG_INFO("Error from socket:...")
        NLOG_INFO(error.message)
        done = true;
    };

    auto loginSucceededCallback = [&done, &connectSucceededCallback, &rtErrorCallback, &client, &rtClient](Nakama::NSessionPtr session) {
        NLOG_INFO("Login successful");
        NLOG_INFO(session->getAuthToken()); // raw JWT token
        Nakama::NRtDefaultClientListener listener;
        listener.setConnectCallback(connectSucceededCallback);
        listener.setErrorCallback(rtErrorCallback);
        rtClient = client->createRtClient();
        rtClient->setListener(&listener);
        NLOG_INFO("Connecting socket");
        rtClient->connect(session, true, Nakama::NRtClientProtocol::Json);
    };

    std::string deviceId = "e872f976-34c1-4c41-88fe-fd6aef118782";
    NLOG_INFO("Authenticating...");

    client->authenticateDevice(
            deviceId,
            Nakama::opt::nullopt,
            Nakama::opt::nullopt,
            {},
            loginSucceededCallback,
            loginFailedCallback);

    while (!done) {
        client->tick();

        if (rtClient)
        {
            rtClient->tick();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    NLOG_INFO("Press any key to continue");
    getchar();
    client->disconnect();
    return 0;
}
