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

#include "nakama-c/log/NLogger.h"
#include "nakama-cpp/log/NLogger.h"

NAKAMA_NAMESPACE_BEGIN

class NLogSinkC : public NLogSinkInterface
{
public:
    NLogSinkC(::NLogSink sink) : _sink(sink) {}

    void log(NLogLevel level, const std::string& message, const char* func = nullptr) override
    {
        if (_sink)
            _sink((eNLogLevel)level, message.c_str(), func);
    }

    void flush() override {}

private:
    ::NLogSink _sink = nullptr;
};

static NLogSinkPtr g_logSink;

NAKAMA_NAMESPACE_END

extern "C" {

void NLogger_initWithConsoleSink(eNLogLevel level)
{
    Nakama::NLogger::initWithConsoleSink((Nakama::NLogLevel)level);
}

void NLogger_init(NLogSink sink, eNLogLevel level)
{
    NLogger_setSink(sink);
    NLogger_setLevel(level);
}

void NLogger_setSink(NLogSink sink)
{
    if (sink)
    {
        Nakama::g_logSink.reset(new Nakama::NLogSinkC(sink));
    }
    else
        Nakama::g_logSink.reset();

    Nakama::NLogger::setSink(Nakama::g_logSink);
}

void NLogger_setLevel(eNLogLevel level)
{
    Nakama::NLogger::setLevel((Nakama::NLogLevel)level);
}

void NLogger_log(eNLogLevel level, const char* message, const char* module_name, const char* func)
{
    Nakama::NLogger::Log((Nakama::NLogLevel)level, message, module_name, func);
}

void NLogger_vformat(eNLogLevel level, const char* module_name, const char* func, const char* format, va_list args)
{
    Nakama::NLogger::vFormat((Nakama::NLogLevel)level, module_name, func, format, args);
}

} // extern "C"
