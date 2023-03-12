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

#include <future>
#include "BaseClient.h"
#include "NRtClient.h"
#include "nakama-cpp/realtime/NWebsocketsFactory.h"
#include "nakama-cpp/log/NLogger.h"

#undef NMODULE_NAME
#define NMODULE_NAME "Nakama::BaseClient"

using namespace std;

namespace Nakama {

#if defined(BUILD_WEBSOCKET_WSLAY) && defined(BUILD_CURL_IO)
NRtClientPtr BaseClient::createRtClient()
{
    return createRtClient(createDefaultWebsocket(_platformParams));
}
#endif

NRtClientPtr BaseClient::createRtClient(NRtTransportPtr transport)
{
    RtClientParameters parameters;
    parameters.host = _host;
    parameters.port = _port;
    parameters.ssl  = _ssl;
    parameters.platformParams = _platformParams;

    if (!transport)
    {
        NLOG_ERROR("No websockets transport passed. Please set transport.");
        return nullptr;
    }

    NRtClientPtr client(new NRtClient(transport, parameters.host, parameters.port, parameters.ssl));
    return client;
}

std::future<NSessionPtr> BaseClient::authenticateDeviceAsync(
    const std::string& id,
    const opt::optional<std::string>& username = opt::nullopt,
    const opt::optional<bool>& create = opt::nullopt,
    const NStringMap& vars = {}
)
{
    std::promise<NSessionPtr> promise;

    authenticateDeviceAsync(id, username, create, vars,
        [&](NSessionPtr restoreSession) {
            promise.set_value(restoreSession);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.getMessage())));
        });

    return promise.get_future();
}

std::future<NSessionPtr> BaseClient::authenticateEmailAsync(
    const std::string& email,
    const std::string& password,
    const std::string& username = std::string(),
    bool create = false,
    const NStringMap& vars = {}
)
{

}

std::future<NSessionPtr> BaseClient::authenticateFacebookAsync(
    const std::string& accessToken,
    const std::string& username = std::string(),
    bool create = false,
    bool importFriends = false,
    const NStringMap& vars = {}
)
{

}

std::future<NSessionPtr> BaseClient::authenticateGoogleAsync(
    const std::string& accessToken,
    const std::string& username = std::string(),
    bool create = false,
    const NStringMap& vars = {}
)
{

}

std::future<NSessionPtr> BaseClient::authenticateGameCenterAsync(
    const std::string& playerId,
    const std::string& bundleId,
    NTimestamp timestampSeconds,
    const std::string& salt,
    const std::string& signature,
    const std::string& publicKeyUrl,
    const std::string& username = std::string(),
    bool create = false,
    const NStringMap& vars = {}
)
{

}

std::future<NSessionPtr> BaseClient::authenticateAppleAsync(
    const std::string& token,
    const std::string& username = std::string(),
    bool create = false,
    const NStringMap& vars = {}
)
{

}

std::future<NSessionPtr> BaseClient::authenticateCustomAsync(
    const std::string& id,
    const std::string& username = std::string(),
    bool create = false,
    const NStringMap& vars = {}
)
{

}

std::future<NSessionPtr> BaseClient::authenticateSteamAsync(
    const std::string& token,
    const std::string& username = std::string(),
    bool create = false,
    const NStringMap& vars = {}
)
{

}

std::future<NSessionPtr> BaseClient::authenticateRefreshAsync(NSessionPtr session)
{

}

std::future<void> BaseClient::linkFacebookAsync(
    NSessionPtr session,
    const std::string& accessToken,
    const opt::optional<bool>& importFriends = opt::nullopt
);

std::future<void> BaseClient::linkEmailAsync(
    NSessionPtr session,
    const std::string& email,
    const std::string& password
){}

std::future<void> BaseClient::linkDeviceAsync(
    NSessionPtr session,
    const std::string& id
){}

std::future<void> BaseClient::linkGoogleAsync(
    NSessionPtr session,
    const std::string& accessToken
){}

std::future<void> BaseClient::linkGameCenterAsync(
    NSessionPtr session,
    const std::string& playerId,
    const std::string& bundleId,
    NTimestamp timestampSeconds,
    const std::string& salt,
    const std::string& signature,
    const std::string& publicKeyUrl
){}

std::future<void> BaseClient::linkAppleAsync(
    NSessionPtr session,
    const std::string& token
){}

std::future<void> BaseClient::linkSteamAsync(
    NSessionPtr session,
    const std::string& token
){}

std::future<void> BaseClient::linkCustomAsync(
    NSessionPtr session,
    const std::string& id
){}

std::future<void> BaseClient::unlinkFacebookAsync(
    NSessionPtr session,
    const std::string& accessToken
);

