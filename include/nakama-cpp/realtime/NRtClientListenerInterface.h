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

#include "nakama-cpp/data/NChannelMessage.h"
#include "nakama-cpp/data/NNotificationList.h"
#include "nakama-cpp/realtime/rtdata/NRtError.h"
#include "nakama-cpp/realtime/rtdata/NChannelPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NMatchmakerMatched.h"
#include "nakama-cpp/realtime/rtdata/NMatchData.h"
#include "nakama-cpp/realtime/rtdata/NMatchPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NStatusPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NStreamPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NStreamData.h"

namespace Nakama {

    /**
     * A listener for receiving {@code NRtClientInterface} events.
     */
    class NRtClientListenerInterface
    {
    public:
        virtual ~NRtClientListenerInterface() {}

        /**
        * Called when the client socket has been connected.
        */
        virtual void onConnect() {}

        /**
        * Called when the client socket disconnects.
        */
        virtual void onDisconnect() {}

        /**
        * Called when the client receives an error.
        *
        * @param error The {@code Error} received.
        */
        virtual void onError(const NRtError& error) {}

        /**
        * Called when a new topic message has been received.
        *
        * @param message The {@code ChannelMessage} received.
        */
        virtual void onChannelMessage(const NChannelMessage& message) {}

        /**
        * Called when a new topic presence update has been received.
        *
        * @param presence The {@code ChannelPresenceEvent} received.
        */
        virtual void onChannelPresence(const NChannelPresenceEvent& presence) {}

        /**
        * Called when a matchmaking has found a match.
        *
        * @param matched The {@code MatchmakerMatched} received.
        */
        virtual void onMatchmakerMatched(NMatchmakerMatchedPtr matched) {}

        /**
        * Called when a new match data is received.
        *
        * @param matchData The {@code MatchData} received.
        */
        virtual void onMatchData(const NMatchData& matchData) {}

        /**
        * Called when a new match presence update is received.
        *
        * @param matchPresence The {@code MatchPresenceEvent} received.
        */
        virtual void onMatchPresence(const NMatchPresenceEvent& matchPresence) {}

        /**
        * Called when the client receives new notifications.
        *
        * @param notifications The list of {@code Notification} received.
        */
        virtual void onNotifications(const NNotificationList& notifications) {}

        /**
        * Called when the client receives status presence updates.
        *
        * @param presence Updated {@code StatusPresenceEvent} presence.
        */
        virtual void onStatusPresence(const NStatusPresenceEvent& presence) {}

        /**
        * Called when the client receives stream presence updates.
        *
        * @param presence Updated {@code StreamPresenceEvent} presence.
        */
        virtual void onStreamPresence(const NStreamPresenceEvent& presence) {}

        /**
        * Called when the client receives stream data.
        *
        * @param data Stream {@code StreamData} data received.
        */
        virtual void onStreamData(const NStreamData& data) {}
    };

    using NRtClientListenerPtr = std::shared_ptr<NRtClientListenerInterface>;
}
