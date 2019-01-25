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

#include "nakama-cpp/NError.h"
#include <sstream>
 
namespace Nakama {

const char * toString(ErrorCode code)
{
    switch (code)
    {
    case Nakama::ErrorCode::Unknown:
        break;
    /*case Nakama::ErrorCode::RuntimeException:
        break;
    case Nakama::ErrorCode::UnrecognizedPayload:
        break;
    case Nakama::ErrorCode::MissingPayload:
        break;
    case Nakama::ErrorCode::BadInput:
        break;
    case Nakama::ErrorCode::AuthError:
        break;
    case Nakama::ErrorCode::UserNotFound:
        break;
    case Nakama::ErrorCode::UserRegisterInuse:
        break;
    case Nakama::ErrorCode::UserLinkInuse:
        break;
    case Nakama::ErrorCode::UserLinkProviderUnavailable:
        break;
    case Nakama::ErrorCode::UserUnlinkDisallowed:
        break;
    case Nakama::ErrorCode::UserHandleInuse:
        break;
    case Nakama::ErrorCode::GroupNameInuse:
        break;
    case Nakama::ErrorCode::StorageRejected:
        break;
    case Nakama::ErrorCode::MatchNotFound:
        break;
    case Nakama::ErrorCode::RuntimeFunctionNotFound:
        break;
    case Nakama::ErrorCode::RuntimeFunctionException:
        break;*/
    case Nakama::ErrorCode::ConnectionError:
        return "ConnectionError";
    default:
        break;
    }

    return "Unknown";
}

std::string toString(const NError & error)
{
    std::stringstream ss;

    ss << "NError: " << toString(error.code);

    if (!error.message.empty())
    {
        ss << std::endl << error.message;
    }

    return ss.str();
}

}
