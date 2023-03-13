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

#include "nakama-cpp/NClientInterface.h"
#include "nakama-cpp/ClientFactory.h"

namespace Nakama {

    /**
     * Base client class
     */
    class BaseClient : public NClientInterface
    {
    public:
        void setErrorCallback(ErrorCallback errorCallback) override { _defaultErrorCallback = errorCallback; }

        void setUserData(void* userData) override { _userData = userData; }
        void* getUserData() const override { return _userData; }

#if defined(BUILD_WEBSOCKET_WSLAY) && defined(BUILD_CURL_IO)
        NRtClientPtr createRtClient() override;
#endif

        NRtClientPtr createRtClient(NRtTransportPtr transport) override;

        std::future<NSessionPtr> authenticateDeviceAsync(
            const std::string& id,
            const opt::optional<std::string>& username = opt::nullopt,
            const opt::optional<bool>& create = opt::nullopt,
            const NStringMap& vars = {}
        );

        std::future<NSessionPtr> authenticateEmailAsync(
            const std::string& email,
            const std::string& password,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        );

        std::future<NSessionPtr> authenticateFacebookAsync(
            const std::string& accessToken,
            const std::string& username = std::string(),
            bool create = false,
            bool importFriends = false,
            const NStringMap& vars = {}
        );

        std::future<NSessionPtr> authenticateGoogleAsync(
            const std::string& accessToken,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        );


        std::future<NSessionPtr> authenticateGameCenterAsync(
            const std::string& playerId,
            const std::string& bundleId,
            NTimestamp timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        );

        std::future<NSessionPtr> authenticateAppleAsync(
            const std::string& token,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        );

        std::future<NSessionPtr> authenticateCustomAsync(
            const std::string& id,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        );

        std::future<NSessionPtr> authenticateSteamAsync(
            const std::string& token,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        );

        std::future<NSessionPtr> authenticateRefreshAsync(NSessionPtr session);

        std::future<void> linkFacebookAsync(
            NSessionPtr session,
            const std::string& accessToken,
            const opt::optional<bool>& importFriends = opt::nullopt
        );

        std::future<void> linkEmailAsync(
            NSessionPtr session,
            const std::string& email,
            const std::string& password
        );

        std::future<void> linkDeviceAsync(
            NSessionPtr session,
            const std::string& id
        );

        std::future<void> linkGoogleAsync(
            NSessionPtr session,
            const std::string& accessToken
        );

        std::future<void> linkGameCenterAsync(
            NSessionPtr session,
            const std::string& playerId,
            const std::string& bundleId,
            NTimestamp timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl
        );

        std::future<void> linkAppleAsync(
            NSessionPtr session,
            const std::string& token
        );

        std::future<void> linkSteamAsync(
            NSessionPtr session,
            const std::string& token
        );

        std::future<void> linkCustomAsync(
            NSessionPtr session,
            const std::string& id
        );

        std::future<void> unlinkFacebookAsync(
            NSessionPtr session,
            const std::string& accessToken
        );

        std::future<void> unlinkEmailAsync(
            NSessionPtr session,
            const std::string& email,
            const std::string& password
        );

        std::future<void> unlinkGoogleAsync(
            NSessionPtr session,
            const std::string& accessToken
        );

        std::future<void> unlinkGameCenterAsync(
            NSessionPtr session,
            const std::string& playerId,
            const std::string& bundleId,
            NTimestamp timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl
        );

        std::future<void> unlinkAppleAsync(
            NSessionPtr session,
            const std::string& token
        );

        std::future<void> unlinkSteamAsync(
            NSessionPtr session,
            const std::string& token
        );

        std::future<void> unlinkDeviceAsync(
            NSessionPtr session,
            const std::string& id
        );

        std::future<void> unlinkCustomAsync(
            NSessionPtr session,
            const std::string& id
        );

        std::future<void> importFacebookFriendsAsync(
            NSessionPtr session,
            const std::string& token,
            const opt::optional<bool>& reset = opt::nullopt
        );

