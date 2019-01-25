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

#include "nakama-cpp/log/NLogger.h"
#include <cstdarg>

namespace Nakama {

NLogger& NLogger::Instance()
{
    static NLogger instance;
    return instance;
}

NLogSinkPtr NLogger::getSink() const
{
    return _sink;
}

void NLogger::setSink(NLogSinkPtr sink)
{
    _sink = sink;
}

void NLogger::setLevel(NLogLevel level)
{
    if (_sink)
    {
        _sink->setLevel(level);
    }
}

bool NLogger::shouldLog(NLogLevel level) const
{
    return _sink && _sink->getLevel() >= level;
}

void NLogger::Trace(const std::string& message)
{
    Log(NLogLevel::Trace, message);
}

void NLogger::Debug(const std::string& message)
{
    Log(NLogLevel::Debug, message);
}

void NLogger::Info(const std::string& message)
{
    Log(NLogLevel::Info, message);
}

void NLogger::Warn(const std::string& message)
{
    Log(NLogLevel::Warn, message);
}

void NLogger::Error(const std::string& message)
{
    Log(NLogLevel::Error, message);
}

void NLogger::Fatal(const std::string& message)
{
    Log(NLogLevel::Fatal, message);
}

void NLogger::Log(NLogLevel level, const std::string & message)
{
    if (Instance().shouldLog(level))
    {
        Instance().log(NLogMessage(message, level));
    }
}

void NLogger::Format(NLogLevel level, const char* format, ...)
{
    if (Instance().shouldLog(level))
    {
        va_list args, argsCpy;

        va_start(args, format);
        va_copy(argsCpy, args);
        size_t len = std::vsnprintf(NULL, 0, format, argsCpy);
        va_end(args);

        if (len > 0)
        {
            std::string str;
            str.resize(len + 1);
            va_start(args, format);
            std::vsnprintf(&str[0], len + 1, format, args);
            va_end(args);

            Instance().log(NLogMessage(str, level));
        }
    }
}

void NLogger::log(const NLogMessage & msg)
{
    if (_sink)
    {
        _sink->log(msg);
    }
}

}
