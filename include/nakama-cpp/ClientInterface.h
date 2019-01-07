/*
* Copyright 2018 The Nakama Authors
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
#include "nakama-cpp/NError.h"
#include "nakama-cpp/data/NAccount.h"
#include "nakama-cpp/data/NGroup.h"

namespace Nakama {

    typedef std::function<void(const NError&)> ErrorCallback;

    /**
     * A client interface to interact with Nakama server.
     */
    class ClientInterface
    {
    public:
        virtual ~ClientInterface() {}

        /**
         * Disconnects the client. This function kills all outgoing exchanges immediately without waiting.
         */
        virtual void disconnect() = 0;

        /**
         * Pumps requests queue in your thread.
         * Call it periodically, each 50 ms is ok.
         */
        virtual void tick() = 0;

        /**
         * Authenticate a user with a device id.
         *
         * @param id A device identifier usually obtained from a platform API.
         * @param username A username used to create the user. Defaults to empty string.
         * @param create True if the user should be created when authenticated. Defaults to false.
         */
        virtual void authenticateDevice(
            const std::string& id,
            const std::string& username = std::string(),
            bool create = false,
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
        */
        virtual void authenticateEmail(
            const std::string& email,
            const std::string& password,
            const std::string& username = std::string(),
            bool create = false,
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
        */
        virtual void authenticateFacebook(
            const std::string& accessToken,
            const std::string& username = std::string(),
            bool create = false,
            bool importFriends = false,
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
        * Authenticate a user with a Google auth token.
        *
        * @param accessToken An OAuth access token from the Google SDK.
        * @param username A username used to create the user.
        * @param create True if the user should be created when authenticated.
        */
        virtual void authenticateGoogle(
            const std::string& accessToken,
            const std::string& username = std::string(),
            bool create = false,
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
         * @param create True if the user should be created when authenticated.
         * @param username A username used to create the user.
         */
        virtual void authenticateGameCenter(
            const std::string& playerId,
            const std::string& bundleId,
            uint64_t timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl,
            const std::string& username = std::string(),
            bool create = false,
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Authenticate a user with a custom id.
         *
         * @param id A custom identifier usually obtained from an external authentication service.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         */
        virtual void authenticateCustom(
            const std::string& id,
            const std::string& username = std::string(),
            bool create = false,
            std::function<void(NSessionPtr)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) = 0;

        /**
         * Authenticate a user with a Steam auth token.
         *
         * @param token An authentication token from the Steam network.
         * @param username A username used to create the user.
         * @param create True if the user should be created when authenticated.
         */
        virtual void authenticateSteam(
            const std::string& token,
            const std::string& username = std::string(),
            bool create = false,
            std::function<void(NSessionPtr)> successCallback = nullptr,
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
         * Create a group.
         *
         * @param session The session of the user.
         * @param name The name for the group.
         * @param description A description for the group.
         * @param avatarUrl An avatar url for the group.
         * @param langTag A language tag in BCP-47 format for the group.
         * @param open True if the group should have open membership.
         */
        virtual void createGroup(
            NSessionPtr session,
            const std::string& name,
            const std::string& description = "",
            const std::string& avatarUrl = "",
            const std::string& langTag = "",
            bool open = false,
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
    };

    using ClientPtr = std::shared_ptr<ClientInterface>;
}
