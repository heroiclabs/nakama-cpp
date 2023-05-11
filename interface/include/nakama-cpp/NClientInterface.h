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
#include <future>

#include <nakama-cpp/NTypes.h>
#include <nakama-cpp/NExport.h>
#include <nakama-cpp/NSessionInterface.h>
#include <nakama-cpp/realtime/NRtClientInterface.h>
#include <nakama-cpp/NError.h>
#include <nakama-cpp/data/NAccount.h>
#include <nakama-cpp/data/NGroup.h>
#include <nakama-cpp/data/NGroupList.h>
#include <nakama-cpp/data/NGroupUserList.h>
#include <nakama-cpp/data/NUsers.h>
#include <nakama-cpp/data/NUserGroupList.h>
#include <nakama-cpp/data/NFriendList.h>
#include <nakama-cpp/data/NLeaderboardRecordList.h>
#include <nakama-cpp/data/NMatchList.h>
#include <nakama-cpp/data/NNotificationList.h>
#include <nakama-cpp/data/NChannelMessageList.h>
#include <nakama-cpp/data/NTournamentList.h>
#include <nakama-cpp/data/NTournamentRecordList.h>
#include <nakama-cpp/data/NStorageObjectList.h>
#include <nakama-cpp/data/NStorageObjectAck.h>
#include <nakama-cpp/data/NStorageObjectWrite.h>
#include <nakama-cpp/data/NStorageObjectId.h>
#include <nakama-cpp/data/NRpc.h>

