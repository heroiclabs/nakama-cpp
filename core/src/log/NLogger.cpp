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

#include <nakama-cpp/log/NConsoleLogSink.h>
#include <nakama-cpp/log/NLogger.h>

namespace Nakama {

static NLogSinkPtr _sink;

static bool shouldLog(NLogLevel level) { return _sink && _sink->getLevel() <= level; }

void NLogger::initWithConsoleSink(NLogLevel level) {
  setSink(std::make_shared<NConsoleLogSink>());
  _sink->setLevel(level);
}

void NLogger::init(NLogSinkPtr sink, NLogLevel level) {
  setSink(sink);

  if (sink) {
    sink->setLevel(level);
  }
}

NLogSinkPtr NLogger::getSink() { return _sink; }

void NLogger::setSink(NLogSinkPtr sink) {
  if (_sink) {
    _sink->flush();
  }

  _sink = std::move(sink);
}

void NLogger::setLevel(NLogLevel level) {
  if (_sink) {
    _sink->setLevel(level);
  }
}

void NLogger::Debug(const std::string& message, const char* module_name, const char* func) {
  Log(NLogLevel::Debug, message, module_name, func);
}

void NLogger::Info(const std::string& message, const char* module_name, const char* func) {
  Log(NLogLevel::Info, message, module_name, func);
}

void NLogger::Warn(const std::string& message, const char* module_name, const char* func) {
  Log(NLogLevel::Warn, message, module_name, func);
}

void NLogger::Error(const std::string& message, const char* module_name, const char* func) {
  Log(NLogLevel::Error, message, module_name, func);
}

void NLogger::Fatal(const std::string& message, const char* module_name, const char* func) {
  Log(NLogLevel::Fatal, message, module_name, func);
}

// this is final log function which sends log to sink
void NLogger::Log(NLogLevel level, const std::string& message, const char* module_name, const char* func) {
  if (shouldLog(level)) {
    if (module_name && module_name[0]) {
      std::string module_and_func(module_name);

      if (func && func[0]) {
        module_and_func += "::";
        module_and_func += func;
      }

      _sink->log(level, message, module_and_func.c_str());
    } else {
      _sink->log(level, message, func);
    }
  }
}

void NLogger::Format(NLogLevel level, const char* module_name, const char* func, const char* format, ...) {
  if (shouldLog(level)) {
    va_list args;
    va_start(args, format);
    NLogger::vFormat(level, module_name, func, format, args);
    va_end(args);
  }
}

void NLogger::vFormat(NLogLevel level, const char* module_name, const char* func, const char* format, va_list args) {
  if (shouldLog(level)) {
    va_list argsCpy;

    va_copy(argsCpy, args);
    size_t len = std::vsnprintf(nullptr, 0, format, argsCpy);
    va_end(argsCpy);

    if (len > 0) {
      std::string str;
      str.resize(len); // string will allocate space for null terminator
      std::vsnprintf(&str[0], len + 1, format, args);

      if (str.size() > 0) {
        NLogger::Log(level, str, module_name, func);
      }
    }
  }
}

void NLogger::Error(const NError& error, const char* module_name, const char* func) {
  if (shouldLog(NLogLevel::Error)) {
    Log(NLogLevel::Error, toString(error), module_name, func);
  }
}

void NLogger::Error(const NRtError& error, const char* module_name, const char* func) {
  if (shouldLog(NLogLevel::Error)) {
    Log(NLogLevel::Error, toString(error), module_name, func);
  }
}

} // namespace Nakama
