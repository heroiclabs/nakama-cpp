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

// Ideas from: https://github.com/gabime/spdlog

#pragma once

#include "nakama-cpp/log/NLogSinkInterface.h"

namespace Nakama {

    class NAKAMA_API NLogger
    {
    public:
        static NLogger& Instance();

        NLogSinkPtr getSink() const;

        void setSink(NLogSinkPtr sink);
        void setLevel(NLogLevel level);
        bool shouldLog(NLogLevel level) const;

        static void Trace(const std::string& message);
        static void Debug(const std::string& message);
        static void Info (const std::string& message);
        static void Warn (const std::string& message);
        static void Error(const std::string& message);
        static void Fatal(const std::string& message);
        static void Log(NLogLevel level, const std::string& message);
        static void Format(NLogLevel level, const char* format, ...);

    private:
        NLogger();
        ~NLogger() {}
        NLogger(NLogger const&) = delete;
        void operator=(NLogger const&) = delete;
        void log(const NLogMessage& msg);

        NLogSinkPtr _sink;
    };

}
