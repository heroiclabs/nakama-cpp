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

#include "nakama-cpp/ClientInterface.h"
#include "nakama-cpp/DefaultClient.h"
#include "api/github.com/heroiclabs/nakama/apigrpc/apigrpc.grpc.pb.h"
#include <set>

namespace Nakama {

    struct RpcRequest
    {
        grpc::ClientContext context;
        grpc::Status status;
        std::function<void()> successCallback;
        ErrorCallback errorCallback;
    };

    /**
     * A client to interact with Nakama server.
     * Don't use it directly, use `createDefaultClient` instead.
     */
    class DefaultClient : public ClientInterface
    {
    public:
        DefaultClient(const DefaultClientParameters& parameters);
        ~DefaultClient();

        void disconnect() override;

        void tick() override;

        void authenticateDevice(
            const std::string& id,
            const std::string& username,
            bool create,
            std::function<void(NSessionPtr)> successCallback,
            ErrorCallback errorCallback
        ) override;

        void authenticateEmail(
            const std::string& email,
            const std::string& password,
            const std::string& username,
            bool create,
            std::function<void(NSessionPtr)> successCallback,
            ErrorCallback errorCallback
        ) override;

        void authenticateFacebook(
            const std::string& accessToken,
            const std::string& username,
            bool create,
            bool importFriends,
            std::function<void(NSessionPtr)> successCallback,
            ErrorCallback errorCallback
        ) override;

        void authenticateGoogle(
            const std::string& accessToken,
            const std::string& username,
            bool create,
            std::function<void(NSessionPtr)> successCallback,
            ErrorCallback errorCallback
        ) override;

        void authenticateGameCenter(
            const std::string& playerId,
            const std::string& bundleId,
            uint64_t timestampSeconds,
            const std::string& salt,
            const std::string& signature,
            const std::string& publicKeyUrl,
            const std::string& username,
            bool create,
            std::function<void(NSessionPtr)> successCallback,
            ErrorCallback errorCallback
        ) override;

        void authenticateCustom(
            const std::string& id,
            const std::string& username,
            bool create,
            std::function<void(NSessionPtr)> successCallback,
            ErrorCallback errorCallback
        ) override;

        void authenticateSteam(
            const std::string& token,
            const std::string& username,
            bool create,
            std::function<void(NSessionPtr)> successCallback,
            ErrorCallback errorCallback
        ) override;

        void getAccount(
            NSessionPtr session,
            std::function<void(const NAccount&)> successCallback = nullptr,
            ErrorCallback errorCallback = nullptr
        ) override;

        void addFriends(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames,
            std::function<void()> successCallback,
            ErrorCallback errorCallback
        ) override;

        void deleteFriends(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames,
            std::function<void()> successCallback,
            ErrorCallback errorCallback
        ) override;

        void blockFriends(
            NSessionPtr session,
            const std::vector<std::string>& ids,
            const std::vector<std::string>& usernames,
            std::function<void()> successCallback,
            ErrorCallback errorCallback
        ) override;

        void createGroup(
            NSessionPtr session,
            const std::string& name,
            const std::string& description,
            const std::string& avatarUrl,
            const std::string& langTag,
            bool open,
            std::function<void(const NGroup&)> successCallback,
            ErrorCallback errorCallback
        ) override;

        void deleteGroup(
            NSessionPtr session,
            const std::string& groupId,
            std::function<void()> successCallback,
            ErrorCallback errorCallback
        ) override;

        void addGroupUsers(
            NSessionPtr session,
            const std::string& groupId,
            const std::vector<std::string>& ids,
            std::function<void()> successCallback,
            ErrorCallback errorCallback
        ) override;

    private:
        RpcRequest* createRpcRequest(NSessionPtr session);
        void onResponse(void* tag, bool ok);

    private:
        std::unique_ptr< nakama::api::Nakama::Stub> _stub;
        grpc::CompletionQueue _cq;
        std::string _basicAuthMetadata;
        std::set<RpcRequest*> _requests;
    };
}
