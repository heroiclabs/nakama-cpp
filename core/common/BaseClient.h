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

#include "nakama-cpp/ClientFactory.h"
#include "nakama-cpp/NClientInterface.h"
#include <optional>
#include <string>
#include <vector>

namespace Nakama {

/**
 * Base client class
 */
class BaseClient : public NClientInterface {
public:
  void setErrorCallback(ErrorCallback errorCallback) override { _defaultErrorCallback = errorCallback; }

  void setUserData(void* userData) override { _userData = userData; }
  void* getUserData() const override { return _userData; }

#ifdef HAVE_DEFAULT_RT_TRANSPORT_FACTORY
  NRtClientPtr createRtClient() override;
#endif

  NRtClientPtr createRtClient(NRtTransportPtr transport) override;

  std::future<NSessionPtr> authenticateDeviceAsync(
      const std::string& id,
      const std::optional<std::string>& username,
      const std::optional<bool>& create,
      const NStringMap& vars) override;

  std::future<NSessionPtr> authenticateEmailAsync(
      const std::string& email,
      const std::string& password,
      const std::string& username,
      bool create,
      const NStringMap& vars) override;

  std::future<NSessionPtr> authenticateFacebookAsync(
      const std::string& accessToken,
      const std::string& username,
      bool create,
      bool importFriends,
      const NStringMap& vars) override;

  std::future<NSessionPtr> authenticateGoogleAsync(
      const std::string& accessToken,
      const std::string& username,
      bool create,
      const NStringMap& vars) override;

  std::future<NSessionPtr> authenticateGameCenterAsync(
      const std::string& playerId,
      const std::string& bundleId,
      NTimestamp timestampSeconds,
      const std::string& salt,
      const std::string& signature,
      const std::string& publicKeyUrl,
      const std::string& username,
      bool create,
      const NStringMap& vars) override;

  std::future<NSessionPtr> authenticateAppleAsync(
      const std::string& token,
      const std::string& username,
      bool create,
      const NStringMap& vars) override;

  std::future<NSessionPtr> authenticateCustomAsync(
      const std::string& id,
      const std::string& username,
      bool create,
      const NStringMap& vars) override;

  std::future<NSessionPtr> authenticateSteamAsync(
      const std::string& token,
      const std::string& username,
      bool create,
      const NStringMap& vars) override;

  std::future<NSessionPtr> authenticateRefreshAsync(NSessionPtr session, const NStringMap& vars) override;

  std::future<void> sessionLogoutAsync(NSessionPtr session) override;

  std::future<void> linkFacebookAsync(
      NSessionPtr session,
      const std::string& accessToken,
      const std::optional<bool>& importFriends) override;

  std::future<void> linkEmailAsync(NSessionPtr session, const std::string& email, const std::string& password) override;

  std::future<void> linkDeviceAsync(NSessionPtr session, const std::string& id) override;

  std::future<void> linkGoogleAsync(NSessionPtr session, const std::string& accessToken) override;

  std::future<void> linkGameCenterAsync(
      NSessionPtr session,
      const std::string& playerId,
      const std::string& bundleId,
      NTimestamp timestampSeconds,
      const std::string& salt,
      const std::string& signature,
      const std::string& publicKeyUrl) override;

  std::future<void> linkAppleAsync(NSessionPtr session, const std::string& token) override;

  std::future<void> linkSteamAsync(NSessionPtr session, const std::string& token) override;

  std::future<void> linkCustomAsync(NSessionPtr session, const std::string& id) override;

  std::future<void> unlinkFacebookAsync(NSessionPtr session, const std::string& accessToken) override;

  std::future<void>
  unlinkEmailAsync(NSessionPtr session, const std::string& email, const std::string& password) override;

  std::future<void> unlinkGoogleAsync(NSessionPtr session, const std::string& accessToken) override;

  std::future<void> unlinkGameCenterAsync(
      NSessionPtr session,
      const std::string& playerId,
      const std::string& bundleId,
      NTimestamp timestampSeconds,
      const std::string& salt,
      const std::string& signature,
      const std::string& publicKeyUrl) override;

