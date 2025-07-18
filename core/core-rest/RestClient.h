/*
 * Copyright 2022 The Nakama Authors
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

#include "../common/BaseClient.h"
#include <google/protobuf/message.h>
#include <set>
#include <optional>
#include <vector>
#include <string>

namespace Nakama {

struct RestReqContext {
  std::string auth;
  std::function<void()> successCallback;
  ErrorCallback errorCallback;
  google::protobuf::Message* data = nullptr;
};

/**
 * REST client (HTTP/1.1) to interact with Nakama server.
 * Don't use it directly, use `createRestClient` instead.
 */
class RestClient : public BaseClient {
public:
  explicit RestClient(const NClientParameters& parameters, NHttpTransportPtr httpClient);
  ~RestClient();

  void disconnect() override;

  void tick() override;

  void authenticateDevice(
      const std::string& id,
      const std::optional<std::string>& username,
      const std::optional<bool>& create,
      const NStringMap& vars,
      std::function<void(NSessionPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void authenticateEmail(
      const std::string& email,
      const std::string& password,
      const std::string& username,
      bool create,
      const NStringMap& vars,
      std::function<void(NSessionPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void authenticateFacebook(
      const std::string& accessToken,
      const std::string& username,
      bool create,
      bool importFriends,
      const NStringMap& vars,
      std::function<void(NSessionPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void authenticateGoogle(
      const std::string& accessToken,
      const std::string& username,
      bool create,
      const NStringMap& vars,
      std::function<void(NSessionPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void authenticateGameCenter(
      const std::string& playerId,
      const std::string& bundleId,
      NTimestamp timestampSeconds,
      const std::string& salt,
      const std::string& signature,
      const std::string& publicKeyUrl,
      const std::string& username,
      bool create,
      const NStringMap& vars,
      std::function<void(NSessionPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void authenticateApple(
      const std::string& token,
      const std::string& username,
      bool create,
      const NStringMap& vars,
      std::function<void(NSessionPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void authenticateCustom(
      const std::string& id,
      const std::string& username,
      bool create,
      const NStringMap& vars,
      std::function<void(NSessionPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void authenticateSteam(
      const std::string& token,
      const std::string& username,
      bool create,
      const NStringMap& vars,
      std::function<void(NSessionPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void authenticateRefresh(
      NSessionPtr session,
      const NStringMap& vars,
      std::function<void(NSessionPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void sessionLogout(NSessionPtr session, std::function<void()> successCallback, ErrorCallback errorCallback) override;

  void linkFacebook(
      NSessionPtr session,
      const std::string& accessToken,
      const std::optional<bool>& importFriends,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void linkEmail(
      NSessionPtr session,
      const std::string& email,
      const std::string& password,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void linkDevice(
      NSessionPtr session,
      const std::string& id,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void linkGoogle(
      NSessionPtr session,
      const std::string& accessToken,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void linkGameCenter(
      NSessionPtr session,
      const std::string& playerId,
      const std::string& bundleId,
      NTimestamp timestampSeconds,
      const std::string& salt,
      const std::string& signature,
      const std::string& publicKeyUrl,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void linkApple(
      NSessionPtr session,
      const std::string& token,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void linkSteam(
      NSessionPtr session,
      const std::string& token,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void linkCustom(
      NSessionPtr session,
      const std::string& id,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void unlinkFacebook(
      NSessionPtr session,
      const std::string& accessToken,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void unlinkEmail(
      NSessionPtr session,
      const std::string& email,
      const std::string& password,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void unlinkGoogle(
      NSessionPtr session,
      const std::string& accessToken,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void unlinkGameCenter(
      NSessionPtr session,
      const std::string& playerId,
      const std::string& bundleId,
      NTimestamp timestampSeconds,
      const std::string& salt,
      const std::string& signature,
      const std::string& publicKeyUrl,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void unlinkApple(
      NSessionPtr session,
      const std::string& token,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void unlinkSteam(
      NSessionPtr session,
      const std::string& token,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void unlinkDevice(
      NSessionPtr session,
      const std::string& id,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void unlinkCustom(
      NSessionPtr session,
      const std::string& id,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void importFacebookFriends(
      NSessionPtr session,
      const std::string& token,
      const std::optional<bool>& reset,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void getAccount(
      NSessionPtr session,
      std::function<void(const NAccount&)> successCallback,
      ErrorCallback errorCallback) override;

  void deleteAccount(NSessionPtr session, std::function<void()> successCallback, ErrorCallback errorCallback) override;

  void updateAccount(
      NSessionPtr session,
      const std::optional<std::string>& username,
      const std::optional<std::string>& displayName,
      const std::optional<std::string>& avatarUrl,
      const std::optional<std::string>& langTag,
      const std::optional<std::string>& location,
      const std::optional<std::string>& timezone,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void getUsers(
      NSessionPtr session,
      const std::vector<std::string>& ids,
      const std::vector<std::string>& usernames,
      const std::vector<std::string>& facebookIds,
      std::function<void(const NUsers&)> successCallback,
      ErrorCallback errorCallback) override;

  void addFriends(
      NSessionPtr session,
      const std::vector<std::string>& ids,
      const std::vector<std::string>& usernames,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void deleteFriends(
      NSessionPtr session,
      const std::vector<std::string>& ids,
      const std::vector<std::string>& usernames,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void blockFriends(
      NSessionPtr session,
      const std::vector<std::string>& ids,
      const std::vector<std::string>& usernames,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void listFriends(
      NSessionPtr session,
      const std::optional<int32_t>& limit,
      const std::optional<NFriend::State>& state,
      const std::string& cursor,
      std::function<void(NFriendListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void createGroup(
      NSessionPtr session,
      const std::string& name,
      const std::string& description,
      const std::string& avatarUrl,
      const std::string& langTag,
      bool open,
      const std::optional<int32_t>& maxCount,
      std::function<void(const NGroup&)> successCallback,
      ErrorCallback errorCallback) override;

  void deleteGroup(
      NSessionPtr session,
      const std::string& groupId,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void addGroupUsers(
      NSessionPtr session,
      const std::string& groupId,
      const std::vector<std::string>& ids,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void listGroupUsers(
      NSessionPtr session,
      const std::string& groupId,
      const std::optional<int32_t>& limit,
      const std::optional<NUserGroupState>& state,
      const std::string& cursor,
      std::function<void(NGroupUserListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void kickGroupUsers(
      NSessionPtr session,
      const std::string& groupId,
      const std::vector<std::string>& ids,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void joinGroup(
      NSessionPtr session,
      const std::string& groupId,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void leaveGroup(
      NSessionPtr session,
      const std::string& groupId,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void listGroups(
      NSessionPtr session,
      const std::string& name,
      int32_t limit,
      const std::string& cursor,
      std::function<void(NGroupListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void listUserGroups(
      NSessionPtr session,
      const std::optional<int32_t>& limit,
      const std::optional<NUserGroupState>& state,
      const std::string& cursor,
      std::function<void(NUserGroupListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void listUserGroups(
      NSessionPtr session,
      const std::string& userId,
      const std::optional<int32_t>& limit,
      const std::optional<NUserGroupState>& state,
      const std::string& cursor,
      std::function<void(NUserGroupListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void promoteGroupUsers(
      NSessionPtr session,
      const std::string& groupId,
      const std::vector<std::string>& ids,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void demoteGroupUsers(
      NSessionPtr session,
      const std::string& groupId,
      const std::vector<std::string>& ids,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void updateGroup(
      NSessionPtr session,
      const std::string& groupId,
      const std::optional<std::string>& name,
      const std::optional<std::string>& description,
      const std::optional<std::string>& avatarUrl,
      const std::optional<std::string>& langTag,
      const std::optional<bool>& open,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void listLeaderboardRecords(
      NSessionPtr session,
      const std::string& leaderboardId,
      const std::vector<std::string>& ownerIds,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor,
      std::function<void(NLeaderboardRecordListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void listLeaderboardRecordsAroundOwner(
      NSessionPtr session,
      const std::string& leaderboardId,
      const std::string& ownerId,
      const std::optional<int32_t>& limit,
      std::function<void(NLeaderboardRecordListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void writeLeaderboardRecord(
      NSessionPtr session,
      const std::string& leaderboardId,
      std::int64_t score,
      const std::optional<std::int64_t>& subscore,
      const std::optional<std::string>& metadata,
      std::function<void(NLeaderboardRecord)> successCallback,
      ErrorCallback errorCallback) override;

  void writeTournamentRecord(
      NSessionPtr session,
      const std::string& tournamentId,
      std::int64_t score,
      const std::optional<std::int64_t>& subscore,
      const std::optional<std::string>& metadata,
      std::function<void(NLeaderboardRecord)> successCallback,
      ErrorCallback errorCallback) override;

  void deleteLeaderboardRecord(
      NSessionPtr session,
      const std::string& leaderboardId,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void listMatches(
      NSessionPtr session,
      const std::optional<int32_t>& min_size,
      const std::optional<int32_t>& max_size,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& label,
      const std::optional<std::string>& query,
      const std::optional<bool>& authoritative,
      std::function<void(NMatchListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void listNotifications(
      NSessionPtr session,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cacheableCursor,
      std::function<void(NNotificationListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void deleteNotifications(
      NSessionPtr session,
      const std::vector<std::string>& notificationIds,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void listChannelMessages(
      NSessionPtr session,
      const std::string& channelId,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor,
      const std::optional<bool>& forward,
      std::function<void(NChannelMessageListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void listTournaments(
      NSessionPtr session,
      const std::optional<uint32_t>& categoryStart,
      const std::optional<uint32_t>& categoryEnd,
      const std::optional<uint32_t>& startTime,
      const std::optional<uint32_t>& endTime,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor,
      std::function<void(NTournamentListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void listTournamentRecords(
      NSessionPtr session,
      const std::string& tournamentId,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor,
      const std::vector<std::string>& ownerIds,
      std::function<void(NTournamentRecordListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void listTournamentRecordsAroundOwner(
      NSessionPtr session,
      const std::string& tournamentId,
      const std::string& ownerId,
      const std::optional<int32_t>& limit,
      std::function<void(NTournamentRecordListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void joinTournament(
      NSessionPtr session,
      const std::string& tournamentId,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void listStorageObjects(
      NSessionPtr session,
      const std::string& collection,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor,
      std::function<void(NStorageObjectListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void listUsersStorageObjects(
      NSessionPtr session,
      const std::string& collection,
      const std::string& userId,
      const std::optional<int32_t>& limit,
      const std::optional<std::string>& cursor,
      std::function<void(NStorageObjectListPtr)> successCallback,
      ErrorCallback errorCallback) override;

  void writeStorageObjects(
      NSessionPtr session,
      const std::vector<NStorageObjectWrite>& objects,
      std::function<void(const NStorageObjectAcks&)> successCallback,
      ErrorCallback errorCallback) override;

  void readStorageObjects(
      NSessionPtr session,
      const std::vector<NReadStorageObjectId>& objectIds,
      std::function<void(const NStorageObjects&)> successCallback,
      ErrorCallback errorCallback) override;

  void deleteStorageObjects(
      NSessionPtr session,
      const std::vector<NDeleteStorageObjectId>& objectIds,
      std::function<void()> successCallback,
      ErrorCallback errorCallback) override;

  void
  rpc(NSessionPtr session,
      const std::string& id,
      const std::optional<std::string>& payload,
      std::function<void(const NRpc&)> successCallback,
      ErrorCallback errorCallback) override;

  void
  rpc(const std::string& http_key,
      const std::string& id,
      const std::optional<std::string>& payload,
      std::function<void(const NRpc&)> successCallback,
      ErrorCallback errorCallback) override;

private:
  RestReqContext* createReqContext(google::protobuf::Message* data);
  void setBasicAuth(RestReqContext* ctx);
  void setSessionAuth(RestReqContext* ctx, NSessionPtr session);

  void sendReq(
      RestReqContext* ctx,
      NHttpReqMethod method,
      std::string&& path,
      std::string&& body,
      NHttpQueryArgs&& args = NHttpQueryArgs());

  void
  sendRpc(RestReqContext* ctx, const std::string& id, const std::optional<std::string>& payload, NHttpQueryArgs&& args);

  void onResponse(RestReqContext* reqContext, NHttpResponsePtr response);
  void reqError(RestReqContext* reqContext, const NError& error);

private:
  std::set<RestReqContext*> _reqContexts;
  NHttpTransportPtr _httpClient;
};
} // namespace Nakama
