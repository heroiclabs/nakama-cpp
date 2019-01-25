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
#include "nakama-cpp/log/NConsoleLogSink.h"
#include <cstdarg>

namespace Nakama {

NLogSinkPtr NLogger::_sink;

void NLogger::initWithConsoleSink(NLogLevel level)
{
    setSink(std::make_shared<NConsoleLogSink>());
    _sink->setLevel(level);
}

void NLogger::init(NLogSinkPtr sink, NLogLevel level)
{
    setSink(sink);

    if (sink)
    {
        sink->setLevel(level);
    }
}

NLogSinkPtr NLogger::getSink()
{
    return _sink;
}

void NLogger::setSink(NLogSinkPtr sink)
{
    if (_sink)
    {
        _sink->flush();
    }

    _sink = sink;
}

void NLogger::setLevel(NLogLevel level)
{
    if (_sink)
    {
        _sink->setLevel(level);
    }
}

bool NLogger::shouldLog(NLogLevel level)
{
    return _sink && _sink->getLevel() <= level;
}

void NLogger::Debug(const std::string& message, const char* func)
{
    Log(NLogLevel::Debug, message, func);
}

void NLogger::Info(const std::string& message, const char* func)
{
    Log(NLogLevel::Info, message, func);
}

void NLogger::Warn(const std::string& message, const char* func)
{
    Log(NLogLevel::Warn, message, func);
}

void NLogger::Error(const std::string& message, const char* func)
{
    Log(NLogLevel::Error, message, func);
}

void NLogger::Fatal(const std::string& message, const char* func)
{
    Log(NLogLevel::Fatal, message, func);
}

void NLogger::Log(NLogLevel level, const std::string & message, const char* func)
{
    if (shouldLog(level))
    {
        _sink->log(level, message, func);
    }
}

void NLogger::Format(NLogLevel level, const char* func, const char* format, ...)
{
    if (shouldLog(level))
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

            _sink->log(level, str, func);
        }
    }
}

void NLogger::Error(const NError & error, const char * func)
{
    if (shouldLog(NLogLevel::Error))
    {
        _sink->log(NLogLevel::Error, toString(error), func);
    }
}

void NLogger::Error(const NRtError & error, const char * func)
{
    if (shouldLog(NLogLevel::Error))
    {
        _sink->log(NLogLevel::Error, toString(error), func);
    }
}

}