  std::future<void> unlinkAppleAsync(NSessionPtr session, const std::string& token) override;

  std::future<void> unlinkSteamAsync(NSessionPtr session, const std::string& token) override;

  std::future<void> unlinkDeviceAsync(NSessionPtr session, const std::string& id) override;

  std::future<void> unlinkCustomAsync(NSessionPtr session, const std::string& id) override;

  std::future<void> importFacebookFriendsAsync(
      NSessionPtr session,
      const std::string& token,
      const std::optional<bool>& reset) override;

  std::future<NAccount> getAccountAsync(NSessionPtr session) override;

  std::future<void> deleteAccountAsync(NSessionPtr session) override;

  std::future<void> updateAccountAsync(
      NSessionPtr session,
      const std::optional<std::string>& username,
      const std::optional<std::string>& displayName,
      const std::optional<std::string>& avatarUrl,
      const std::optional<std::string>& langTag,
      const std::optional<std::string>& location,
      const std::optional<std::string>& timezone) override;

  std::future<NUsers> getUsersAsync(
      NSessionPtr session,
      const std::vector<std::string>& ids,
      const std::vector<std::string>& usernames,
      const std::vector<std::string>& facebookIds) override;

  std::future<void> addFriendsAsync(
      NSessionPtr session,
      const std::vector<std::string>& ids,
      const std::vector<std::string>& usernames) override;

  std::future<void> deleteFriendsAsync(
      NSessionPtr session,
      const std::vector<std::string>& ids,
      const std::vector<std::string>& usernames) override;

  std::future<void> blockFriendsAsync(
      NSessionPtr session,
      const std::vector<std::string>& ids,
      const std::vector<std::string>& usernames) override;

  std::future<NFriendListPtr> listFriendsAsync(
      NSessionPtr session,
      const std::optional<int32_t>& limit,
      const std::optional<NFriend::State>& state,
      const std::string& cursor) override;
  ;

  std::future<NGroup> createGroupAsync(
      NSessionPtr session,
      const std::string& name,
      const std::string& description,
      const std::string& avatarUrl,
      const std::string& langTag,
      bool open,
      const std::optional<int32_t>& maxCount) override;

  std::future<void> deleteGroupAsync(NSessionPtr session, const std::string& groupId) override;
  ;

  std::future<void>
  addGroupUsersAsync(NSessionPtr session, const std::string& groupId, const std::vector<std::string>& ids) override;

  std::future<NGroupUserListPtr> listGroupUsersAsync(
      NSessionPtr session,
      const std::string& groupId,
      const std::optional<int32_t>& limit,
      const std::optional<NUserGroupState>& state,
      const std::string& cursor) override;

  std::future<void>
  kickGroupUsersAsync(NSessionPtr session, const std::string& groupId, const std::vector<std::string>& ids) override;

  std::future<void> joinGroupAsync(NSessionPtr session, const std::string& groupId) override;

  std::future<void> leaveGroupAsync(NSessionPtr session, const std::string& groupId) override;

  std::future<NGroupListPtr>
  listGroupsAsync(NSessionPtr session, const std::string& name, int32_t limit, const std::string& cursor)
      override;

  std::future<NUserGroupListPtr> listUserGroupsAsync(
      NSessionPtr session,
      const std::optional<int32_t>& limit,
      const std::optional<NUserGroupState>& state,
      const std::string& cursor) override;

  std::future<NUserGroupListPtr> listUserGroupsAsync(
      NSessionPtr session,
      const std::string& userId,
      const std::optional<int32_t>& limit,
      const std::optional<NUserGroupState>& state,
      const std::string& cursor) override;

  std::future<void>
  promoteGroupUsersAsync(NSessionPtr session, const std::string& groupId, const std::vector<std::string>& ids) override;

  std::future<void>
  demoteGroupUsersAsync(NSessionPtr session, const std::string& groupId, const std::vector<std::string>& ids) override;

  std::future<void> updateGroupAsync(
      NSessionPtr session,
      const std::string& groupId,
      const std::optional<std::string>& name,
      const std::optional<std::string>& description,
      const std::optional<std::string>& avatarUrl,
      const std::optional<std::string>& langTag,
      const std::optional<bool>& open) override;