NAKAMA_NAMESPACE_BEGIN

    using ErrorCallback = std::function<void(const NError&)>;

    /**
     * A client interface to interact with Nakama server.
     */
    class NAKAMA_API NClientInterface
    {
    public:
        virtual ~NClientInterface() {}

        /**
         * Set default error callback.
         *
         * Will be called if a request fails and no error callback was set for the request.
         *
         * @param errorCallback The error callback.
         */
        virtual void setErrorCallback(ErrorCallback errorCallback) = 0;

        /**
         * Set user data.
         *
         * Client just holds this data so you can receive it later when you need it.
         *
         * @param userData The user data.
         */
        virtual void setUserData(void* userData) = 0;

        /**
         * Get user data.
         *
         * @return The user data.
         */
        virtual void* getUserData() const = 0;

        /**
         * Disconnects the client. This function kills all outgoing exchanges immediately without waiting.
         */
        virtual void disconnect() = 0;

        /**
         * Pumps requests queue in your thread.
         * Call it periodically, each 50 ms is ok.
         */
        virtual void tick() = 0;

#if !defined(WITH_EXTERNAL_WS) && !defined(BUILD_IO_EXTERNAL)
        /**
         * Create a new real-time client with parameters from client.
         * @return a new NRtClient instance.
         */
        virtual NRtClientPtr createRtClient() = 0;
#endif

        /**
         * Create a new real-time client with parameters from client.
         *
         * @param parameters The real-time client parameters.
         * @param transport The websocket transport.
         * @return a new NRtClient instance.
         */
        virtual NRtClientPtr createRtClient(NRtTransportPtr transport) = 0;

        /**
         * Authenticate a user with a device id.
         *
         * @param id A device identifier usually obtained from a platform API.
         * @param username A username used to create the user. Defaults to empty string.
         * @param create True if the user should be created when authenticated. Defaults to false.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual void authenticateDevice(
            const std::string& id,
            const opt::optional<std::string>& username = opt::nullopt,
            const opt::optional<bool>& create = opt::nullopt,
            const NStringMap& vars = {},
            std::function<void (NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Authenticate a user with an email and password.
         *
         * @param email The email address of the user.
         * @param password The password for the user.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual void authenticateEmail(
            const std::string& email,
            const std::string& password,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {},
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Authenticate a user with a Facebook auth token.
         *
         * @param accessToken An OAuth access token from the Facebook SDK.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param importFriends True if the Facebook friends should be imported.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual void authenticateFacebook(
            const std::string& accessToken,
            const std::string& username = std::string(),
            bool create = false,
            bool importFriends = false,
            const NStringMap& vars = {},
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Authenticate a user with a Google auth token.
         *
         * @param accessToken An OAuth access token from the Google SDK.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual void authenticateGoogle(
            const std::string& accessToken,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {},
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Authenticate a user with Apple Game Center.
         *
         * @param playerId The player id of the user in Game Center.
         * @param bundleId The bundle id of the Game Center application.
         * @param timestampSeconds The date and time that the signature was created.
         * @param salt A random <c>NSString</c> used to compute the hash and keep it randomized.
         * @param signature The verification signature data generated.
         * @param publicKeyUrl The URL for the public encryption key.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual void authenticateGameCenter(
            const std::string& playerId,
            const std::string& bundleId,
            NTimestamp timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {},
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Authenticate a user with Apple Sign In.
         *
         * @param token The ID token received from Apple to validate.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual void authenticateApple(
            const std::string& token,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {},
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Authenticate a user with a custom id.
         *
         * @param id A custom identifier usually obtained from an external authentication service.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual void authenticateCustom(
            const std::string& id,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {},
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Authenticate a user with a Steam auth token.
         *
         * @param token An authentication token from the Steam network.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual void authenticateSteam(
            const std::string& token,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {},
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Refresh a user's session using a refresh token retrieved from a previous authentication request.
         * @param session The session of the user.
        **/
        virtual void authenticateRefresh(
            NSessionPtr session,
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr) = 0;

        /**
         * Link a Facebook profile to a user account.
         *
         * @param session The session of the user.
         * @param accessToken An OAuth access token from the Facebook SDK.
         * @param importFriends True if the Facebook friends should be imported.
         */
        virtual void linkFacebook(
            NSessionPtr session,
            const std::string& accessToken,
            const opt::optional<bool>& importFriends = opt::nullopt,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Link an email with password to the user account owned by the session.
         *
         * @param session The session of the user.
         * @param email The email address of the user.
         * @param password The password for the user.
         */
        virtual void linkEmail(
            NSessionPtr session,
            const std::string& email,
            const std::string& password,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Link a device id to the user account owned by the session.
         *
         * @param session The session of the user.
         * @param id A device identifier usually obtained from a platform API.
         */
        virtual void linkDevice(
            NSessionPtr session,
            const std::string& id,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Link a Google profile to a user account.
         *
         * @param session The session of the user.
         * @param accessToken An OAuth access token from the Google SDK.
         */
        virtual void linkGoogle(
            NSessionPtr session,
            const std::string& accessToken,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Link a Game Center profile to a user account.
         *
         * @param session The session of the user.
         * @param playerId The player id of the user in Game Center.
         * @param bundleId The bundle id of the Game Center application.
         * @param timestampSeconds The date and time that the signature was created.
         * @param salt A random <c>NSString</c> used to compute the hash and keep it randomized.
         * @param signature The verification signature data generated.
         * @param publicKeyUrl The URL for the public encryption key.
         */
        virtual void linkGameCenter(
            NSessionPtr session,
            const std::string& playerId,
            const std::string& bundleId,
            NTimestamp timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Link an Apple ID to the social profiles on the current user's account.
         *
         * @param session The session of the user.
         * @param token The ID token received from Apple.
         */
        virtual void linkApple(
            NSessionPtr session,
            const std::string& token,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Link a Steam profile to a user account.
         *
         * @param session The session of the user.
         * @param token An authentication token from the Steam network.
         */
        virtual void linkSteam(
            NSessionPtr session,
            const std::string& token,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Link a custom id to the user account owned by the session.
         *
         * @param session The session of the user.
         * @param id A custom identifier usually obtained from an external authentication service.
         */
        virtual void linkCustom(
            NSessionPtr session,
            const std::string& id,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Unlink a Facebook profile from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param accessToken An OAuth access token from the Facebook SDK.
         */
        virtual void unlinkFacebook(
            NSessionPtr session,
            const std::string& accessToken,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Unlink an email with password from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param email The email address of the user.
         * @param password The password for the user.
         */
        virtual void unlinkEmail(
            NSessionPtr session,
            const std::string& email,
            const std::string& password,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Unlink a Google profile from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param accessToken An OAuth access token from the Google SDK.
         */
        virtual void unlinkGoogle(
            NSessionPtr session,
            const std::string& accessToken,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Unlink a Game Center profile from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param playerId The player id of the user in Game Center.
         * @param bundleId The bundle id of the Game Center application.
         * @param timestampSeconds The date and time that the signature was created.
         * @param salt A random <c>NSString</c> used to compute the hash and keep it randomized.
         * @param signature The verification signature data generated.
         * @param publicKeyUrl The URL for the public encryption key.
         */
        virtual void unlinkGameCenter(
            NSessionPtr session,
            const std::string& playerId,
            const std::string& bundleId,
            NTimestamp timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Unlink a Apple profile from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param token An Apple authentication token.
         */
        virtual void unlinkApple(
            NSessionPtr session,
            const std::string& token,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Unlink a Steam profile from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param token An authentication token from the Steam network.
         */
        virtual void unlinkSteam(
            NSessionPtr session,
            const std::string& token,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Unlink a device id from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param id A device identifier usually obtained from a platform API.
         */
        virtual void unlinkDevice(
            NSessionPtr session,
            const std::string& id,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Unlink a custom id from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param id A custom identifier usually obtained from an external authentication service.
         */
        virtual void unlinkCustom(
            NSessionPtr session,
            const std::string& id,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Import Facebook friends and add them to the user's account.
         *
         * The server will import friends when the user authenticates with Facebook. This function can be used to be
         * explicit with the import operation.
         *
         * @param session The session of the user.
         * @param token An OAuth access token from the Facebook SDK.
         * @param reset True if the Facebook friend import for the user should be reset.
         */
        virtual void importFacebookFriends(
            NSessionPtr session,
            const std::string& token,
            const opt::optional<bool>& reset = opt::nullopt,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Fetch the user account owned by the session.
         *
         * @param session The session of the user.
         */
        virtual void getAccount(
            NSessionPtr session,
            std::function<void(const NAccount&)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Update the current user's account on the server.
         *
         * @param session The session for the user.
         * @param username The new username for the user.
         * @param displayName A new display name for the user.
         * @param avatarUrl A new avatar url for the user.
         * @param langTag A new language tag in BCP-47 format for the user.
         * @param location A new location for the user.
         * @param timezone New timezone information for the user.
         */
        virtual void updateAccount(
            NSessionPtr session,
            const opt::optional<std::string>& username    = opt::nullopt,
            const opt::optional<std::string>& displayName = opt::nullopt,
            const opt::optional<std::string>& avatarUrl   = opt::nullopt,
            const opt::optional<std::string>& langTag     = opt::nullopt,
            const opt::optional<std::string>& location    = opt::nullopt,
            const opt::optional<std::string>& timezone    = opt::nullopt,
            std::function<void()> successCallback         = nullptr,
            ErrorCallback errorCallback                   = nullptr
        ) = 0;

        /**
         * Fetch one or more users by id, usernames, and Facebook ids.
         *
         * @param session The session of the user.
         * @param ids List of user IDs.
         * @param usernames List of usernames.
         * @param facebookIds List of Facebook IDs.
         */
        virtual void getUsers(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {},
            const std::vector<std::string>& facebookIds = {},
            std::function<void(const NUsers&)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Add one or more friends by id.
         *
         * @param session The session of the user.
         * @param ids The ids of the users to add or invite as friends.
         * @param usernames The usernames of the users to add as friends.
         */
        virtual void addFriends(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {},
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Delete one more or users by id or username from friends.
         *
         * @param session The session of the user.
         * @param ids the user ids to remove as friends.
         * @param usernames The usernames to remove as friends.
         */
        virtual void deleteFriends(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {},
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Block one or more friends by id.
         *
         * @param session The session of the user.
         * @param ids The ids of the users to block.
         * @param usernames The usernames of the users to block.
         */
        virtual void blockFriends(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {},
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List of friends of the current user.
         *
         * @param session The session of the user.
         * @param limit The max number of records to return. Between 1 and 100.
         * @param state The friend state to list.
         * @param cursor An optional next page cursor.
         */
        virtual void listFriends(
            NSessionPtr session,
            const opt::optional<int32_t>& limit,
            const opt::optional<NFriend::State>& state,
            const std::string& cursor = "",
            std::function<void(NFriendListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Create a group.
         *
         * @param session The session of the user.
         * @param name The name for the group.
         * @param description A description for the group.
         * @param avatarUrl An avatar url for the group.
         * @param langTag A language tag in BCP-47 format for the group.
         * @param open True if the group should have open membership.
         * @param maxCount Maximum number of group members.
         */
        virtual void createGroup(
            NSessionPtr session,
            const std::string& name,
            const std::string& description = "",
            const std::string& avatarUrl = "",
            const std::string& langTag = "",
            bool open = false,
            const opt::optional<int32_t>& maxCount = {},
            std::function<void(const NGroup&)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Delete a group by id.
         *
         * @param session The session of the user.
         * @param groupId The group id to to remove.
         */
        virtual void deleteGroup(
            NSessionPtr session,
            const std::string& groupId,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Add one or more users to the group.
         *
         * @param session The session of the user.
         * @param groupId The id of the group to add users into.
         * @param ids The ids of the users to add or invite to the group.
         */
        virtual void addGroupUsers(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List all users part of the group.
         *
         * @param session The session of the user.
         * @param groupId The id of the group.
         * @param limit The max number of records to return. Between 1 and 100.
         * @param state The group membership state to list.
         * @param cursor An optional next page cursor.
         */
        virtual void listGroupUsers(
            NSessionPtr session,
            const std::string& groupId,
            const opt::optional<int32_t>& limit,
            const opt::optional<NUserGroupState>& state,
            const std::string& cursor = "",
            std::function<void(NGroupUserListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Kick one or more users from the group.
         *
         * @param session The session of the user.
         * @param groupId The id of the group.
         * @param ids The ids of the users to kick.
         */
        virtual void kickGroupUsers(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Join a group if it has open membership or request to join it.
         *
         * @param session The session of the user.
         * @param groupId The id of the group to join.
         */
        virtual void joinGroup(
            NSessionPtr session,
            const std::string& groupId,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Leave a group by id.
         *
         * @param session The session of the user.
         * @param groupId The id of the group to leave.
         */
        virtual void leaveGroup(
            NSessionPtr session,
            const std::string& groupId,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List groups on the server.
         *
         * @param session The session of the user.
         * @param name The name filter to apply to the group list.
         * @param limit The number of groups to list.
         * @param cursor A cursor for the current position in the groups to list.
         */
        virtual void listGroups(
            NSessionPtr session,
            const std::string& name,
            int32_t limit = 0,
            const std::string& cursor = "",
            std::function<void(NGroupListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List of groups the current user is a member of.
         *
         * @param session The session of the user.
         * @param limit The max number of records to return. Between 1 and 100.
         * @param state The group membership state to list.
         * @param cursor An optional next page cursor.
         */
        virtual void listUserGroups(
            NSessionPtr session,
            const opt::optional<int32_t>& limit,
            const opt::optional<NUserGroupState>& state,
            const std::string& cursor = "",
            std::function<void(NUserGroupListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List groups a user is a member of.
         *
         * @param session The session of the user.
         * @param userId The id of the user whose groups to list.
         * @param limit The max number of records to return. Between 1 and 100.
         * @param state The group membership state to list.
         * @param cursor An optional next page cursor.
         */
        virtual void listUserGroups(
            NSessionPtr session,
            const std::string& userId,
            const opt::optional<int32_t>& limit,
            const opt::optional<NUserGroupState>& state,
            const std::string& cursor = "",
            std::function<void(NUserGroupListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Promote a set of users in a group to the next role up.
         *
         * @param session The session of the user.
         * @param groupId The group ID to promote in.
         * @param ids The ids of the users to promote.
         */
        virtual void promoteGroupUsers(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Demote a set of users in a group to the next role down.
         *
         * @param session The session of the user.
         * @param groupId The group ID to demote in.
         * @param ids The ids of the users to demote.
         */
        virtual void demoteGroupUsers(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Update a group.
         *
         * The user must have the correct access permissions for the group.
         *
         * @param session The session of the user.
         * @param groupId The id of the group to update.
         * @param name A new name for the group.
         * @param description A new description for the group.
         * @param avatarUrl A new avatar url for the group.
         * @param langTag A new language tag in BCP-47 format for the group.
         * @param open True if the group should have open membership.
         */
        virtual void updateGroup(
            NSessionPtr session,
            const std::string& groupId,
            const opt::optional<std::string>& name = opt::nullopt,
            const opt::optional<std::string>& description = opt::nullopt,
            const opt::optional<std::string>& avatarUrl = opt::nullopt,
            const opt::optional<std::string>& langTag = opt::nullopt,
            const opt::optional<bool>& open = opt::nullopt,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List records from a leaderboard.
         *
         * @param session The session of the user.
         * @param leaderboardId The id of the leaderboard to list.
         * @param ownerIds Record owners to fetch with the list of records.
         * @param limit The number of records to list.
         * @param cursor A cursor for the current position in the leaderboard records to list.
         */
        virtual void listLeaderboardRecords(
            NSessionPtr session,
            const std::string& leaderboardId,
            const std::vector<std::string>& ownerIds = {},
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt,
            std::function<void(NLeaderboardRecordListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List leaderboard records from a given leaderboard around the owner.
         *
         * @param session The session of the user.
         * @param leaderboardId The id of the leaderboard to list.
         * @param ownerId The owner to retrieve records around.
         * @param limit Max number of records to return. Between 1 and 100.
         */
        virtual void listLeaderboardRecordsAroundOwner(
            NSessionPtr session,
            const std::string& leaderboardId,
            const std::string& ownerId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            std::function<void(NLeaderboardRecordListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Write a record to a leaderboard.
         *
         * @param session The session for the user.
         * @param leaderboardId The id of the leaderboard to write.
         * @param score The score for the leaderboard record.
         * @param subscore The subscore for the leaderboard record.
         * @param metadata The metadata for the leaderboard record.
         */
        virtual void writeLeaderboardRecord(
            NSessionPtr session,
            const std::string& leaderboardId,
            std::int64_t score,
            const opt::optional<std::int64_t>& subscore = opt::nullopt,
            const opt::optional<std::string>& metadata = opt::nullopt,
            std::function<void(NLeaderboardRecord)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * A request to submit a score to a tournament.
         *
         * @param session The session for the user.
         * @param tournamentId The tournament ID to write the record for.
         * @param score The score value to submit.
         * @param subscore  An optional secondary value.
         * @param metadata A JSON object of additional properties.
         */
        virtual void writeTournamentRecord(
            NSessionPtr session,
            const std::string& tournamentId,
            std::int64_t score,
            const opt::optional<std::int64_t>& subscore = opt::nullopt,
            const opt::optional<std::string>& metadata = opt::nullopt,
            std::function<void(NLeaderboardRecord)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Delete a leaderboard record.
         *
         * @param session The session of the user.
         * @param leaderboardId The id of the leaderboard with the record to be deleted.
         */
        virtual void deleteLeaderboardRecord(
            NSessionPtr session,
            const std::string& leaderboardId,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Fetch a list of matches active on the server.
         *
         * @param session The session of the user.
         * @param min_size The minimum number of match participants.
         * @param max_size The maximum number of match participants.
         * @param limit The number of matches to list.
         * @param label The label to filter the match list on.
         * @param authoritative <c>true</c> to include authoritative matches.
         */
        virtual void listMatches(
            NSessionPtr session,
            const opt::optional<int32_t>& min_size = opt::nullopt,
            const opt::optional<int32_t>& max_size = opt::nullopt,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& label = opt::nullopt,
            const opt::optional<std::string>& query = opt::nullopt,
            const opt::optional<bool>& authoritative = opt::nullopt,
            std::function<void(NMatchListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List notifications for the user with an optional cursor.
         *
         * @param session The session of the user.
         * @param limit The number of notifications to list.
         * @param cacheableCursor A cursor for the current position in notifications to list.
         */
        virtual void listNotifications(
            NSessionPtr session,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cacheableCursor = opt::nullopt,
            std::function<void(NNotificationListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Delete one or more notifications by id.
         *
         * @param session The session of the user.
         * @param notificationIds The notification ids to remove.
         */
        virtual void deleteNotifications(
            NSessionPtr session,
            const std::vector<std::string>& notificationIds,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List messages from a chat channel.
         *
         * @param session The session of the user.
         * @param channelId A channel identifier.
         * @param limit The number of chat messages to list.
         * @param cursor A cursor for the current position in the messages history to list.
         * @param forward Fetch messages forward from the current cursor (or the start).
         */
        virtual void listChannelMessages(
            NSessionPtr session,
            const std::string& channelId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt,
            const opt::optional<bool>& forward = opt::nullopt,
            std::function<void(NChannelMessageListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List active/upcoming tournaments based on given filters.
         *
         * @param session The session of the user.
         * @param categoryStart The start of the categories to include. Defaults to 0.
         * @param categoryEnd The end of the categories to include. Defaults to 128.
         * @param startTime The start time for tournaments. Defaults to current Unix time.
         * @param endTime The end time for tournaments. Defaults to +1 year from current Unix time.
         * @param limit Max number of records to return. Between 1 and 100.
         * @param cursor A next page cursor for listings.
         */
        virtual void listTournaments(
            NSessionPtr session,
            const opt::optional<uint32_t>& categoryStart = opt::nullopt,
            const opt::optional<uint32_t>& categoryEnd = opt::nullopt,
            const opt::optional<uint32_t>& startTime = opt::nullopt,
            const opt::optional<uint32_t>& endTime = opt::nullopt,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt,
            std::function<void(NTournamentListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List tournament records from a given tournament.
         *
         * @param session The session of the user.
         * @param tournamentId The ID of the tournament to list for.
         * @param limit Max number of records to return. Between 1 and 100.
         * @param cursor A next or previous page cursor.
         * @param ownerIds One or more owners to retrieve records for.
         */
        virtual void listTournamentRecords(
            NSessionPtr session,
            const std::string& tournamentId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt,
            const std::vector<std::string>& ownerIds = {},
            std::function<void(NTournamentRecordListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List tournament records from a given tournament around the owner.
         *
         * @param session The session of the user.
         * @param tournamentId The ID of the tournament to list for.
         * @param ownerId The owner to retrieve records around.
         * @param limit Max number of records to return. Between 1 and 100.
         */
        virtual void listTournamentRecordsAroundOwner(
            NSessionPtr session,
            const std::string& tournamentId,
            const std::string& ownerId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            std::function<void(NTournamentRecordListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Join a tournament if it has open membership or request to join it.
         *
         * @param session The session of the user.
         * @param tournamentId The id of the tournament to join.
         */
        virtual void joinTournament(
            NSessionPtr session,
            const std::string& tournamentId,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List storage objects in a collection which have public read access.
         *
         * @param session The session of the user.
         * @param collection The collection to list over.
         * @param limit The number of objects to list.
         * @param cursor A cursor to paginate over the collection.
         */
        virtual void listStorageObjects(
            NSessionPtr session,
            const std::string& collection,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt,
            std::function<void(NStorageObjectListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * List storage objects in a collection which belong to a specific user and have public read access.
         *
         * @param session The session of the user.
         * @param collection The collection to list over.
         * @param userId The user ID of the user to list objects for.
         * @param limit The number of objects to list.
         * @param cursor A cursor to paginate over the collection.
         */
        virtual void listUsersStorageObjects(
            NSessionPtr session,
            const std::string& collection,
            const std::string& userId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt,
            std::function<void(NStorageObjectListPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Write objects to the storage engine.
         *
         * @param session The session of the user.
         * @param objects The objects to write.
         */
        virtual void writeStorageObjects(
            NSessionPtr session,
            const std::vector<NStorageObjectWrite>& objects,
            std::function<void(const NStorageObjectAcks&)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Read one or more objects from the storage engine.
         *
         * @param session The session of the user.
         * @param objectIds The objects to read.
         */
        virtual void readStorageObjects(
            NSessionPtr session,
            const std::vector<NReadStorageObjectId>& objectIds,
            std::function<void(const NStorageObjects&)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Delete one or more storage objects.
         *
         * @param session The session of the user.
         * @param objectIds The ids of the objects to delete.
         */
        virtual void deleteStorageObjects(
            NSessionPtr session,
            const std::vector<NDeleteStorageObjectId>& objectIds,
            std::function<void()> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Execute a server framework function with an input payload on the server.
         *
         * @param session The session of the user.
         * @param id The id of the function to execute on the server.
         * @param payload The payload to send with the function call.
         */
        virtual void rpc(
            NSessionPtr session,
            const std::string& id,
            const opt::optional<std::string>& payload = opt::nullopt,
            std::function<void(const NRpc&)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Execute a server framework function with an input payload on the server.
         *
         * @param http_key The server's runtime HTTP key.
         * @param id The id of the function to execute on the server.
         * @param payload The payload to send with the function call.
         */
        virtual void rpc(
            const std::string& http_key,
            const std::string& id,
            const opt::optional<std::string>& payload = opt::nullopt,
            std::function<void(const NRpc&)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Authenticate a user with a device id.
         *
         * @param id A device identifier usually obtained from a platform API.
         * @param username A username used to create the user. Defaults to empty string.
         * @param create True if the user should be created when authenticated. Defaults to false.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual std::future<NSessionPtr> authenticateDeviceAsync(
            const std::string& id,
            const opt::optional<std::string>& username = opt::nullopt,
            const opt::optional<bool>& create = opt::nullopt,
            const NStringMap& vars = {}
        ) = 0;

        /**
         * Authenticate a user with an email and password.
         *
         * @param email The email address of the user.
         * @param password The password for the user.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual std::future<NSessionPtr> authenticateEmailAsync(
            const std::string& email,
            const std::string& password,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        ) = 0;

        /**
         * Authenticate a user with a Facebook auth token.
         *
         * @param accessToken An OAuth access token from the Facebook SDK.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param importFriends True if the Facebook friends should be imported.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual std::future<NSessionPtr> authenticateFacebookAsync(
            const std::string& accessToken,
            const std::string& username = std::string(),
            bool create = false,
            bool importFriends = false,
            const NStringMap& vars = {}
        ) = 0;

        /**
         * Authenticate a user with a Google auth token.
         *
         * @param accessToken An OAuth access token from the Google SDK.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual std::future<NSessionPtr> authenticateGoogleAsync(
            const std::string& accessToken,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        ) = 0;

        /**
         * Authenticate a user with Apple Game Center.
         *
         * @param playerId The player id of the user in Game Center.
         * @param bundleId The bundle id of the Game Center application.
         * @param timestampSeconds The date and time that the signature was created.
         * @param salt A random <c>NSString</c> used to compute the hash and keep it randomized.
         * @param signature The verification signature data generated.
         * @param publicKeyUrl The URL for the public encryption key.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
       virtual std::future<NSessionPtr> authenticateGameCenterAsync(
            const std::string& playerId,
            const std::string& bundleId,
            NTimestamp timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        ) = 0;

        /**
         * Authenticate a user with Apple Sign In.
         *
         * @param token The ID token received from Apple to validate.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual std::future<NSessionPtr> authenticateAppleAsync(
            const std::string& token,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        ) = 0;

        /**
         * Authenticate a user with a custom id.
         *
         * @param id A custom identifier usually obtained from an external authentication service.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual std::future<NSessionPtr> authenticateCustomAsync(
            const std::string& id,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        ) = 0;

        /**
         * Authenticate a user with a Steam auth token.
         *
         * @param token An authentication token from the Steam network.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         * @param vars Extra information that will be bundled in the session token.
         */
        virtual std::future<NSessionPtr> authenticateSteamAsync(
            const std::string& token,
            const std::string& username = std::string(),
            bool create = false,
            const NStringMap& vars = {}
        ) = 0;

        /**
         * Refresh a user's session using a refresh token retrieved from a previous authentication request.
         * @param session The session of the user.
        **/
        virtual std::future<NSessionPtr> authenticateRefreshAsync(NSessionPtr session) = 0;

        /**
         * Link a Facebook profile to a user account.
         *
         * @param session The session of the user.
         * @param accessToken An OAuth access token from the Facebook SDK.
         * @param importFriends True if the Facebook friends should be imported.
         */
        virtual std::future<void> linkFacebookAsync(
            NSessionPtr session,
            const std::string& accessToken,
            const opt::optional<bool>& importFriends = opt::nullopt
        ) = 0;

        /**
         * Link an email with password to the user account owned by the session.
         *
         * @param session The session of the user.
         * @param email The email address of the user.
         * @param password The password for the user.
         */
        virtual std::future<void> linkEmailAsync(
            NSessionPtr session,
            const std::string& email,
            const std::string& password
        ) = 0;

        /**
         * Link a device id to the user account owned by the session.
         *
         * @param session The session of the user.
         * @param id A device identifier usually obtained from a platform API.
         */
        virtual std::future<void> linkDeviceAsync(
            NSessionPtr session,
            const std::string& id
        ) = 0;

        /**
         * Link a Google profile to a user account.
         *
         * @param session The session of the user.
         * @param accessToken An OAuth access token from the Google SDK.
         */
        virtual std::future<void> linkGoogleAsync(
            NSessionPtr session,
            const std::string& accessToken
        ) = 0;

        /**
         * Link a Game Center profile to a user account.
         *
         * @param session The session of the user.
         * @param playerId The player id of the user in Game Center.
         * @param bundleId The bundle id of the Game Center application.
         * @param timestampSeconds The date and time that the signature was created.
         * @param salt A random <c>NSString</c> used to compute the hash and keep it randomized.
         * @param signature The verification signature data generated.
         * @param publicKeyUrl The URL for the public encryption key.
         */
        virtual std::future<void> linkGameCenterAsync(
            NSessionPtr session,
            const std::string& playerId,
            const std::string& bundleId,
            NTimestamp timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl
        ) = 0;

        /**
         * Link an Apple ID to the social profiles on the current user's account.
         *
         * @param session The session of the user.
         * @param token The ID token received from Apple.
         */
        virtual std::future<void> linkAppleAsync(
            NSessionPtr session,
            const std::string& token
        ) = 0;

        /**
         * Link a Steam profile to a user account.
         *
         * @param session The session of the user.
         * @param token An authentication token from the Steam network.
         */
        virtual std::future<void> linkSteamAsync(
            NSessionPtr session,
            const std::string& token
        ) = 0;

        /**
         * Link a custom id to the user account owned by the session.
         *
         * @param session The session of the user.
         * @param id A custom identifier usually obtained from an external authentication service.
         */
        virtual std::future<void> linkCustomAsync(
            NSessionPtr session,
            const std::string& id
        ) = 0;

        /**
         * Unlink a Facebook profile from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param accessToken An OAuth access token from the Facebook SDK.
         */
        virtual std::future<void> unlinkFacebookAsync(
            NSessionPtr session,
            const std::string& accessToken
        ) = 0;

        /**
         * Unlink an email with password from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param email The email address of the user.
         * @param password The password for the user.
         */
        virtual std::future<void> unlinkEmailAsync(
            NSessionPtr session,
            const std::string& email,
            const std::string& password
        ) = 0;

        /**
         * Unlink a Google profile from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param accessToken An OAuth access token from the Google SDK.
         */
        virtual std::future<void> unlinkGoogleAsync(
            NSessionPtr session,
            const std::string& accessToken
        ) = 0;

        /**
         * Unlink a Game Center profile from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param playerId The player id of the user in Game Center.
         * @param bundleId The bundle id of the Game Center application.
         * @param timestampSeconds The date and time that the signature was created.
         * @param salt A random <c>NSString</c> used to compute the hash and keep it randomized.
         * @param signature The verification signature data generated.
         * @param publicKeyUrl The URL for the public encryption key.
         */
        virtual std::future<void> unlinkGameCenterAsync(
            NSessionPtr session,
            const std::string& playerId,
            const std::string& bundleId,
            NTimestamp timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl
        ) = 0;

        /**
         * Unlink a Apple profile from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param token An Apple authentication token.
         */
        virtual std::future<void> unlinkAppleAsync(
            NSessionPtr session,
            const std::string& token
        ) = 0;

        /**
         * Unlink a Steam profile from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param token An authentication token from the Steam network.
         */
        virtual std::future<void> unlinkSteamAsync(
            NSessionPtr session,
            const std::string& token
        ) = 0;

        /**
         * Unlink a device id from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param id A device identifier usually obtained from a platform API.
         */
        virtual std::future<void> unlinkDeviceAsync(
            NSessionPtr session,
            const std::string& id
        ) = 0;

        /**
         * Unlink a custom id from the user account owned by the session.
         *
         * @param session The session of the user.
         * @param id A custom identifier usually obtained from an external authentication service.
         */
        virtual std::future<void> unlinkCustomAsync(
            NSessionPtr session,
            const std::string& id
        ) = 0;

        /**
         * Import Facebook friends and add them to the user's account.
         *
         * The server will import friends when the user authenticates with Facebook. This function can be used to be
         * explicit with the import operation.
         *
         * @param session The session of the user.
         * @param token An OAuth access token from the Facebook SDK.
         * @param reset True if the Facebook friend import for the user should be reset.
         */
        virtual std::future<void> importFacebookFriendsAsync(
            NSessionPtr session,
            const std::string& token,
            const opt::optional<bool>& reset = opt::nullopt
        ) = 0;

        /**
         * Fetch the user account owned by the session.
         *
         * @param session The session of the user.
         */
        virtual std::future<NAccount> getAccountAsync(
            NSessionPtr session
        ) = 0;

        /**
         * Update the current user's account on the server.
         *
         * @param session The session for the user.
         * @param username The new username for the user.
         * @param displayName A new display name for the user.
         * @param avatarUrl A new avatar url for the user.
         * @param langTag A new language tag in BCP-47 format for the user.
         * @param location A new location for the user.
         * @param timezone New timezone information for the user.
         */
        virtual std::future<void> updateAccountAsync(
            NSessionPtr session,
            const opt::optional<std::string>& username    = opt::nullopt,
            const opt::optional<std::string>& displayName = opt::nullopt,
            const opt::optional<std::string>& avatarUrl   = opt::nullopt,
            const opt::optional<std::string>& langTag     = opt::nullopt,
            const opt::optional<std::string>& location    = opt::nullopt,
            const opt::optional<std::string>& timezone    = opt::nullopt
        ) = 0;

        /**
         * Fetch one or more users by id, usernames, and Facebook ids.
         *
         * @param session The session of the user.
         * @param ids List of user IDs.
         * @param usernames List of usernames.
         * @param facebookIds List of Facebook IDs.
         */
        virtual std::future<NUsers> getUsersAsync(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {},
            const std::vector<std::string>& facebookIds = {}
        ) = 0;

        /**
         * Add one or more friends by id.
         *
         * @param session The session of the user.
         * @param ids The ids of the users to add or invite as friends.
         * @param usernames The usernames of the users to add as friends.
         */
        virtual std::future<void> addFriendsAsync(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {}
        ) = 0;

        /**
         * Delete one more or users by id or username from friends.
         *
         * @param session The session of the user.
         * @param ids the user ids to remove as friends.
         * @param usernames The usernames to remove as friends.
         */
        virtual std::future<void> deleteFriendsAsync(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {}
        ) = 0;

        /**
         * Block one or more friends by id.
         *
         * @param session The session of the user.
         * @param ids The ids of the users to block.
         * @param usernames The usernames of the users to block.
         */
        virtual std::future<void> blockFriendsAsync(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames = {}
        ) = 0;

        /**
         * List of friends of the current user.
         *
         * @param session The session of the user.
         * @param limit The max number of records to return. Between 1 and 100.
         * @param state The friend state to list.
         * @param cursor An optional next page cursor.
         */
        virtual std::future<NFriendListPtr> listFriendsAsync(
            NSessionPtr session,
            const opt::optional<int32_t>& limit,
            const opt::optional<NFriend::State>& state,
            const std::string& cursor = ""
        ) = 0;

        /**
         * Create a group.
         *
         * @param session The session of the user.
         * @param name The name for the group.
         * @param description A description for the group.
         * @param avatarUrl An avatar url for the group.
         * @param langTag A language tag in BCP-47 format for the group.
         * @param open True if the group should have open membership.
         * @param maxCount Maximum number of group members.
         */
        virtual std::future<NGroup> createGroupAsync(
            NSessionPtr session,
            const std::string& name,
            const std::string& description = "",
            const std::string& avatarUrl = "",
            const std::string& langTag = "",
            bool open = false,
            const opt::optional<int32_t>& maxCount = {}
        ) = 0;

        /**
         * Delete a group by id.
         *
         * @param session The session of the user.
         * @param groupId The group id to to remove.
         */
        virtual std::future<void> deleteGroupAsync(
            NSessionPtr session,
            const std::string& groupId
        ) = 0;

        /**
         * Add one or more users to the group.
         *
         * @param session The session of the user.
         * @param groupId The id of the group to add users into.
         * @param ids The ids of the users to add or invite to the group.
         */
        virtual std::future<void> addGroupUsersAsync(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids
        ) = 0;

        /**
         * List all users part of the group.
         *
         * @param session The session of the user.
         * @param groupId The id of the group.
         * @param limit The max number of records to return. Between 1 and 100.
         * @param state The group membership state to list.
         * @param cursor An optional next page cursor.
         */
        virtual std::future<NGroupUserListPtr> listGroupUsersAsync(
            NSessionPtr session,
            const std::string& groupId,
            const opt::optional<int32_t>& limit,
            const opt::optional<NUserGroupState>& state,
            const std::string& cursor = ""
        ) = 0;

        /**
         * Kick one or more users from the group.
         *
         * @param session The session of the user.
         * @param groupId The id of the group.
         * @param ids The ids of the users to kick.
         */
        virtual std::future<void> kickGroupUsersAsync(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids
        ) = 0;

        /**
         * Join a group if it has open membership or request to join it.
         *
         * @param session The session of the user.
         * @param groupId The id of the group to join.
         */
        virtual std::future<void> joinGroupAsync(
            NSessionPtr session,
            const std::string& groupId
        ) = 0;

        /**
         * Leave a group by id.
         *
         * @param session The session of the user.
         * @param groupId The id of the group to leave.
         */
        virtual std::future<void> leaveGroupAsync(
            NSessionPtr session,
            const std::string& groupId
        ) = 0;

        /**
         * List groups on the server.
         *
         * @param session The session of the user.
         * @param name The name filter to apply to the group list.
         * @param limit The number of groups to list.
         * @param cursor A cursor for the current position in the groups to list.
         */
        virtual std::future<NGroupListPtr> listGroupsAsync(
            NSessionPtr session,
            const std::string& name,
            int32_t limit = 0,
            const std::string& cursor = ""
        ) = 0;

        /**
         * List of groups the current user is a member of.
         *
         * @param session The session of the user.
         * @param limit The max number of records to return. Between 1 and 100.
         * @param state The group membership state to list.
         * @param cursor An optional next page cursor.
         */
        virtual std::future<NUserGroupListPtr> listUserGroupsAsync(
            NSessionPtr session,
            const opt::optional<int32_t>& limit,
            const opt::optional<NUserGroupState>& state,
            const std::string& cursor = ""
        ) = 0;

        /**
         * List groups a user is a member of.
         *
         * @param session The session of the user.
         * @param userId The id of the user whose groups to list.
         * @param limit The max number of records to return. Between 1 and 100.
         * @param state The group membership state to list.
         * @param cursor An optional next page cursor.
         */
        virtual std::future<NUserGroupListPtr> listUserGroupsAsync(
            NSessionPtr session,
            const std::string& userId,
            const opt::optional<int32_t>& limit,
            const opt::optional<NUserGroupState>& state,
            const std::string& cursor = ""
        ) = 0;

        /**
         * Promote a set of users in a group to the next role up.
         *
         * @param session The session of the user.
         * @param groupId The group ID to promote in.
         * @param ids The ids of the users to promote.
         */
        virtual std::future<void> promoteGroupUsersAsync(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids
        ) = 0;

        /**
         * Demote a set of users in a group to the next role down.
         *
         * @param session The session of the user.
         * @param groupId The group ID to demote in.
         * @param ids The ids of the users to demote.
         */
        virtual std::future<void> demoteGroupUsersAsync(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids
        ) = 0;

        /**
         * Update a group.
         *
         * The user must have the correct access permissions for the group.
         *
         * @param session The session of the user.
         * @param groupId The id of the group to update.
         * @param name A new name for the group.
         * @param description A new description for the group.
         * @param avatarUrl A new avatar url for the group.
         * @param langTag A new language tag in BCP-47 format for the group.
         * @param open True if the group should have open membership.
         */
        virtual std::future<void> updateGroupAsync(
            NSessionPtr session,
            const std::string& groupId,
            const opt::optional<std::string>& name = opt::nullopt,
            const opt::optional<std::string>& description = opt::nullopt,
            const opt::optional<std::string>& avatarUrl = opt::nullopt,
            const opt::optional<std::string>& langTag = opt::nullopt,
            const opt::optional<bool>& open = opt::nullopt
        ) = 0;

        /**
         * List records from a leaderboard.
         *
         * @param session The session of the user.
         * @param leaderboardId The id of the leaderboard to list.
         * @param ownerIds Record owners to fetch with the list of records.
         * @param limit The number of records to list.
         * @param cursor A cursor for the current position in the leaderboard records to list.
         */
        virtual std::future<NLeaderboardRecordListPtr> listLeaderboardRecordsAsync(
            NSessionPtr session,
            const std::string& leaderboardId,
            const std::vector<std::string>& ownerIds = {},
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt
        ) = 0;

        /**
         * List leaderboard records from a given leaderboard around the owner.
         *
         * @param session The session of the user.
         * @param leaderboardId The id of the leaderboard to list.
         * @param ownerId The owner to retrieve records around.
         * @param limit Max number of records to return. Between 1 and 100.
         */
        virtual std::future<NLeaderboardRecordListPtr> listLeaderboardRecordsAroundOwnerAsync(
            NSessionPtr session,
            const std::string& leaderboardId,
            const std::string& ownerId,
            const opt::optional<int32_t>& limit = opt::nullopt
        ) = 0;

        /**
         * Write a record to a leaderboard.
         *
         * @param session The session for the user.
         * @param leaderboardId The id of the leaderboard to write.
         * @param score The score for the leaderboard record.
         * @param subscore The subscore for the leaderboard record.
         * @param metadata The metadata for the leaderboard record.
         */
        virtual std::future<NLeaderboardRecord> writeLeaderboardRecordAsync(
            NSessionPtr session,
            const std::string& leaderboardId,
            std::int64_t score,
            const opt::optional<std::int64_t>& subscore = opt::nullopt,
            const opt::optional<std::string>& metadata = opt::nullopt
        ) = 0;

        /**
         * A request to submit a score to a tournament.
         *
         * @param session The session for the user.
         * @param tournamentId The tournament ID to write the record for.
         * @param score The score value to submit.
         * @param subscore  An optional secondary value.
         * @param metadata A JSON object of additional properties.
         */
        virtual std::future<NLeaderboardRecord> writeTournamentRecordAsync(
            NSessionPtr session,
            const std::string& tournamentId,
            std::int64_t score,
            const opt::optional<std::int64_t>& subscore = opt::nullopt,
            const opt::optional<std::string>& metadata = opt::nullopt
        ) = 0;

        /**
         * Delete a leaderboard record.
         *
         * @param session The session of the user.
         * @param leaderboardId The id of the leaderboard with the record to be deleted.
         */
        virtual std::future<void> deleteLeaderboardRecordAsync(
            NSessionPtr session,
            const std::string& leaderboardId
        ) = 0;

        /**
         * Fetch a list of matches active on the server.
         *
         * @param session The session of the user.
         * @param min_size The minimum number of match participants.
         * @param max_size The maximum number of match participants.
         * @param limit The number of matches to list.
         * @param label The label to filter the match list on.
         * @param authoritative <c>true</c> to include authoritative matches.
         */
        virtual std::future<NMatchListPtr> listMatchesAsync(
            NSessionPtr session,
            const opt::optional<int32_t>& min_size = opt::nullopt,
            const opt::optional<int32_t>& max_size = opt::nullopt,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& label = opt::nullopt,
            const opt::optional<std::string>& query = opt::nullopt,
            const opt::optional<bool>& authoritative = opt::nullopt
        ) = 0;

        /**
         * List notifications for the user with an optional cursor.
         *
         * @param session The session of the user.
         * @param limit The number of notifications to list.
         * @param cacheableCursor A cursor for the current position in notifications to list.
         */
        virtual std::future<NNotificationListPtr> listNotificationsAsync(
            NSessionPtr session,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cacheableCursor = opt::nullopt
        ) = 0;

        /**
         * Delete one or more notifications by id.
         *
         * @param session The session of the user.
         * @param notificationIds The notification ids to remove.
         */
        virtual std::future<void> deleteNotificationsAsync(
            NSessionPtr session,
            const std::vector<std::string>& notificationIds
        ) = 0;

        /**
         * List messages from a chat channel.
         *
         * @param session The session of the user.
         * @param channelId A channel identifier.
         * @param limit The number of chat messages to list.
         * @param cursor A cursor for the current position in the messages history to list.
         * @param forward Fetch messages forward from the current cursor (or the start).
         */
        virtual std::future<NChannelMessageListPtr> listChannelMessagesAsync(
            NSessionPtr session,
            const std::string& channelId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt,
            const opt::optional<bool>& forward = opt::nullopt
        ) = 0;

        /**
         * List active/upcoming tournaments based on given filters.
         *
         * @param session The session of the user.
         * @param categoryStart The start of the categories to include. Defaults to 0.
         * @param categoryEnd The end of the categories to include. Defaults to 128.
         * @param startTime The start time for tournaments. Defaults to current Unix time.
         * @param endTime The end time for tournaments. Defaults to +1 year from current Unix time.
         * @param limit Max number of records to return. Between 1 and 100.
         * @param cursor A next page cursor for listings.
         */
        virtual std::future<NTournamentListPtr> listTournamentsAsync(
            NSessionPtr session,
            const opt::optional<uint32_t>& categoryStart = opt::nullopt,
            const opt::optional<uint32_t>& categoryEnd = opt::nullopt,
            const opt::optional<uint32_t>& startTime = opt::nullopt,
            const opt::optional<uint32_t>& endTime = opt::nullopt,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt
        ) = 0;

        /**
         * List tournament records from a given tournament.
         *
         * @param session The session of the user.
         * @param tournamentId The ID of the tournament to list for.
         * @param limit Max number of records to return. Between 1 and 100.
         * @param cursor A next or previous page cursor.
         * @param ownerIds One or more owners to retrieve records for.
         */
        virtual std::future<NTournamentRecordListPtr> listTournamentRecordsAsync(
            NSessionPtr session,
            const std::string& tournamentId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt,
            const std::vector<std::string>& ownerIds = {}
        ) = 0;

        /**
         * List tournament records from a given tournament around the owner.
         *
         * @param session The session of the user.
         * @param tournamentId The ID of the tournament to list for.
         * @param ownerId The owner to retrieve records around.
         * @param limit Max number of records to return. Between 1 and 100.
         */
        virtual std::future<NTournamentRecordListPtr> listTournamentRecordsAroundOwnerAsync(
            NSessionPtr session,
            const std::string& tournamentId,
            const std::string& ownerId,
            const opt::optional<int32_t>& limit = opt::nullopt
        ) = 0;

        /**
         * Join a tournament if it has open membership or request to join it.
         *
         * @param session The session of the user.
         * @param tournamentId The id of the tournament to join.
         */
        virtual std::future<void> joinTournamentAsync(
            NSessionPtr session,
            const std::string& tournamentId
        ) = 0;

        /**
         * List storage objects in a collection which have public read access.
         *
         * @param session The session of the user.
         * @param collection The collection to list over.
         * @param limit The number of objects to list.
         * @param cursor A cursor to paginate over the collection.
         */
        virtual std::future<NStorageObjectListPtr> listStorageObjectsAsync(
            NSessionPtr session,
            const std::string& collection,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt
        ) = 0;

        /**
         * List storage objects in a collection which belong to a specific user and have public read access.
         *
         * @param session The session of the user.
         * @param collection The collection to list over.
         * @param userId The user ID of the user to list objects for.
         * @param limit The number of objects to list.
         * @param cursor A cursor to paginate over the collection.
         */
        virtual std::future<NStorageObjectListPtr> listUsersStorageObjectsAsync(
            NSessionPtr session,
            const std::string& collection,
            const std::string& userId,
            const opt::optional<int32_t>& limit = opt::nullopt,
            const opt::optional<std::string>& cursor = opt::nullopt
        ) = 0;

        /**
         * Write objects to the storage engine.
         *
         * @param session The session of the user.
         * @param objects The objects to write.
         */
        virtual std::future<NStorageObjectAcks> writeStorageObjectsAsync(
            NSessionPtr session,
            const std::vector<NStorageObjectWrite>& objects
        ) = 0;

        /**
         * Read one or more objects from the storage engine.
         *
         * @param session The session of the user.
         * @param objectIds The objects to read.
         */
        virtual std::future<NStorageObjects> readStorageObjectsAsync(
            NSessionPtr session,
            const std::vector<NReadStorageObjectId>& objectIds
        ) = 0;

        /**
         * Delete one or more storage objects.
         *
         * @param session The session of the user.
         * @param objectIds The ids of the objects to delete.
         */
        virtual std::future<void> deleteStorageObjectsAsync(
            NSessionPtr session,
            const std::vector<NDeleteStorageObjectId>& objectIds
        ) = 0;

        /**
         * Execute a server framework function with an input payload on the server.
         *
         * @param session The session of the user.
         * @param id The id of the function to execute on the server.
         * @param payload The payload to send with the function call.
         */
        virtual std::future<NRpc> rpcAsync(
            NSessionPtr session,
            const std::string& id,
            const opt::optional<std::string>& payload = opt::nullopt
        ) = 0;

        /**
         * Execute an RPC function with an input payload on the server.
         *
         * @param http_key The server's runtime HTTP key.
         * @param id The id of the function to execute on the server.
         * @param payload The payload to send with the function call.
         */
        virtual std::future<NRpc> rpcAsync(
            const std::string& http_key,
            const std::string& id,
            const opt::optional<std::string>& payload = opt::nullopt
        ) = 0;
    };

    using NClientPtr = std::shared_ptr<NClientInterface>;

NAKAMA_NAMESPACE_END
