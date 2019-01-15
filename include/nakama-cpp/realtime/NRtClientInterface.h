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

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include "nakama-cpp/NSessionInterface.h"
#include "nakama-cpp/realtime/NRtClientListenerInterface.h"
#include "nakama-cpp/realtime/rtdata/NChannel.h"

namespace Nakama {

    /**
     * A real-time client interface to interact with Nakama server.
     */
    class NRtClientInterface
    {
    public:
        virtual ~NRtClientInterface() {}

        /**
         * Close the connection with the server.
         */
        virtual void disconnect() = 0;

        /**
         * Pumps requests queue in your thread.
         * Call it periodically, each 50 ms is ok.
         */
        virtual void tick() = 0;

        /**
        * Set events listener
        *
        * @param listener The listener of client events.
        */
        virtual void setListener(NRtClientListenerInterface& listener) = 0;

        /**
        * Connect to the server.
        *
        * @param session The session of the user.
        * @param createStatus True if the socket should show the user as online to others.
        */
        virtual void connect(NSessionPtr session, bool createStatus) = 0;

        /**
        * Join a chat channel on the server.
        *
        * @param target The target channel to join.
        * @param type The type of channel to join.
        * @param persistence True if chat messages should be stored.
        * @param hidden True if the user should be hidden on the channel.
        */
        virtual void joinChat(
            const std::string& target,
            NChannelType type,
            const opt::optional<bool>& persistence,
            const opt::optional<bool>& hidden,
            std::function<void (NChannelPtr)>
        ) = 0;
    };

    using NRtClientPtr = std::shared_ptr<NRtClientInterface>;
}
