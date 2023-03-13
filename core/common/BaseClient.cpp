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


#include "nakama-cpp/NClientInterface.h"
#include "BaseClient.h"

namespace Nakama
{
std::future<NSessionPtr> BaseClient::authenticateDeviceAsync(
    const std::string& id,
    const opt::optional<std::string>& username,
    const opt::optional<bool>& create,
    const NStringMap& vars
)
{
    std::promise<NSessionPtr> promise;

    authenticateDevice(id, username, create, vars,
        [&](NSessionPtr session) {
            promise.set_value(session);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NSessionPtr> BaseClient::authenticateEmailAsync(
    const std::string& email,
    const std::string& password,
    const std::string& username,
    bool create,
    const NStringMap& vars
)
{
    std::promise<NSessionPtr> promise;

    authenticateEmail(email, password, username, create, vars,
        [&](NSessionPtr session) {
            promise.set_value(session);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NSessionPtr> BaseClient::authenticateFacebookAsync(
    const std::string& accessToken,
    const std::string& username,
    bool create,
    bool importFriends,
    const NStringMap& vars
)
{
    std::promise<NSessionPtr> promise;

    authenticateFacebook(accessToken, username, create, importFriends, vars,
        [&](NSessionPtr session) {
            promise.set_value(session);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NSessionPtr> BaseClient::authenticateGoogleAsync(
    const std::string& accessToken,
    const std::string& username,
    bool create,
    const NStringMap& vars
)
{
    std::promise<NSessionPtr> promise;

    authenticateGoogle(accessToken, username, create, vars,
        [&](NSessionPtr session) {
            promise.set_value(session);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
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
    const NStringMap& vars
)
{
    std::promise<NSessionPtr> promise;

    authenticateGameCenter(playerId, bundleId, timestampSeconds, salt, signature, publicKeyUrl, username, create, vars,
        [&](NSessionPtr session) {
            promise.set_value(session);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NSessionPtr> BaseClient::authenticateAppleAsync(
    const std::string& token,
    const std::string& username,
    bool create,
    const NStringMap& vars
)
{
    std::promise<NSessionPtr> promise;

    authenticateApple(token, username, create, vars,
        [&](NSessionPtr session) {
            promise.set_value(session);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NSessionPtr> BaseClient::authenticateCustomAsync(
    const std::string& id,
    const std::string& username,
    bool create,
    const NStringMap& vars
)
{
    std::promise<NSessionPtr> promise;

    authenticateCustom(id, username, create, vars,
        [&](NSessionPtr session) {
            promise.set_value(session);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NSessionPtr> BaseClient::authenticateSteamAsync(
    const std::string& token,
    const std::string& username,
    bool create,
    const NStringMap& vars)
{
    std::promise<NSessionPtr> promise;

    authenticateSteam(token, username, create, vars,
        [&](NSessionPtr session) {
            promise.set_value(session);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NSessionPtr> BaseClient::authenticateRefreshAsync(NSessionPtr session)
{
    std::promise<NSessionPtr> promise;

    authenticateRefresh(session,
        [&](NSessionPtr session) {
            promise.set_value(session);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::linkFacebookAsync(
    NSessionPtr session,
    const std::string& accessToken,
    const opt::optional<bool>& importFriends
)
{
    std::promise<void> promise;

    linkFacebook(session, accessToken, importFriends,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::linkEmailAsync(
    NSessionPtr session,
    const std::string& email,
    const std::string& password
)
{
    std::promise<void> promise;

    linkEmail(session, email, password,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::linkDeviceAsync(
    NSessionPtr session,
    const std::string& id
)
{
    std::promise<void> promise;

    linkDevice(session, id,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::linkGoogleAsync(
    NSessionPtr session,
    const std::string& accessToken
)
{
    std::promise<void> promise;

    linkGoogle(session, accessToken,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::linkGameCenterAsync(
    NSessionPtr session,
    const std::string& playerId,
    const std::string& bundleId,
    NTimestamp timestampSeconds,
    const std::string& salt,
    const std::string& signature,
    const std::string& publicKeyUrl
)
{
    std::promise<void> promise;

    linkGameCenter(session, playerId, bundleId, timestampSeconds, salt, signature, publicKeyUrl,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::linkAppleAsync(
    NSessionPtr session,
    const std::string& token
)
{
    std::promise<void> promise;

    linkApple(session, token,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::linkSteamAsync(
    NSessionPtr session,
    const std::string& token
)
{
    std::promise<void> promise;

    linkSteam(session, token,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::linkCustomAsync(
    NSessionPtr session,
    const std::string& id
)
{
    std::promise<void> promise;

    linkCustom(session, id,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::unlinkFacebookAsync(
    NSessionPtr session,
    const std::string& accessToken
)
{
    std::promise<void> promise;

    unlinkFacebook(session, accessToken,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::unlinkEmailAsync(
    NSessionPtr session,
    const std::string& email,
    const std::string& password
)
{
    std::promise<void> promise;

    unlinkEmail(session, email, password,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::unlinkGoogleAsync(
    NSessionPtr session,
    const std::string& accessToken
)
{
    std::promise<void> promise;

    unlinkGoogle(session, accessToken,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::unlinkGameCenterAsync(
    NSessionPtr session,
    const std::string& playerId,
    const std::string& bundleId,
    NTimestamp timestampSeconds,
    const std::string& salt,
    const std::string& signature,
    const std::string& publicKeyUrl
)
{
    std::promise<void> promise;

    unlinkGameCenter(session, playerId, bundleId, timestampSeconds, salt, signature, publicKeyUrl,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::unlinkAppleAsync(
    NSessionPtr session,
    const std::string& token
)
{
    std::promise<void> promise;

    unlinkApple(session, token,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::unlinkSteamAsync(
    NSessionPtr session,
    const std::string& token
)
{
    std::promise<void> promise;

    unlinkSteam(session, token,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::unlinkDeviceAsync(
    NSessionPtr session,
    const std::string& id
)
{
    std::promise<void> promise;

    unlinkDevice(session, id,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::unlinkCustomAsync(
    NSessionPtr session,
    const std::string& id
)
{
    std::promise<void> promise;

    unlinkCustom(session, id,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::importFacebookFriendsAsync(
    NSessionPtr session,
    const std::string& token,
    const opt::optional<bool>& reset
)
{
    std::promise<void> promise;

    importFacebookFriends(session, token, reset,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<const NAccount&> getAccountAsync(NSessionPtr session)
{
    std::promise<const NAccount&> promise;
/*
    updateAccount(session,
        [&](const NAccount& account) {
            promise.set_value(account);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });
*/
    return promise.get_future();
}

std::future<void> BaseClient::updateAccountAsync(
    NSessionPtr session,
    const opt::optional<std::string>& username,
    const opt::optional<std::string>& displayName,
    const opt::optional<std::string>& avatarUrl,
    const opt::optional<std::string>& langTag,
    const opt::optional<std::string>& location,
    const opt::optional<std::string>& timezone)
{
    std::promise<void> promise;

    updateAccount(session, username, displayName, avatarUrl, langTag, location, timezone,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<const NUsers&> BaseClient::getUsersAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    const std::vector<std::string>& facebookIds
)
{
    std::promise<const NUsers&> promise;

    getUsers(session, ids, usernames, facebookIds,
        [&](const NUsers& users) {
            promise.set_value(users);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::addFriendsAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames
)
{
    std::promise<void> promise;

    addFriends(session, ids, usernames,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::deleteFriendsAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames
)
{
    std::promise<void> promise;

    deleteFriends(session, ids, usernames,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::blockFriendsAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames
)
{
    std::promise<void> promise;

    blockFriends(session, ids, usernames,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NFriendListPtr> BaseClient::listFriendsAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<NFriend::State>& state,
    const std::string& cursor
) {
    std::promise<NFriendListPtr> promise;

    listFriends(session, limit, state, cursor,
        [&](NFriendListPtr friendList) {
            promise.set_value(friendList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<const NGroup&> BaseClient::createGroupAsync(
    NSessionPtr session,
    const std::string& name,
    const std::string& description,
    const std::string& avatarUrl,
    const std::string& langTag,
    bool open,
    const opt::optional<int32_t>& maxCount
)
{
    std::promise<const NGroup&> promise;

    createGroup(session, name, description, avatarUrl, langTag, open, maxCount,
        [&](const NGroup& group) {
            promise.set_value(group);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::deleteGroupAsync(
    NSessionPtr session,
    const std::string& groupId
)
{
    std::promise<void> promise;

    deleteGroup(session, groupId,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::addGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids
)
{
    std::promise<void> promise;

    addGroupUsers(session, groupId, ids,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}


std::future<NGroupUserListPtr> BaseClient::listGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor
)
{
    std::promise<NGroupUserListPtr> promise;

    listGroupUsers(session, groupId, limit, state, cursor,
        [&](NGroupUserListPtr groupList) {
            promise.set_value(groupList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::kickGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids
)
{
    std::promise<void> promise;

    kickGroupUsers(session, groupId, ids,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}


std::future<void> BaseClient::joinGroupAsync(
    NSessionPtr session,
    const std::string& groupId
)
{
    std::promise<void> promise;

    joinGroup(session, groupId,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::leaveGroupAsync(
    NSessionPtr session,
    const std::string& groupId
)
{
    std::promise<void> promise;

    leaveGroup(session, groupId,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NGroupListPtr> BaseClient::listGroupsAsync(
    NSessionPtr session,
    const std::string& name,
    int32_t limit,
    const std::string& cursor
)
{
    std::promise<NGroupListPtr> promise;

    listGroups(session, name, limit, cursor,
        [&](NGroupListPtr groups) {
            promise.set_value(groups);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NUserGroupListPtr> BaseClient::listUserGroupsAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor
)
{
    std::promise<NUserGroupListPtr> promise;

    listUserGroups(session, limit, state, cursor,
        [&](NUserGroupListPtr groupList) {
            promise.set_value(groupList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NUserGroupListPtr> BaseClient::listUserGroupsAsync(
    NSessionPtr session,
    const std::string& userId,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor)
{
    std::promise<NUserGroupListPtr> promise;

    listUserGroups(session, userId, limit, state, cursor,
        [&](NUserGroupListPtr groupList) {
            promise.set_value(groupList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::promoteGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids
)
{
    std::promise<void> promise;

    promoteGroupUsers(session, groupId, ids,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::demoteGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids
)
{
    std::promise<void> promise;

    demoteGroupUsers(session, groupId, ids,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::updateGroupAsync(
    NSessionPtr session,
    const std::string& groupId,
    const opt::optional<std::string>& name,
    const opt::optional<std::string>& description,
    const opt::optional<std::string>& avatarUrl,
    const opt::optional<std::string>& langTag,
    const opt::optional<bool>& open
)
{
    std::promise<void> promise;

    updateGroup(session, groupId, name, description, avatarUrl, langTag, open,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NLeaderboardRecordListPtr> BaseClient::listLeaderboardRecordsAsync(
    NSessionPtr session,
    const std::string& leaderboardId,
    const std::vector<std::string>& ownerIds,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor
)
{
    std::promise<NLeaderboardRecordListPtr> promise;

    listLeaderboardRecords(session, leaderboardId, ownerIds, limit, cursor,
        [&](NLeaderboardRecordListPtr recordList) {
            promise.set_value(recordList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NLeaderboardRecordListPtr> BaseClient::listLeaderboardRecordsAroundOwnerAsync(
    NSessionPtr session,
    const std::string& leaderboardId,
    const std::string& ownerId,
    const opt::optional<int32_t>& limit
)
{
    std::promise<NLeaderboardRecordListPtr> promise;

    listLeaderboardRecordsAroundOwner(session, leaderboardId, ownerId, limit,
        [&](NLeaderboardRecordListPtr recordList) {
            promise.set_value(recordList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NLeaderboardRecord> writeLeaderboardRecordAsync(
    NSessionPtr session,
    const std::string& leaderboardId,
    std::int64_t score,
    const opt::optional<std::int64_t>& subscore,
    const opt::optional<std::string>& metadata
)
{
    std::promise<NLeaderboardRecord> promise;
/*
    writeLeaderboardRecord(session, leaderboardId, score, subscore, metadata,
        [&](NLeaderboardRecord record) {
            promise.set_value(record);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });
*/
    return promise.get_future();
}

std::future<NLeaderboardRecord> BaseClient::writeTournamentRecordAsync(
    NSessionPtr session,
    const std::string& tournamentId,
    std::int64_t score,
    const opt::optional<std::int64_t>& subscore,
    const opt::optional<std::string>& metadata
)
{
    std::promise<NLeaderboardRecord> promise;

    writeLeaderboardRecord(session, tournamentId, score, subscore, metadata,
        [&](NLeaderboardRecord recordList) {
            promise.set_value(recordList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::deleteLeaderboardRecordAsync(
    NSessionPtr session,
    const std::string& leaderboardId
)
{
    std::promise<void> promise;

    deleteLeaderboardRecord(session, leaderboardId,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NMatchListPtr> BaseClient::listMatchesAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& min_size,
    const opt::optional<int32_t>& max_size,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& label,
    const opt::optional<std::string>& query,
    const opt::optional<bool>& authoritative
)
{
    std::promise<NMatchListPtr> promise;

    listMatches(session, min_size, max_size, limit, label, query, authoritative,
        [&](NMatchListPtr matchList) {
            promise.set_value(matchList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NNotificationListPtr> BaseClient::listNotificationsAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cacheableCursor
)
{
    std::promise<NNotificationListPtr> promise;

    listNotifications(session, limit, cacheableCursor,
        [&](NNotificationListPtr notificationList) {
            promise.set_value(notificationList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::deleteNotificationsAsync(
    NSessionPtr session,
    const std::vector<std::string>& notificationIds
)
{
    std::promise<void> promise;

    deleteNotifications(session, notificationIds,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NChannelMessageListPtr> BaseClient::listChannelMessagesAsync(
    NSessionPtr session,
    const std::string& channelId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const opt::optional<bool>& forward
)
{
    std::promise<NChannelMessageListPtr> promise;

    listChannelMessages(session, channelId, limit, cursor, forward,
        [&](NChannelMessageListPtr channelMessageList) {
            promise.set_value(channelMessageList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NTournamentListPtr> BaseClient::listTournamentsAsync(
    NSessionPtr session,
    const opt::optional<uint32_t>& categoryStart,
    const opt::optional<uint32_t>& categoryEnd,
    const opt::optional<uint32_t>& startTime,
    const opt::optional<uint32_t>& endTime,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor
)
{
    std::promise<NTournamentListPtr> promise;

    listTournaments(session, categoryStart, categoryEnd, startTime, endTime, limit, cursor,
        [&](NTournamentListPtr tournamentList) {
            promise.set_value(tournamentList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NTournamentRecordListPtr> BaseClient::listTournamentRecordsAsync(
    NSessionPtr session,
    const std::string& tournamentId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const std::vector<std::string>& ownerIds
)
{
    std::promise<NTournamentRecordListPtr> promise;

    listTournamentRecords(session, tournamentId, limit, cursor, ownerIds,
        [&](NTournamentRecordListPtr tournamentRecordList) {
            promise.set_value(tournamentRecordList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NTournamentRecordListPtr> BaseClient::listTournamentRecordsAroundOwnerAsync(
    NSessionPtr session,
    const std::string& tournamentId,
    const std::string& ownerId,
    const opt::optional<int32_t>& limit
)
{
    std::promise<NTournamentRecordListPtr> promise;

    listTournamentRecordsAroundOwner(session, tournamentId, ownerId, limit,
        [&](NTournamentRecordListPtr tournamentRecordList) {
            promise.set_value(tournamentRecordList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::joinTournamentAsync(
    NSessionPtr session,
    const std::string& tournamentId
)
{
    std::promise<void> promise;

    joinTournament(session, tournamentId,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NStorageObjectListPtr> BaseClient::listStorageObjectsAsync(
    NSessionPtr session,
    const std::string& collection,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor
)
{
    std::promise<NStorageObjectListPtr> promise;

    listStorageObjects(session, collection, limit, cursor,
        [&](NStorageObjectListPtr objectList) {
            promise.set_value(objectList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NStorageObjectListPtr> BaseClient::listUsersStorageObjectsAsync(
    NSessionPtr session,
    const std::string& collection,
    const std::string& userId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor
)
{
    std::promise<NStorageObjectListPtr> promise;

    listUsersStorageObjects(session, collection, userId, limit, cursor,
        [&](NStorageObjectListPtr objectList) {
            promise.set_value(objectList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<const NStorageObjectAcks&> BaseClient::writeStorageObjectsAsync(
    NSessionPtr session,
    const std::vector<NStorageObjectWrite>& objects
)
{
    std::promise<const NStorageObjectAcks&> promise;

    writeStorageObjects(session, objects,
        [&](NStorageObjectAcks acks) {
            promise.set_value(acks);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<NStorageObjects> BaseClient::readStorageObjectsAsync(
    NSessionPtr session,
    const std::vector<NReadStorageObjectId>& objectIds
)
{
    std::promise<NStorageObjects> promise;

    readStorageObjects(session, objectIds,
        [&](NStorageObjects objects) {
            promise.set_value(objects);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<void> BaseClient::deleteStorageObjectsAsync(
    NSessionPtr session,
    const std::vector<NDeleteStorageObjectId>& objectIds
)
{
    std::promise<void> promise;

    deleteStorageObjects(session, objectIds,
        [&]() {
            promise.set_value();
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<const NRpc&> BaseClient::rpcAsync(
    NSessionPtr session,
    const std::string& id,
    const opt::optional<std::string>& payload
)
{
    std::promise<const NRpc&> promise;

    rpc(session, id, payload,
        [&](const NRpc& rpc) {
            promise.set_value(rpc);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}

std::future<const NRpc&> BaseClient::rpcAsync(
    const std::string& http_key,
    const std::string& id,
    const opt::optional<std::string>& payload
)
{
    std::promise<const NRpc&> promise;

    rpc(http_key, id, payload,
        [&](const NRpc& rpc) {
            promise.set_value(rpc);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.message)));
        });

    return promise.get_future();
}


}