std::future<void> BaseClient::unlinkEmailAsync(
    NSessionPtr session,
    const std::string& email,
    const std::string& password
){}

std::future<void> BaseClient::unlinkGoogleAsync(
    NSessionPtr session,
    const std::string& accessToken
);

std::future<void> BaseClient::unlinkGameCenterAsync(
    NSessionPtr session,
    const std::string& playerId,
    const std::string& bundleId,
    NTimestamp timestampSeconds,
    const std::string& salt,
    const std::string& signature,
    const std::string& publicKeyUrl
){}

std::future<void> BaseClient::unlinkAppleAsync(
    NSessionPtr session,
    const std::string& token
){}

std::future<void> BaseClient::unlinkSteamAsync(
    NSessionPtr session,
    const std::string& token
){}

std::future<void> BaseClient::unlinkDeviceAsync(
    NSessionPtr session,
    const std::string& id
){}

std::future<void> BaseClient::unlinkCustomAsync(
    NSessionPtr session,
    const std::string& id
){}

std::future<void> BaseClient::importFacebookFriendsAsync(
    NSessionPtr session,
    const std::string& token,
    const opt::optional<bool>& reset = opt::nullopt
){}

std::future<void> BaseClient::updateAccount(
    NSessionPtr session,
    const opt::optional<std::string>& username    = opt::nullopt,
    const opt::optional<std::string>& displayName = opt::nullopt,
    const opt::optional<std::string>& avatarUrl   = opt::nullopt,
    const opt::optional<std::string>& langTag     = opt::nullopt,
    const opt::optional<std::string>& location    = opt::nullopt,
    const opt::optional<std::string>& timezone    = opt::nullopt
){}

std::future<const NUsers&> BaseClient::getUsersAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames = {},
    const std::vector<std::string>& facebookIds = {}
){}

std::future<void> BaseClient::addFriendsAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames = {}
){}

std::future<void> BaseClient::deleteFriendsAsync(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames = {},
    std::function<void()> successCallback = nullptr,
    ErrorCallback errorCallback = nullptr
){}

std::future<void> BaseClient::blockFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames = {}
){}

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
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.getMessage())));
        });

    return promise.get_future();
}

std::future<const NGroup&> BaseClient::createGroupAsync(
    NSessionPtr session,
    const std::string& name,
    const std::string& description = "",
    const std::string& avatarUrl = "",
    const std::string& langTag = "",
    bool open = false,
    const opt::optional<int32_t>& maxCount = {}
){}

std::future<void> BaseClient::deleteGroupAsync(
    NSessionPtr session,
    const std::string& groupId
){}

std::future<void> BaseClient::addGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids
){}


std::future<NGroupUserListPtr> BaseClient::listGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor = ""
){}

std::future<void> BaseClient::kickGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids
){}


std::future<void> BaseClient::joinGroupAsync(
    NSessionPtr session,
    const std::string& groupId
){}

std::future<void> BaseClient::leaveGroupAsync(
    NSessionPtr session,
    const std::string& groupId
){}

std::future<NGroupListPtr> BaseClient::listGroupsAsync(
    NSessionPtr session,
    const std::string& name,
    int32_t limit = 0,
    const std::string& cursor = ""
){}

std::future<NUserGroupListPtr> BaseClient::listUserGroupsAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor = ""
){}

std::future<NUserGroupListPtr> BaseClient::listUserGroupsAsync(
    NSessionPtr session,
    const std::string& userId,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor = "",
    std::function<void(NUserGroupListPtr)> successCallback = nullptr,
    ErrorCallback errorCallback = nullptr
){}

std::future<void> BaseClient::promoteGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids
){}

std::future<void> BaseClient::demoteGroupUsersAsync(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids
){}

std::future<void> BaseClient::updateGroupAsync(
    NSessionPtr session,
    const std::string& groupId,
    const opt::optional<std::string>& name = opt::nullopt,
    const opt::optional<std::string>& description = opt::nullopt,
    const opt::optional<std::string>& avatarUrl = opt::nullopt,
    const opt::optional<std::string>& langTag = opt::nullopt,
    const opt::optional<bool>& open = opt::nullopt
){}

std::future<NLeaderboardRecordListPtr> BaseClient::listLeaderboardRecordsAsync(
    NSessionPtr session,
    const std::string& leaderboardId,
    const std::vector<std::string>& ownerIds = {},
    const opt::optional<int32_t>& limit = opt::nullopt,
    const opt::optional<std::string>& cursor = opt::nullopt
){}

std::future<NLeaderboardRecordListPtr> BaseClient::listLeaderboardRecordsAroundOwnerAsync(
    NSessionPtr session,
    const std::string& leaderboardId,
    const std::string& ownerId,
    const opt::optional<int32_t>& limit = opt::nullopt
){}

