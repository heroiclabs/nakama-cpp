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

#include "nakama-cpp/realtime/rtdata/NRtError.h"
#include <sstream>

namespace Nakama {

    const char* toString(RtErrorCode code)
    {
        switch (code)
        {
        case Nakama::RtErrorCode::CONNECT_ERROR             : return "CONNECT_ERROR";
        case Nakama::RtErrorCode::TRANSPORT_ERROR           : return "TRANSPORT_ERROR";
        case Nakama::RtErrorCode::RUNTIME_EXCEPTION         : return "RUNTIME_EXCEPTION";
        case Nakama::RtErrorCode::UNRECOGNIZED_PAYLOAD      : return "UNRECOGNIZED_PAYLOAD";
        case Nakama::RtErrorCode::MISSING_PAYLOAD           : return "MISSING_PAYLOAD";
        case Nakama::RtErrorCode::BAD_INPUT                 : return "BAD_INPUT";
        case Nakama::RtErrorCode::MATCH_NOT_FOUND           : return "MATCH_NOT_FOUND";
        case Nakama::RtErrorCode::MATCH_JOIN_REJECTED       : return "MATCH_JOIN_REJECTED";
        case Nakama::RtErrorCode::RUNTIME_FUNCTION_NOT_FOUND: return "RUNTIME_FUNCTION_NOT_FOUND";
        case Nakama::RtErrorCode::RUNTIME_FUNCTION_EXCEPTION: return "RUNTIME_FUNCTION_EXCEPTION";
        default:
            break;
        }

        return "Unknown";
    }

    std::string toString(const NRtError & error)
    {
        std::stringstream ss;

        ss << "NRtError: " << toString(error.code);

        if (!error.message.empty())
        {
            ss << std::endl << error.message;
        }

        if (!error.context.empty())
        {
            ss << std::endl << "Context:";

            for (auto it : error.context)
            {
                ss << std::endl << it.first << "=" << it.second;
            }
        }

        return ss.str();
    }
}
