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
    case ErrorCode::Unknown:
        break;
    case ErrorCode::NotFound:
        return "NotFound";
    case ErrorCode::InvalidArgument:
        return "InvalidArgument";
    /*case ErrorCode::RuntimeException:
        break;
    case ErrorCode::UnrecognizedPayload:
        break;
    case ErrorCode::MissingPayload:
        break;
    case ErrorCode::BadInput:
        break;
    case ErrorCode::AuthError:
        break;
    case ErrorCode::UserNotFound:
        break;
    case ErrorCode::UserRegisterInuse:
        break;
    case ErrorCode::UserLinkInuse:
        break;
    case ErrorCode::UserLinkProviderUnavailable:
        break;
    case ErrorCode::UserUnlinkDisallowed:
        break;
    case ErrorCode::UserHandleInuse:
        break;
    case ErrorCode::GroupNameInuse:
        break;
    case ErrorCode::StorageRejected:
        break;
    case ErrorCode::MatchNotFound:
        break;*/
    case ErrorCode::ConnectionError:
        return "ConnectionError";
    case ErrorCode::InternalError:
        return "InternalError";
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
