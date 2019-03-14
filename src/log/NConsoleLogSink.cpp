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

#include "nakama-cpp/log/NConsoleLogSink.h"
#include <iostream>

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

namespace Nakama {

    using namespace std;

    void NConsoleLogSink::log(NLogLevel level, const std::string& message, const char* func)
    {
        std::string tmp;

        if (func && func[0])
        {
            tmp.append("[").append(func).append("] ");
        }

        tmp.append(message).append("\n");

        // write to console
        std::ostream& os = (level >= NLogLevel::Error) ? std::cerr : std::cout;
        os << tmp;

#ifdef WIN32
        // write debug string so Visual Studio and DebugView will catch it
        OutputDebugStringA(tmp.c_str());
#endif
    }

    void NConsoleLogSink::flush()
    {
        std::cout.flush();
        std::cerr.flush();
    }

}