        std::future<const NAccount&> getAccountAsync(
            NSessionPtr session
        );;

        std::future<void> updateAccount(
            NSessionPtr session,
            const opt::optional<std::string>& username    = opt::nullopt,
            const opt::optional<std::string>& displayName = opt::nullopt,
            const opt::optional<std::string>& avatarUrl   = opt::nullopt,
            const opt::optional<std::string>& langTag     = opt::nullopt,
            const opt::optional<std::string>& location    = opt::nullopt,
            const opt::optional<std::string>& timezone    = opt::nullopt
        );

        std::future<const NUsers&> getUsersAsync(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {},
            const std::vector<std::string>& facebookIds = {}
        );

        std::future<void> addFriendsAsync(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {}
        );

        std::future<void> deleteFriendsAsync(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {},
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        );

        std::future<void> blockFriends(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {}
        );

        std::future<NFriendListPtr> listFriendsAsync(
            NSessionPtr session,
            const opt::optional<int32_t>& limit,
            const opt::optional<NFriend::State>& state,
            const std::string& cursor = ""
        );

        std::future<const NGroup&> createGroupAsync(
            NSessionPtr session,
            const std::string& name,
            const std::string& description = "",
            const std::string& avatarUrl = "",
            const std::string& langTag = "",
            bool open = false,
            const opt::optional<int32_t>& maxCount = {}
        );

        std::future<void> deleteGroupAsync(
            NSessionPtr session,
            const std::string& groupId
        );

        std::future<void> addGroupUsersAsync(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids
        );

        std::future<NGroupUserListPtr> listGroupUsersAsync(
            NSessionPtr session,
            const std::string& groupId,
            const opt::optional<int32_t>& limit,
            const opt::optional<NUserGroupState>& state,
            const std::string& cursor = ""
        );

        std::future<void> kickGroupUsersAsync(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids
        );

        std::future<void> joinGroupAsync(
            NSessionPtr session,
            const std::string& groupId
        );

        std::future<void> leaveGroupAsync(
            NSessionPtr session,
            const std::string& groupId
        );

        std::future<NGroupListPtr> listGroupsAsync(
            NSessionPtr session,
            const std::string& name,
            int32_t limit = 0,
            const std::string& cursor = ""
        );

        std::future<NUserGroupListPtr> listUserGroupsAsync(
            NSessionPtr session,
            const opt::optional<int32_t>& limit,
            const opt::optional<NUserGroupState>& state,
            const std::string& cursor = ""
        );

