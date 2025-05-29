/*
 * Copyright 2023 The Nakama Authors
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

#include "BaseClient.h"
#include "../core-rt/NRtClient.h"
#include <future>
#include <nakama-cpp/NClientInterface.h>
#include <nakama-cpp/NException.h>
#include <nakama-cpp/log/NLogger.h>
#include <nakama-cpp/realtime/NWebsocketsFactory.h>

namespace Nakama {

#if defined(HAVE_DEFAULT_RT_TRANSPORT_FACTORY)
NRtClientPtr BaseClient::createRtClient() { return createRtClient(createDefaultWebsocket(_platformParams)); }
#endif

NRtClientPtr BaseClient::createRtClient(NRtTransportPtr transport) {
  RtClientParameters parameters;
  parameters.host = _host;
  parameters.port = _port;
  parameters.ssl = _ssl;
  parameters.platformParams = _platformParams;

  if (!transport) {
    NLOG_ERROR("No websockets transport passed. Please set transport.");
    return nullptr;
  }

  NRtClientPtr client(new NRtClient(transport, parameters.host, parameters.port, parameters.ssl));
  return client;
}

std::future<NSessionPtr> BaseClient::authenticateDeviceAsync(
    const std::string& id,
    const opt::optional<std::string>& username,
    const opt::optional<bool>& create,
    const NStringMap& vars) {

  auto promise = std::make_shared<std::promise<NSessionPtr>>();

  authenticateDevice(
      id, username, create, vars, [=](NSessionPtr session) { promise->set_value(session); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NSessionPtr> BaseClient::authenticateEmailAsync(
    const std::string& email,
    const std::string& password,
    const std::string& username,
    bool create,
    const NStringMap& vars) {
  auto promise = std::make_shared<std::promise<NSessionPtr>>();

  authenticateEmail(
      email, password, username, create, vars, [=](NSessionPtr session) { promise->set_value(session); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NSessionPtr> BaseClient::authenticateFacebookAsync(
    const std::string& accessToken,
    const std::string& username,
    bool create,
    bool importFriends,
    const NStringMap& vars) {
  auto promise = std::make_shared<std::promise<NSessionPtr>>();

  authenticateFacebook(
      accessToken, username, create, importFriends, vars, [=](NSessionPtr session) { promise->set_value(session); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NSessionPtr> BaseClient::authenticateGoogleAsync(
    const std::string& accessToken,
    const std::string& username,
    bool create,
    const NStringMap& vars) {
  auto promise = std::make_shared<std::promise<NSessionPtr>>();

  authenticateGoogle(
      accessToken, username, create, vars, [=](NSessionPtr session) { promise->set_value(session); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NSessionPtr> BaseClient::authenticateGameCenterAsync(
    const std::string& playerId,
    const std::string& bundleId,
    NTimestamp timestampSeconds,
    const std::string& salt,
    const std::string& signature,
    const std::string& publicKeyUrl,
    const std::string& username,
    bool create,
    const NStringMap& vars) {
  auto promise = std::make_shared<std::promise<NSessionPtr>>();

  authenticateGameCenter(
      playerId, bundleId, timestampSeconds, salt, signature, publicKeyUrl, username, create, vars,
      [=](NSessionPtr session) { promise->set_value(session); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NSessionPtr> BaseClient::authenticateAppleAsync(
    const std::string& token,
    const std::string& username,
    bool create,
    const NStringMap& vars) {
  auto promise = std::make_shared<std::promise<NSessionPtr>>();

  authenticateApple(
      token, username, create, vars, [=](NSessionPtr session) { promise->set_value(session); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NSessionPtr> BaseClient::authenticateCustomAsync(
    const std::string& id,
    const std::string& username,
    bool create,
    const NStringMap& vars) {
  auto promise = std::make_shared<std::promise<NSessionPtr>>();

  authenticateCustom(
      id, username, create, vars, [=](NSessionPtr session) { promise->set_value(session); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NSessionPtr> BaseClient::authenticateSteamAsync(
    const std::string& token,
    const std::string& username,
    bool create,
    const NStringMap& vars) {
  auto promise = std::make_shared<std::promise<NSessionPtr>>();

  authenticateSteam(
      token, username, create, vars, [=](NSessionPtr session) { promise->set_value(session); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NSessionPtr> BaseClient::authenticateRefreshAsync(NSessionPtr session) {
  auto promise = std::make_shared<std::promise<NSessionPtr>>();

  authenticateRefresh(
      session, [=](NSessionPtr session) { promise->set_value(session); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::linkFacebookAsync(
    NSessionPtr session,
    const std::string& accessToken,
    const opt::optional<bool>& importFriends) {
  auto promise = std::make_shared<std::promise<void>>();

  linkFacebook(
      session, accessToken, importFriends, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void>
BaseClient::linkEmailAsync(NSessionPtr session, const std::string& email, const std::string& password) {
  auto promise = std::make_shared<std::promise<void>>();

  linkEmail(
      session, email, password, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::linkDeviceAsync(NSessionPtr session, const std::string& id) {
  auto promise = std::make_shared<std::promise<void>>();

  linkDevice(
      session, id, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::linkGoogleAsync(NSessionPtr session, const std::string& accessToken) {
  auto promise = std::make_shared<std::promise<void>>();

  linkGoogle(
      session, accessToken, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::linkGameCenterAsync(
    NSessionPtr session,
    const std::string& playerId,
    const std::string& bundleId,
    NTimestamp timestampSeconds,
    const std::string& salt,
    const std::string& signature,
    const std::string& publicKeyUrl) {
  auto promise = std::make_shared<std::promise<void>>();

  linkGameCenter(
      session, playerId, bundleId, timestampSeconds, salt, signature, publicKeyUrl, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::linkAppleAsync(NSessionPtr session, const std::string& token) {
  auto promise = std::make_shared<std::promise<void>>();

  linkApple(
      session, token, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::linkSteamAsync(NSessionPtr session, const std::string& token) {
  auto promise = std::make_shared<std::promise<void>>();

  linkSteam(
      session, token, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::linkCustomAsync(NSessionPtr session, const std::string& id) {
  auto promise = std::make_shared<std::promise<void>>();

  linkCustom(
      session, id, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::unlinkFacebookAsync(NSessionPtr session, const std::string& accessToken) {
  auto promise = std::make_shared<std::promise<void>>();

  unlinkFacebook(
      session, accessToken, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void>
BaseClient::unlinkEmailAsync(NSessionPtr session, const std::string& email, const std::string& password) {
  auto promise = std::make_shared<std::promise<void>>();

  unlinkEmail(
      session, email, password, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::unlinkGoogleAsync(NSessionPtr session, const std::string& accessToken) {
  auto promise = std::make_shared<std::promise<void>>();

  unlinkGoogle(
      session, accessToken, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::unlinkGameCenterAsync(
    NSessionPtr session,
    const std::string& playerId,
    const std::string& bundleId,
    NTimestamp timestampSeconds,
    const std::string& salt,
    const std::string& signature,
    const std::string& publicKeyUrl) {
  auto promise = std::make_shared<std::promise<void>>();

  unlinkGameCenter(
      session, playerId, bundleId, timestampSeconds, salt, signature, publicKeyUrl, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::unlinkAppleAsync(NSessionPtr session, const std::string& token) {
  auto promise = std::make_shared<std::promise<void>>();

  unlinkApple(
      session, token, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::unlinkSteamAsync(NSessionPtr session, const std::string& token) {
  auto promise = std::make_shared<std::promise<void>>();

  unlinkSteam(
      session, token, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::unlinkDeviceAsync(NSessionPtr session, const std::string& id) {
  auto promise = std::make_shared<std::promise<void>>();

  unlinkDevice(
      session, id, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::unlinkCustomAsync(NSessionPtr session, const std::string& id) {
  auto promise = std::make_shared<std::promise<void>>();

  unlinkCustom(
      session, id, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::importFacebookFriendsAsync(
    NSessionPtr session,
    const std::string& token,
    const opt::optional<bool>& reset) {
  auto promise = std::make_shared<std::promise<void>>();

  importFacebookFriends(
      session, token, reset, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NAccount> BaseClient::getAccountAsync(NSessionPtr session) {
  auto promise = std::make_shared<std::promise<NAccount>>();

  getAccount(
      session, [=](const NAccount& account) { promise->set_value(account); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::updateAccountAsync(
    NSessionPtr session,
    const opt::optional<std::string>& username,
    const opt::optional<std::string>& displayName,
    const opt::optional<std::string>& avatarUrl,
    const opt::optional<std::string>& langTag,
    const opt::optional<std::string>& location,
    const opt::optional<std::string>& timezone) {
  auto promise = std::make_shared<std::promise<void>>();

  updateAccount(
      session, username, displayName, avatarUrl, langTag, location, timezone, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NUsers> BaseClient::getUsersAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    const std::vector<std::string>& facebookIds) {
  std::shared_ptr<std::promise<NUsers>> promise = std::make_shared<std::promise<NUsers>>();

  getUsers(
      session, ids, usernames, facebookIds, [=](const NUsers& users) { promise->set_value(users); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::addFriendsAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames) {
  std::shared_ptr<std::promise<void>> promise = std::make_shared<std::promise<void>>();

  addFriends(
      session, ids, usernames, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::deleteFriendsAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames) {
  auto promise = std::make_shared<std::promise<void>>();

  deleteFriends(
      session, ids, usernames, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::blockFriendsAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames) {
  auto promise = std::make_shared<std::promise<void>>();

  blockFriends(
      session, ids, usernames, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NFriendListPtr> BaseClient::listFriendsAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<NFriend::State>& state,
    const std::string& cursor) {
  auto promise = std::make_shared<std::promise<NFriendListPtr>>();

  listFriends(
      session, limit, state, cursor, [=](NFriendListPtr friendList) { promise->set_value(friendList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NGroup> BaseClient::createGroupAsync(
    NSessionPtr session,
    const std::string& name,
    const std::string& description,
    const std::string& avatarUrl,
    const std::string& langTag,
    bool open,
    const opt::optional<int32_t>& maxCount) {
  auto promise = std::make_shared<std::promise<NGroup>>();

  createGroup(
      session, name, description, avatarUrl, langTag, open, maxCount,
      [=](const NGroup& group) { promise->set_value(group); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::deleteGroupAsync(NSessionPtr session, const std::string& groupId) {
  auto promise = std::make_shared<std::promise<void>>();

  deleteGroup(
      session, groupId, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void>
BaseClient::addGroupUsersAsync(NSessionPtr session, const std::string& groupId, const std::vector<std::string>& ids) {
  auto promise = std::make_shared<std::promise<void>>();

  addGroupUsers(
      session, groupId, ids, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NGroupUserListPtr> BaseClient::listGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor) {
  auto promise = std::make_shared<std::promise<NGroupUserListPtr>>();

  listGroupUsers(
      session, groupId, limit, state, cursor, [=](NGroupUserListPtr groupList) { promise->set_value(groupList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void>
BaseClient::kickGroupUsersAsync(NSessionPtr session, const std::string& groupId, const std::vector<std::string>& ids) {
  auto promise = std::make_shared<std::promise<void>>();

  kickGroupUsers(
      session, groupId, ids, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::joinGroupAsync(NSessionPtr session, const std::string& groupId) {
  auto promise = std::make_shared<std::promise<void>>();

  joinGroup(
      session, groupId, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::leaveGroupAsync(NSessionPtr session, const std::string& groupId) {
  auto promise = std::make_shared<std::promise<void>>();

  leaveGroup(
      session, groupId, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NGroupListPtr>
BaseClient::listGroupsAsync(NSessionPtr session, const std::string& name, int32_t limit, const std::string& cursor) {
  auto promise = std::make_shared<std::promise<NGroupListPtr>>();

  listGroups(
      session, name, limit, cursor, [=](NGroupListPtr groups) { promise->set_value(groups); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NUserGroupListPtr> BaseClient::listUserGroupsAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor) {
  auto promise = std::make_shared<std::promise<NUserGroupListPtr>>();

  listUserGroups(
      session, limit, state, cursor, [=](NUserGroupListPtr groupList) { promise->set_value(groupList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NUserGroupListPtr> BaseClient::listUserGroupsAsync(
    NSessionPtr session,
    const std::string& userId,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor) {
  auto promise = std::make_shared<std::promise<NUserGroupListPtr>>();

  listUserGroups(
      session, userId, limit, state, cursor, [=](NUserGroupListPtr groupList) { promise->set_value(groupList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::promoteGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids) {
  auto promise = std::make_shared<std::promise<void>>();

  promoteGroupUsers(
      session, groupId, ids, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::demoteGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids) {
  auto promise = std::make_shared<std::promise<void>>();

  demoteGroupUsers(
      session, groupId, ids, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::updateGroupAsync(
    NSessionPtr session,
    const std::string& groupId,
    const opt::optional<std::string>& name,
    const opt::optional<std::string>& description,
    const opt::optional<std::string>& avatarUrl,
    const opt::optional<std::string>& langTag,
    const opt::optional<bool>& open) {
  auto promise = std::make_shared<std::promise<void>>();

  updateGroup(
      session, groupId, name, description, avatarUrl, langTag, open, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NLeaderboardRecordListPtr> BaseClient::listLeaderboardRecordsAsync(
    NSessionPtr session,
    const std::string& leaderboardId,
    const std::vector<std::string>& ownerIds,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor) {
  auto promise = std::make_shared<std::promise<NLeaderboardRecordListPtr>>();

  listLeaderboardRecords(
      session, leaderboardId, ownerIds, limit, cursor,
      [=](NLeaderboardRecordListPtr recordList) { promise->set_value(recordList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NLeaderboardRecordListPtr> BaseClient::listLeaderboardRecordsAroundOwnerAsync(
    NSessionPtr session,
    const std::string& leaderboardId,
    const std::string& ownerId,
    const opt::optional<int32_t>& limit) {
  auto promise = std::make_shared<std::promise<NLeaderboardRecordListPtr>>();

  listLeaderboardRecordsAroundOwner(
      session, leaderboardId, ownerId, limit,
      [=](NLeaderboardRecordListPtr recordList) { promise->set_value(recordList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NLeaderboardRecord> BaseClient::writeLeaderboardRecordAsync(
    NSessionPtr session,
    const std::string& leaderboardId,
    std::int64_t score,
    const opt::optional<std::int64_t>& subscore,
    const opt::optional<std::string>& metadata) {
  auto promise = std::make_shared<std::promise<NLeaderboardRecord>>();

  writeLeaderboardRecord(
      session, leaderboardId, score, subscore, metadata, [=](NLeaderboardRecord record) { promise->set_value(record); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NLeaderboardRecord> BaseClient::writeTournamentRecordAsync(
    NSessionPtr session,
    const std::string& tournamentId,
    std::int64_t score,
    const opt::optional<std::int64_t>& subscore,
    const opt::optional<std::string>& metadata) {
  auto promise = std::make_shared<std::promise<NLeaderboardRecord>>();

  writeLeaderboardRecord(
      session, tournamentId, score, subscore, metadata,
      [=](NLeaderboardRecord recordList) { promise->set_value(recordList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::deleteLeaderboardRecordAsync(NSessionPtr session, const std::string& leaderboardId) {
  auto promise = std::make_shared<std::promise<void>>();

  deleteLeaderboardRecord(
      session, leaderboardId, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NMatchListPtr> BaseClient::listMatchesAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& min_size,
    const opt::optional<int32_t>& max_size,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& label,
    const opt::optional<std::string>& query,
    const opt::optional<bool>& authoritative) {
  auto promise = std::make_shared<std::promise<NMatchListPtr>>();

  listMatches(
      session, min_size, max_size, limit, label, query, authoritative,
      [=](NMatchListPtr matchList) { promise->set_value(matchList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NNotificationListPtr> BaseClient::listNotificationsAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cacheableCursor) {
  auto promise = std::make_shared<std::promise<NNotificationListPtr>>();

  listNotifications(
      session, limit, cacheableCursor,
      [=](NNotificationListPtr notificationList) { promise->set_value(notificationList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void>
BaseClient::deleteNotificationsAsync(NSessionPtr session, const std::vector<std::string>& notificationIds) {
  auto promise = std::make_shared<std::promise<void>>();

  deleteNotifications(
      session, notificationIds, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NChannelMessageListPtr> BaseClient::listChannelMessagesAsync(
    NSessionPtr session,
    const std::string& channelId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const opt::optional<bool>& forward) {
  auto promise = std::make_shared<std::promise<NChannelMessageListPtr>>();

  listChannelMessages(
      session, channelId, limit, cursor, forward,
      [=](NChannelMessageListPtr channelMessageList) { promise->set_value(channelMessageList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NTournamentListPtr> BaseClient::listTournamentsAsync(
    NSessionPtr session,
    const opt::optional<uint32_t>& categoryStart,
    const opt::optional<uint32_t>& categoryEnd,
    const opt::optional<uint32_t>& startTime,
    const opt::optional<uint32_t>& endTime,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor) {
  auto promise = std::make_shared<std::promise<NTournamentListPtr>>();

  listTournaments(
      session, categoryStart, categoryEnd, startTime, endTime, limit, cursor,
      [=](NTournamentListPtr tournamentList) { promise->set_value(tournamentList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NTournamentRecordListPtr> BaseClient::listTournamentRecordsAsync(
    NSessionPtr session,
    const std::string& tournamentId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const std::vector<std::string>& ownerIds) {
  auto promise = std::make_shared<std::promise<NTournamentRecordListPtr>>();

  listTournamentRecords(
      session, tournamentId, limit, cursor, ownerIds,
      [=](NTournamentRecordListPtr tournamentRecordList) { promise->set_value(tournamentRecordList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NTournamentRecordListPtr> BaseClient::listTournamentRecordsAroundOwnerAsync(
    NSessionPtr session,
    const std::string& tournamentId,
    const std::string& ownerId,
    const opt::optional<int32_t>& limit) {
  auto promise = std::make_shared<std::promise<NTournamentRecordListPtr>>();

  listTournamentRecordsAroundOwner(
      session, tournamentId, ownerId, limit,
      [=](NTournamentRecordListPtr tournamentRecordList) { promise->set_value(tournamentRecordList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void> BaseClient::joinTournamentAsync(NSessionPtr session, const std::string& tournamentId) {
  auto promise = std::make_shared<std::promise<void>>();

  joinTournament(
      session, tournamentId, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NStorageObjectListPtr> BaseClient::listStorageObjectsAsync(
    NSessionPtr session,
    const std::string& collection,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor) {
  auto promise = std::make_shared<std::promise<NStorageObjectListPtr>>();

  listStorageObjects(
      session, collection, limit, cursor, [=](NStorageObjectListPtr objectList) { promise->set_value(objectList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NStorageObjectListPtr> BaseClient::listUsersStorageObjectsAsync(
    NSessionPtr session,
    const std::string& collection,
    const std::string& userId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor) {
  auto promise = std::make_shared<std::promise<NStorageObjectListPtr>>();

  listUsersStorageObjects(
      session, collection, userId, limit, cursor,
      [=](NStorageObjectListPtr objectList) { promise->set_value(objectList); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NStorageObjectAcks>
BaseClient::writeStorageObjectsAsync(NSessionPtr session, const std::vector<NStorageObjectWrite>& objects) {
  auto promise = std::make_shared<std::promise<NStorageObjectAcks>>();

  writeStorageObjects(
      session, objects, [=](const NStorageObjectAcks& acks) { promise->set_value(acks); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NStorageObjects>
BaseClient::readStorageObjectsAsync(NSessionPtr session, const std::vector<NReadStorageObjectId>& objectIds) {
  auto promise = std::make_shared<std::promise<NStorageObjects>>();

  readStorageObjects(
      session, objectIds, [=](NStorageObjects objects) { promise->set_value(objects); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<void>
BaseClient::deleteStorageObjectsAsync(NSessionPtr session, const std::vector<NDeleteStorageObjectId>& objectIds) {
  auto promise = std::make_shared<std::promise<void>>();

  deleteStorageObjects(
      session, objectIds, [=]() { promise->set_value(); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

std::future<NRpc>
BaseClient::rpcAsync(NSessionPtr session, const std::string& id, const opt::optional<std::string>& payload) {
  auto promise = std::make_shared<std::promise<NRpc>>();

  rpc(
      session, id, payload, [=](const NRpc& rpc) { promise->set_value(rpc); },
      [=](const NError& error) {
        NLOG_INFO("rpc async lambda called");
        promise->set_exception(std::make_exception_ptr<NException>(error));
      });

  return promise->get_future();
}

std::future<NRpc>
BaseClient::rpcAsync(const std::string& http_key, const std::string& id, const opt::optional<std::string>& payload) {
  auto promise = std::make_shared<std::promise<NRpc>>();

  rpc(
      http_key, id, payload, [=](const NRpc& rpc) { promise->set_value(rpc); },
      [=](const NError& error) { promise->set_exception(std::make_exception_ptr<NException>(error)); });

  return promise->get_future();
}

} // namespace Nakama
