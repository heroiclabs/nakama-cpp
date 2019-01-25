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

namespace Nakama {

    void NConsoleLogSink::log(NLogLevel level, const std::string& message, const char* func)
    {
        std::ostream& os = (level >= NLogLevel::Error) ? std::cerr : std::cout;

        if (func == nullptr || func[0] == 0)
        {
            os << message << std::endl;
        }
        else
        {
            os << func << ": " << message << std::endl;
        }
    }

    void NConsoleLogSink::flush()
    {
        std::cout.flush();
        std::cerr.flush();
    }

}
