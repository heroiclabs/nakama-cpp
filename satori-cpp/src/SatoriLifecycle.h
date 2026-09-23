/*
 * Copyright 2026 The Nakama Authors
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

#pragma once

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "InternalLowLevelSatoriAPI.h"
#include "SClientInterface.h"
#include "nakama-cpp/ClientFactory.h"
#include "nakama-cpp/satori/SatoriClientFactory.h"

namespace Satori {

namespace EventNames {
    constexpr const char* APP_LAUNCHED   = "appLaunched";
    constexpr const char* APP_FOREGROUND = "appForeground";
    constexpr const char* APP_BACKGROUND = "appBackground";
    constexpr const char* GAME_START     = "gameStart";
    constexpr const char* GAME_END       = "gameEnd";
}

/**
 * Usage:
 * 1. Externally owned client. Users are expected to manually call client->tick().
 *
 *   auto lifecycle = std::make_shared<SatoriLifecycle>(client, session);
 *   lifecycle->onAppLaunched();
 *   lifecycle->onGameStart();
 *   // In game loop:
 *   client->tick();
 *   
 *   
 * 2. Internally managed client. Ticking is done internally and client and session are owned by this class.
 * 
 *   auto lifecycle = SatoriLifecycle::createClientAndInitialize(identityId, onError);
 *   lifecycle->onAppLaunched();
 *   lifecycle->onGameStart();
 */
class SatoriLifecycle {
private:
    SClientPtr _client;
  
    SSessionPtr _session;
    mutable std::mutex _sessionMutex;
  
    std::atomic<bool> _ticking{false};
    std::thread _tickThread;

public:
    // External client: caller owns the client and tick loop.
    SatoriLifecycle(SClientPtr client, SSessionPtr session)
        : _client(std::move(client)), _session(std::move(session)) {}

    ~SatoriLifecycle() {
        _ticking = false;
        if (_tickThread.joinable()) _tickThread.join();
    }
  
    SatoriLifecycle(const SatoriLifecycle&) = delete;
    SatoriLifecycle& operator=(const SatoriLifecycle&) = delete;

    /**
     * Internally-owned client/session are created from a factory method.
     * It also connects the client and starts an internal background ticking thread.
     */
    static std::shared_ptr<SatoriLifecycle> createClientAndInitialize(
        const std::string& identityId,
        Nakama::ErrorCallback onError = nullptr)
    {
        // TODO: fetch from config
        Nakama::NClientParameters params;
        params.serverKey = "95e22293-6299-4758-ab6f-8d363e750b21";
        params.host = "127.0.0.1";
        params.port = 7450;
        params.ssl = false;

        auto lc = std::make_shared<SatoriLifecycle>(Satori::createRestClient(params), nullptr);
        lc->_client->authenticate(
            identityId,
            {}, {},
            [lc](SSessionPtr session) {
                {
                    std::lock_guard<std::mutex> lock(lc->_sessionMutex);
                    lc->_session = session;
                }
                lc->_client->onAppLaunched();
            },
            std::move(onError));
        lc->startTickThread();
        return lc;
    }

    SClientPtr getClient() const { return _client; }

    SSessionPtr getSession() const {
        std::lock_guard<std::mutex> lock(_sessionMutex);
        return _session;
    }
    void setSession(SSessionPtr session) {
        std::lock_guard<std::mutex> lock(_sessionMutex);
        _session = std::move(session);
    }

    std::future<void> onAppLaunched() {
        SEvent e;
        e.name = EventNames::APP_LAUNCHED;
        return _client->postEventAsync(session(), {e});
    }

    std::future<void> onAppForeground() {
        SEvent e;
        e.name = EventNames::APP_FOREGROUND;
        return _client->postEventAsync(session(), {e});
    }

    std::future<void> onAppBackground() {
        SEvent e;
        e.name = EventNames::APP_BACKGROUND;
        return _client->postEventAsync(session(), {e});
    }

    std::future<void> onGameStart(const std::string& val) {
        SEvent e;
        e.name = EventNames::GAME_START;
        e.value = val;
        return _client->postEventAsync(session(), {e});
    }

    std::future<void> onGameEnd(const std::string& val) {
        SEvent e;
        e.name = EventNames::GAME_END;
        e.value = val;
        return _client->postEventAsync(session(), {e});
    }

private:
    void startTickThread() {
        _ticking = true;
        _tickThread = std::thread([this]() {
            while (_ticking) {
                _client->tick();
                // TODO: Should the time be configurable?
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }
};

} // namespace Satori
