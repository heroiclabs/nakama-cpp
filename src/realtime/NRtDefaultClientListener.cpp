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

#include "nakama-cpp/realtime/NRtDefaultClientListener.h"

namespace Nakama {

    void NRtDefaultClientListener::onConnect()
    {
        if (_connectCallback)
        {
            _connectCallback();
        }
    }

    void NRtDefaultClientListener::onDisconnect()
    {
        if (_disconnectCallback)
        {
            _disconnectCallback();
        }
    }

    void NRtDefaultClientListener::onError(const NRtError & error)
    {
        if (_errorCallback)
        {
            _errorCallback(error);
        }
    }

    void NRtDefaultClientListener::onChannelMessage(const NChannelMessage & message)
    {
        if (_channelMessageCallback)
        {
            _channelMessageCallback(message);
        }
    }

    void NRtDefaultClientListener::onChannelPresence(const NChannelPresenceEvent & presence)
    {
        if (_channelPresenceCallback)
        {
            _channelPresenceCallback(presence);
        }
    }

    void NRtDefaultClientListener::onMatchmakerMatched(NMatchmakerMatchedPtr matched)
    {
        if (_matchmakerMatchedCallback)
        {
            _matchmakerMatchedCallback(matched);
        }
    }

    void NRtDefaultClientListener::onMatchData(const NMatchData & matchData)
    {
        if (_matchDataCallback)
        {
            _matchDataCallback(matchData);
        }
    }

    void NRtDefaultClientListener::onMatchPresence(const NMatchPresenceEvent & matchPresence)
    {
        if (_matchPresenceCallback)
        {
            _matchPresenceCallback(matchPresence);
        }
    }

    void NRtDefaultClientListener::onNotifications(const NNotificationList & notifications)
    {
        if (_notificationsCallback)
        {
            _notificationsCallback(notifications);
        }
    }

    void NRtDefaultClientListener::onStatusPresence(const NStatusPresenceEvent & presence)
    {
        if (_statusPresenceCallback)
        {
            _statusPresenceCallback(presence);
        }
    }

    void NRtDefaultClientListener::onStreamPresence(const NStreamPresenceEvent & presence)
    {
        if (_streamPresenceCallback)
        {
            _streamPresenceCallback(presence);
        }
    }

    void NRtDefaultClientListener::onStreamData(const NStreamData & data)
    {
        if (_streamDataCallback)
        {
            _streamDataCallback(data);
        }
    }

} // namespace Nakama
