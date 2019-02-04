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

#include "realtime/NRtClientProtocol_Json.h"
#include "google/protobuf/util/json_util.h"

namespace Nakama {

bool NRtClientProtocol_Json::serialize(const google::protobuf::Message& message, NBytes& output)
{
    auto status = google::protobuf::util::MessageToJsonString(message, &output);

    return status.ok();
}

bool NRtClientProtocol_Json::parse(const NBytes& input, google::protobuf::Message& message)
{
    auto status = google::protobuf::util::JsonStringToMessage(input, &message);

    return status.ok();
}

}
