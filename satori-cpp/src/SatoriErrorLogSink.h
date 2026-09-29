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

#include <string>

#include "SClientInterface.h"
#include "nakama-cpp/log/NLogSinkInterface.h"

namespace Satori {

class SatoriErrorLogSink : public Nakama::NLogSinkInterface {
    private:
      SClientPtr _client;
      SSessionPtr _session;

    public:
      SatoriErrorLogSink(SClientPtr client, SSessionPtr session)
          : _client(std::move(client)), _session(std::move(session)) {}

    void log(Nakama::NLogLevel level, const std::string& message, const char* func) override {
        if (level < Nakama::NLogLevel::Error) {
            return;
        }

        if (!_session) return;

        SEvent e;
        e.name = "error";
        e.value = message;
        if (func) {
            e.metadata["func"] = func;
        }
        _client->postEvent(_session, {e}, nullptr, nullptr);
    }

    void flush() override {}
};

} // namespace Satori