        std::future<NUserGroupListPtr> listUserGroupsAsync(
            NSessionPtr session,
            const std::string& userId,
            const opt::optional<int32_t>& limit,
            const opt::optional<NUserGroupState>& state,
            const std::string& cursor = "",
            std::function<void(NUserGroupListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        );

        std::future<void> promoteGroupUsersAsync(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids
        );

        std::future<void> demoteGroupUsersAsync(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids
        );

        std::future<void> updateGroupAsync(
            NSessionPtr session,
            const std::string& groupId,
            const opt::optional<std::string>& name = opt::nullopt,
            const opt::optional<std::string>& description = opt::nullopt,
            const opt::optional<std::string>& avatarUrl = opt::nullopt,
            const opt::optional<std::string>& langTag = opt::nullopt,
            const opt::optional<bool>& open = opt::nullopt
        );

        std::future<NLeaderboardRecordListPtr> listLeaderboardRecordsAsync(
            NSessionPtr session,
            const std::string& leaderboardId,
            const std::vector<std::string>& ownerIds = {},
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt
        );

        std::future<NLeaderboardRecordListPtr> listLeaderboardRecordsAroundOwnerAsync(
            NSessionPtr session,
            const std::string& leaderboardId,
            const std::string& ownerId,
            const opt::optional<int32_t>& limit = opt::nullopt
        );

        std::future<NLeaderboardRecord> writeLeaderboardRecordAsync(
            NSessionPtr session,
            const std::string& leaderboardId,
            int64_t score,
            const opt::optional<int64_t>& subscore = opt::nullopt,
            const opt::optional<std::string>& metadata = opt::nullopt
        );

        std::future<NLeaderboardRecord> writeTournamentRecordAsync(
            NSessionPtr session,
            const std::string& tournamentId,
            int64_t score,
            const opt::optional<int64_t>& subscore = opt::nullopt,
            const opt::optional<std::string>& metadata = opt::nullopt
        );

        std::future<void> deleteLeaderboardRecordAsync(
            NSessionPtr session,
            const std::string& leaderboardId
        );

        std::future<NMatchListPtr> listMatchesAsync(
            NSessionPtr session,
            const opt::optional<int32_t>& min_size = opt::nullopt,
            const opt::optional<int32_t>& max_size = opt::nullopt,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& label = opt::nullopt,
            const opt::optional<std::string>& query = opt::nullopt,
            const opt::optional<bool>& authoritative = opt::nullopt
        );

        std::future<NNotificationListPtr> listNotificationsAsync(
            NSessionPtr session,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cacheableCursor = opt::nullopt
        );

        std::future<void> deleteNotificationsAsync(
            NSessionPtr session,
            const std::vector<std::string>& notificationIds
        );

        std::future<NChannelMessageListPtr> listChannelMessagesAsync(
            NSessionPtr session,
            const std::string& channelId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt,
            const opt::optional<bool>& forward = opt::nullopt
        );

        std::future<NTournamentListPtr> listTournamentsAsync(
            NSessionPtr session,
            const opt::optional<uint32_t>& categoryStart = opt::nullopt,
            const opt::optional<uint32_t>& categoryEnd = opt::nullopt,
            const opt::optional<uint32_t>& startTime = opt::nullopt,
            const opt::optional<uint32_t>& endTime = opt::nullopt,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt
        );

        std::future<NTournamentRecordListPtr> listTournamentRecordsAsync(
            NSessionPtr session,
            const std::string& tournamentId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt,
            const std::vector<std::string>& ownerIds = {}
        );

        std::future<NTournamentRecordListPtr> listTournamentRecordsAroundOwnerAsync(
            NSessionPtr session,
            const std::string& tournamentId,
            const std::string& ownerId,
            const opt::optional<int32_t>& limit = opt::nullopt
        );

        std::future<void> joinTournamentAsync(
            NSessionPtr session,
            const std::string& tournamentId
        );

        std::future<NStorageObjectListPtr> listStorageObjectsAsync(
            NSessionPtr session,
            const std::string& collection,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt
        );

        std::future<NStorageObjectListPtr> listUsersStorageObjectsAsync(
            NSessionPtr session,
            const std::string& collection,
            const std::string& userId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt
        );

        std::future<const NStorageObjectAcks&> writeStorageObjectsAsync(
            NSessionPtr session,
            const std::vector<NStorageObjectWrite>& objects
        );

        std::future<NStorageObjects> readStorageObjectsAsync(
            NSessionPtr session,
            const std::vector<NReadStorageObjectId>& objectIds,
            std::function<void(const NStorageObjects&)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        );

        std::future<NDeleteStorageObjectId> deleteStorageObjectsASync(
            NSessionPtr session,
            const std::vector<NDeleteStorageObjectId>& objectIds
        );

        std::future<const NRpc&> rpcAsync(
            NSessionPtr session,
            const std::string& id,
            const opt::optional<std::string>& payload = opt::nullopt
        );

        std::future<const NRpc&> rpcAsync(
            const std::string& http_key,
            const std::string& id,
            const opt::optional<std::string>& payload = opt::nullopt
        );

    protected:
        int _port;
        std::string _host;
        bool _ssl = false;
        std::string _basicAuthMetadata;
        ErrorCallback _defaultErrorCallback;
        void* _userData = nullptr;
        NPlatformParameters _platformParams;
    };
}
