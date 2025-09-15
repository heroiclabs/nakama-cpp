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

#include <iostream>
#include <nakama-cpp/log/NConsoleLogSink.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef WIN32
#include <Windows.h>
#endif

namespace Nakama {

using namespace std;

#ifdef __ANDROID__
static android_LogPriority androidLogPrio(NLogLevel level) {
  switch (level) {
    case NLogLevel::Fatal:
      return ANDROID_LOG_FATAL;
    case NLogLevel::Error:
      return ANDROID_LOG_ERROR;
    case NLogLevel::Warn:
      return ANDROID_LOG_WARN;
    case NLogLevel::Info:
      return ANDROID_LOG_INFO;
    case NLogLevel::Debug:
      return ANDROID_LOG_DEBUG;
    default:
      return ANDROID_LOG_UNKNOWN;
  }
}
#endif

void NConsoleLogSink::log(NLogLevel level, const std::string& message, const char* func) {
  std::string tmp;

  if (func && func[0]) {
    tmp.append("[").append(func).append("] ");
  }

  tmp.append(message).append("\n");

#ifdef __ANDROID__
  __android_log_print(androidLogPrio(level), "nakama", "%s", message.c_str());
#else
  // write to console
  std::ostream& os = (level >= NLogLevel::Error) ? std::cerr : std::cout;
  os << tmp;
#endif

#ifdef WIN32
  // write debug string so Visual Studio and DebugView will catch it
  OutputDebugStringA(tmp.c_str());
#endif
}

void NConsoleLogSink::flush() {
#if !defined(__ANDROID__)
  std::cout.flush();
  std::cerr.flush();
#endif
}

} // namespace Nakama