std::future<NLeaderboardRecord> writeLeaderboardRecordAsync(
    NSessionPtr session,
    const std::string& leaderboardId,
    int64_t score,
    const opt::optional<int64_t>& subscore = opt::nullopt,
    const opt::optional<std::string>& metadata = opt::nullopt
){}

std::future<NLeaderboardRecord> BaseClient::writeTournamentRecordAsync(
    NSessionPtr session,
    const std::string& tournamentId,
    int64_t score,
    const opt::optional<int64_t>& subscore = opt::nullopt,
    const opt::optional<std::string>& metadata = opt::nullopt
){}

std::future<void> BaseClient::deleteLeaderboardRecordAsync(
    NSessionPtr session,
    const std::string& leaderboardId
){}

std::future<NMatchListPtr> BaseClient::listMatchesAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& min_size = opt::nullopt,
    const opt::optional<int32_t>& max_size = opt::nullopt,
    const opt::optional<int32_t>& limit = opt::nullopt,
    const opt::optional<std::string>& label = opt::nullopt,
    const opt::optional<std::string>& query = opt::nullopt,
    const opt::optional<bool>& authoritative = opt::nullopt
){}

std::future<NNotificationListPtr> BaseClient::listNotificationsAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& limit = opt::nullopt,
    const opt::optional<std::string>& cacheableCursor = opt::nullopt
){}

std::future<void> BaseClient::deleteNotificationsAsync(
    NSessionPtr session,
    const std::vector<std::string>& notificationIds
){}

std::future<NChannelMessageListPtr> BaseClient::listChannelMessagesAsync(
    NSessionPtr session,
    const std::string& channelId,
    const opt::optional<int32_t>& limit = opt::nullopt,
    const opt::optional<std::string>& cursor = opt::nullopt,
    const opt::optional<bool>& forward = opt::nullopt
){}

std::future<NTournamentListPtr> BaseClient::listTournamentsAsync(
    NSessionPtr session,
    const opt::optional<uint32_t>& categoryStart = opt::nullopt,
    const opt::optional<uint32_t>& categoryEnd = opt::nullopt,
    const opt::optional<uint32_t>& startTime = opt::nullopt,
    const opt::optional<uint32_t>& endTime = opt::nullopt,
    const opt::optional<int32_t>& limit = opt::nullopt,
    const opt::optional<std::string>& cursor = opt::nullopt
){}


std::future<NTournamentRecordListPtr> BaseClient::listTournamentRecordsAsync(
    NSessionPtr session,
    const std::string& tournamentId,
    const opt::optional<int32_t>& limit = opt::nullopt,
    const opt::optional<std::string>& cursor = opt::nullopt,
    const std::vector<std::string>& ownerIds = {}
){}

std::future<NTournamentRecordListPtr> BaseClient::listTournamentRecordsAroundOwnerAsync(
    NSessionPtr session,
    const std::string& tournamentId,
    const std::string& ownerId,
    const opt::optional<int32_t>& limit = opt::nullopt
){}

std::future<void> BaseClient::joinTournamentAsync(
    NSessionPtr session,
    const std::string& tournamentId
){}

std::future<NStorageObjectListPtr> BaseClient::listStorageObjectsAsync(
    NSessionPtr session,
    const std::string& collection,
    const opt::optional<int32_t>& limit = opt::nullopt,
    const opt::optional<std::string>& cursor = opt::nullopt
){}

std::future<NStorageObjectListPtr> BaseClient::listUsersStorageObjects(
    NSessionPtr session,
    const std::string& collection,
    const std::string& userId,
    const opt::optional<int32_t>& limit = opt::nullopt,
    const opt::optional<std::string>& cursor = opt::nullopt
){}

std::future<const NStorageObjectAcks&> BaseClient::writeStorageObjectsAsync(
    NSessionPtr session,
    const std::vector<NStorageObjectWrite>& objects
){}

std::future<NStorageObjects> BaseClient::readStorageObjectsAsync(
    NSessionPtr session,
    const std::vector<NReadStorageObjectId>& objectIds,
    std::function<void(const NStorageObjects&)> successCallback = nullptr,
    ErrorCallback errorCallback = nullptr
){}

std::future<NDeleteStorageObjectId> BaseClient::deleteStorageObjectsASync(
    NSessionPtr session,
    const std::vector<NDeleteStorageObjectId>& objectIds,
){}

std::future<const NRpc&> BaseClient::rpcAsync(
    NSessionPtr session,
    const std::string& id,
    const opt::optional<std::string>& payload = opt::nullopt
){}

std::future<const NRpc&> BaseClient::rpcAsync(
    const std::string& http_key,
    const std::string& id,
    const opt::optional<std::string>& payload = opt::nullopt
){}


}
