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

#include <nakama-cpp/NTypes.h>
#include <nakama-cpp/realtime/rtdata/NUserPresence.h>
#include <vector>

NAKAMA_NAMESPACE_BEGIN

    /// A set of joins and leaves on a particular channel.
    struct NChannelPresenceEvent
    {
        std::string channelId;                   ///< The channel identifier this event is for.
        std::vector<NUserPresence> joins;        ///< Presences joining the channel as part of this event, if any.
        std::vector<NUserPresence> leaves;       ///< Presences leaving the channel as part of this event, if any.
        std::string roomName;                    ///< The name of the chat room, or an empty string if this message was not sent through a chat room.
        std::string groupId;                     ///< The ID of the group, or an empty string if this message was not sent through a group channel.
        std::string userIdOne;                   ///< The ID of the first DM user, or an empty string if this message was not sent through a DM chat.
        std::string userIdTwo;                   ///< The ID of the second DM user, or an empty string if this message was not sent through a DM chat.
    };

NAKAMA_NAMESPACE_END
