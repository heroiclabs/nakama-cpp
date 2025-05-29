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

namespace Nakama {

const char* toString(ErrorCode code) {
  switch (code) {
    case ErrorCode::Unknown:
      return "Unknown";
    case ErrorCode::NotFound:
      return "NotFound";
    case ErrorCode::AlreadyExists:
      return "AlreadyExists";
    case ErrorCode::InvalidArgument:
      return "InvalidArgument";
    case ErrorCode::ConnectionError:
      return "ConnectionError";
    case ErrorCode::InternalError:
      return "InternalError";
    case ErrorCode::Unauthenticated:
      return "Unauthenticated";
    case ErrorCode::PermissionDenied:
      return "PermissionDenied";
    case ErrorCode::CancelledByUser:
      return "CancelledByUser";
    default:
      break;
  }

  return "";
}

std::string toString(const NError& error) {
  std::string str;

  str.append("NError: ").append(toString(error.code));

  if (!error.message.empty()) {
    str.append("\n").append(error.message);
  }

  return str;
}

} // namespace Nakama
