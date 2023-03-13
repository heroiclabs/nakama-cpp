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

#pragma once

#include <nakama-cpp/realtime/rtdata/NUserPresence.h>
#include <nakama-cpp/NTypes.h>
#include <nakama-cpp/NExport.h>

NAKAMA_NAMESPACE_BEGIN

    /// Realtime match data received from the server.
    struct NMatchData
    {
        std::string matchId;       ///< The match unique ID.
        NUserPresence presence;    ///< A reference to the user presence that sent this data, if any.
        std::int64_t opCode;            ///< Op code value.
        NBytes data;               ///< Data payload, if any.
    };

NAKAMA_NAMESPACE_END