  std::future<NLeaderboardRecordListPtr> listLeaderboardRecordsAsync(
      NSessionPtr session,
      const std::string& leaderboardId,
      const std::vector<std::string>& ownerIds,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor) override;

  std::future<NLeaderboardRecordListPtr> listLeaderboardRecordsAroundOwnerAsync(
      NSessionPtr session,
      const std::string& leaderboardId,
      const std::string& ownerId,
      const std::optional<int32_t>& limit) override;

  std::future<NLeaderboardRecord> writeLeaderboardRecordAsync(
      NSessionPtr session,
      const std::string& leaderboardId,
      std::int64_t score,
      const std::optional<std::int64_t>& subscore,
      const std::optional<std::string>& metadata) override;

  std::future<NLeaderboardRecord> writeTournamentRecordAsync(
      NSessionPtr session,
      const std::string& tournamentId,
      std::int64_t score,
      const std::optional<std::int64_t>& subscore,
      const std::optional<std::string>& metadata) override;

  std::future<void> deleteLeaderboardRecordAsync(NSessionPtr session, const std::string& leaderboardId) override;

  std::future<NMatchListPtr> listMatchesAsync(
      NSessionPtr session,
      const std::optional<int32_t>& min_size,
      const std::optional<int32_t>& max_size,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& label,
      const std::optional<std::string>& query,
      const std::optional<bool>& authoritative) override;

  std::future<NNotificationListPtr> listNotificationsAsync(
      NSessionPtr session,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cacheableCursor) override;

  std::future<void>
  deleteNotificationsAsync(NSessionPtr session, const std::vector<std::string>& notificationIds) override;

  std::future<NChannelMessageListPtr> listChannelMessagesAsync(
      NSessionPtr session,
      const std::string& channelId,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor,
      const std::optional<bool>& forward) override;

  std::future<NTournamentListPtr> listTournamentsAsync(
      NSessionPtr session,
      const std::optional<uint32_t>& categoryStart,
      const std::optional<uint32_t>& categoryEnd,
      const std::optional<uint32_t>& startTime,
      const std::optional<uint32_t>& endTime,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor) override;

  std::future<NTournamentRecordListPtr> listTournamentRecordsAsync(
      NSessionPtr session,
      const std::string& tournamentId,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor,
      const std::vector<std::string>& ownerIds) override;

  std::future<NTournamentRecordListPtr> listTournamentRecordsAroundOwnerAsync(
      NSessionPtr session,
      const std::string& tournamentId,
      const std::string& ownerId,
      const std::optional<int32_t>& limit) override;

  std::future<void> joinTournamentAsync(NSessionPtr session, const std::string& tournamentId) override;

  std::future<NStorageObjectListPtr> listStorageObjectsAsync(
      NSessionPtr session,
      const std::string& collection,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor) override;

  std::future<NStorageObjectListPtr> listUsersStorageObjectsAsync(
      NSessionPtr session,
      const std::string& collection,
      const std::string& userId,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor) override;

  std::future<NStorageObjectAcks>
  writeStorageObjectsAsync(NSessionPtr session, const std::vector<NStorageObjectWrite>& objects) override;

  std::future<NStorageObjects>
  readStorageObjectsAsync(NSessionPtr session, const std::vector<NReadStorageObjectId>& objectIds) override;

  std::future<void>
  deleteStorageObjectsAsync(NSessionPtr session, const std::vector<NDeleteStorageObjectId>& objectIds) override;

  std::future<NRpc> rpcAsync(
      NSessionPtr session,
      const std::string& id,
      const std::optional<std::string>& payload) override;

  std::future<NRpc> rpcAsync(
      const std::string& http_key,
      const std::string& id,
      const std::optional<std::string>& payload) override;

protected:
  int _port = -1;
  bool _ssl = false;
  std::string _host;
  std::string _basicAuthMetadata;
  void* _userData = nullptr;
  ErrorCallback _defaultErrorCallback;
  NPlatformParameters _platformParams;
};
} // namespace Nakama
