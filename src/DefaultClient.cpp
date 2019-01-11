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

#include "DefaultClient.h"
#include "nakama-cpp/StrUtil.h"
#include "DefaultSession.h"
#include "DataHelper.h"
#include <grpc++/create_channel.h>
#include <sstream>

using namespace std;

namespace Nakama {

ClientPtr createDefaultClient(const DefaultClientParameters& parameters)
{
    ClientPtr client(new DefaultClient(parameters));
    return client;
}

DefaultClient::DefaultClient(const DefaultClientParameters& parameters)
{
    std::string target = parameters.host + ":" + std::to_string(parameters.port);

    auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());

    _stub = nakama::api::Nakama::NewStub(channel);

    _basicAuthMetadata = "Basic " + base64_encode(parameters.serverKey + ":");
}

DefaultClient::~DefaultClient()
{
    disconnect();
}

void DefaultClient::disconnect()
{
    _cq.Shutdown();
}

void DefaultClient::tick()
{
    bool ok;
    void* tag;
    bool continueLoop = true;
    gpr_timespec timespec;

    timespec.clock_type = GPR_TIMESPAN;
    timespec.tv_sec = 0;
    timespec.tv_nsec = 0;

    do {
        switch (_cq.AsyncNext(&tag, &ok, timespec))
        {
        case grpc::CompletionQueue::SHUTDOWN:
            std::cerr << "The completion queue unexpectedly shutdown." << std::endl;
            continueLoop = false;
            break;

        case grpc::CompletionQueue::GOT_EVENT:
            std::cout << "call completed " << (int)(tag) << " ok: " << ok << std::endl;
            onResponse(tag, ok);
            break;

        case grpc::CompletionQueue::TIMEOUT:
            continueLoop = false;
            break;
        }
    } while (continueLoop);
}

RpcRequest * DefaultClient::createRpcRequest(NSessionPtr session)
{
    RpcRequest* rpcRequest = new RpcRequest();

    if (session)
    {
        rpcRequest->context.AddMetadata("authorization", "Bearer " + session->getAuthToken());
    }
    else
    {
        rpcRequest->context.AddMetadata("authorization", _basicAuthMetadata);
    }

    _requests.emplace(rpcRequest);
    return rpcRequest;
}

void DefaultClient::onResponse(void * tag, bool ok)
{
    auto it = _requests.find((RpcRequest*)tag);

    if (it != _requests.end())
    {
        RpcRequest* reqStatus = *it;

        if (ok)
        {
            if (reqStatus->status.ok())
            {
                if (reqStatus->successCallback)
                {
                    reqStatus->successCallback();
                }
            }
            else if (reqStatus->errorCallback)
            {
                std::stringstream ss;

                ss << "grpc call failed" << std::endl;
                ss << "code: "    << reqStatus->status.error_code() << std::endl;
                ss << "message: " << reqStatus->status.error_message() << std::endl;
                ss << "details: " << reqStatus->status.error_details();

                reqStatus->errorCallback(NError(
                    ss.str(),
                    ErrorCode::GrpcCallFailed
                ));
            }
        }
        else if (reqStatus->errorCallback)
        {
            reqStatus->errorCallback(NError(
                "grpc call failed",
                ErrorCode::GrpcCallFailed
            ));
        }

        delete reqStatus;
        _requests.erase(it);
    }
    else
    {
        std::cout << "DefaultClient::onResponse: not found tag " << tag << std::endl;
    }
}

void DefaultClient::authenticateDevice(
    const std::string& id,
    const opt::optional<std::string>& username,
    const opt::optional<bool>& create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(nullptr);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        rpcRequest->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
            successCallback(session);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AuthenticateDeviceRequest req;

    req.mutable_account()->set_id(id);

    if (username)
        req.set_username(*username);

    if (create)
        req.mutable_create()->set_value(*create);

    auto responseReader = _stub->AsyncAuthenticateDevice(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::authenticateEmail(
    const std::string & email,
    const std::string & password,
    const std::string & username,
    bool create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(nullptr);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        rpcRequest->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
            successCallback(session);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AuthenticateEmailRequest req;

    if (!email.empty())
        req.mutable_account()->set_email(email);

    req.mutable_account()->set_password(password);

    if (!username.empty())
        req.set_username(username);

    req.mutable_create()->set_value(create);

    auto responseReader = _stub->AsyncAuthenticateEmail(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::authenticateFacebook(
    const std::string & accessToken,
    const std::string & username,
    bool create,
    bool importFriends,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(nullptr);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        rpcRequest->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
            successCallback(session);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AuthenticateFacebookRequest req;

    req.mutable_account()->set_token(accessToken);

    if (!username.empty())
        req.set_username(username);

    req.mutable_create()->set_value(create);
    req.mutable_sync()->set_value(importFriends);

    auto responseReader = _stub->AsyncAuthenticateFacebook(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::authenticateGoogle(
    const std::string & accessToken,
    const std::string & username,
    bool create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(nullptr);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        rpcRequest->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
            successCallback(session);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AuthenticateGoogleRequest req;

    req.mutable_account()->set_token(accessToken);

    if (!username.empty())
        req.set_username(username);

    req.mutable_create()->set_value(create);

    auto responseReader = _stub->AsyncAuthenticateGoogle(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::authenticateGameCenter(
    const std::string & playerId,
    const std::string & bundleId,
    NTimestamp timestampSeconds,
    const std::string & salt,
    const std::string & signature,
    const std::string & publicKeyUrl,
    const std::string & username,
    bool create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(nullptr);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        rpcRequest->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
            successCallback(session);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AuthenticateGameCenterRequest req;

    req.mutable_account()->set_player_id(playerId);
    req.mutable_account()->set_bundle_id(bundleId);
    req.mutable_account()->set_timestamp_seconds(timestampSeconds);
    req.mutable_account()->set_salt(salt);
    req.mutable_account()->set_signature(signature);
    req.mutable_account()->set_public_key_url(publicKeyUrl);

    if (!username.empty())
        req.set_username(username);

    req.mutable_create()->set_value(create);

    auto responseReader = _stub->AsyncAuthenticateGameCenter(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::authenticateCustom(
    const std::string & id,
    const std::string & username,
    bool create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(nullptr);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        rpcRequest->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
            successCallback(session);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AuthenticateCustomRequest req;

    req.mutable_account()->set_id(id);

    if (!username.empty())
        req.set_username(username);

    req.mutable_create()->set_value(create);

    auto responseReader = _stub->AsyncAuthenticateCustom(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::authenticateSteam(
    const std::string & token,
    const std::string & username,
    bool create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(nullptr);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        rpcRequest->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
            successCallback(session);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AuthenticateSteamRequest req;

    req.mutable_account()->set_token(token);

    if (!username.empty())
        req.set_username(username);

    req.mutable_create()->set_value(create);

    auto responseReader = _stub->AsyncAuthenticateSteam(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::linkFacebook(
    NSessionPtr session,
    const std::string & accessToken,
    const opt::optional<bool>& importFriends,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::LinkFacebookRequest req;

    req.mutable_account()->set_token(accessToken);
    if (importFriends) req.mutable_sync()->set_value(*importFriends);

    auto responseReader = _stub->AsyncLinkFacebook(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::linkEmail(
    NSessionPtr session,
    const std::string & email,
    const std::string & password,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountEmail req;

    req.set_email(email);
    req.set_password(password);

    auto responseReader = _stub->AsyncLinkEmail(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::linkDevice(
    NSessionPtr session,
    const std::string & id,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountDevice req;

    req.set_id(id);

    auto responseReader = _stub->AsyncLinkDevice(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::linkGoogle(
    NSessionPtr session,
    const std::string & accessToken,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountGoogle req;

    req.set_token(accessToken);

    auto responseReader = _stub->AsyncLinkGoogle(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::linkGameCenter(
    NSessionPtr session,
    const std::string & playerId,
    const std::string & bundleId,
    NTimestamp timestampSeconds,
    const std::string & salt,
    const std::string & signature,
    const std::string & publicKeyUrl,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountGameCenter req;

    req.set_player_id(playerId);
    req.set_bundle_id(bundleId);
    req.set_timestamp_seconds(timestampSeconds);
    req.set_salt(salt);
    req.set_signature(signature);
    req.set_public_key_url(publicKeyUrl);

    auto responseReader = _stub->AsyncLinkGameCenter(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::linkSteam(
    NSessionPtr session,
    const std::string & token,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountSteam req;

    req.set_token(token);

    auto responseReader = _stub->AsyncLinkSteam(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::linkCustom(NSessionPtr session, const std::string & id, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountCustom req;

    req.set_id(id);

    auto responseReader = _stub->AsyncLinkCustom(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::unlinkFacebook(NSessionPtr session, const std::string & accessToken, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountFacebook req;

    req.set_token(accessToken);

    auto responseReader = _stub->AsyncUnlinkFacebook(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::unlinkEmail(NSessionPtr session, const std::string & email, const std::string & password, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountEmail req;

    req.set_email(email);
    req.set_password(password);

    auto responseReader = _stub->AsyncUnlinkEmail(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::unlinkGoogle(NSessionPtr session, const std::string & accessToken, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountGoogle req;

    req.set_token(accessToken);

    auto responseReader = _stub->AsyncUnlinkGoogle(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::unlinkGameCenter(NSessionPtr session, const std::string & playerId, const std::string & bundleId, NTimestamp timestampSeconds, const std::string & salt, const std::string & signature, const std::string & publicKeyUrl, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountGameCenter req;

    req.set_player_id(playerId);
    req.set_bundle_id(bundleId);
    req.set_timestamp_seconds(timestampSeconds);
    req.set_salt(salt);
    req.set_signature(signature);
    req.set_public_key_url(publicKeyUrl);

    auto responseReader = _stub->AsyncUnlinkGameCenter(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::unlinkSteam(NSessionPtr session, const std::string & token, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountSteam req;

    req.set_token(token);

    auto responseReader = _stub->AsyncUnlinkSteam(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::unlinkDevice(NSessionPtr session, const std::string & id, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountDevice req;

    req.set_id(id);

    auto responseReader = _stub->AsyncUnlinkDevice(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::unlinkCustom(NSessionPtr session, const std::string & id, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AccountCustom req;

    req.set_id(id);

    auto responseReader = _stub->AsyncUnlinkCustom(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::importFacebookFriends(
    NSessionPtr session,
    const std::string& token,
    const opt::optional<bool>& reset,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ImportFacebookFriendsRequest req;

    req.mutable_account()->set_token(token);
    if (reset) req.mutable_reset()->set_value(*reset);

    auto responseReader = _stub->AsyncImportFacebookFriends(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::getAccount(
    NSessionPtr session,
    std::function<void(const NAccount&)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto accoutData(make_shared<nakama::api::Account>());

    if (successCallback)
    {
        rpcRequest->successCallback = [accoutData, successCallback]()
        {
            NAccount account;
            assign(account, *accoutData);
            successCallback(account);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    auto responseReader = _stub->AsyncGetAccount(&rpcRequest->context, {}, &_cq);

    responseReader->Finish(&(*accoutData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::updateAccount(
    NSessionPtr session,
    const opt::optional<std::string>& username,
    const opt::optional<std::string>& displayName,
    const opt::optional<std::string>& avatarUrl,
    const opt::optional<std::string>& langTag,
    const opt::optional<std::string>& location,
    const opt::optional<std::string>& timezone,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::UpdateAccountRequest req;

    if (username) req.mutable_username()->set_value(*username);
    if (displayName) req.mutable_display_name()->set_value(*displayName);
    if (avatarUrl) req.mutable_avatar_url()->set_value(*avatarUrl);
    if (langTag) req.mutable_lang_tag()->set_value(*langTag);
    if (location) req.mutable_location()->set_value(*location);
    if (timezone) req.mutable_timezone()->set_value(*timezone);

    auto responseReader = _stub->AsyncUpdateAccount(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::getUsers(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    const std::vector<std::string>& facebookIds,
    std::function<void(const NUsers&)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto usersData(make_shared<nakama::api::Users>());

    if (successCallback)
    {
        rpcRequest->successCallback = [usersData, successCallback]()
        {
            NUsers users;
            assign(users, *usersData);
            successCallback(users);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::GetUsersRequest req;

    for (auto& id : ids)
    {
        req.mutable_ids()->Add()->assign(id);
    }

    for (auto& username : usernames)
    {
        req.mutable_usernames()->Add()->assign(username);
    }

    for (auto& facebookId : facebookIds)
    {
        req.mutable_facebook_ids()->Add()->assign(facebookId);
    }

    auto responseReader = _stub->AsyncGetUsers(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*usersData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::addFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AddFriendsRequest req;

    for (auto& id : ids)
    {
        req.mutable_ids()->Add()->assign(id);
    }

    for (auto& username : usernames)
    {
        req.mutable_usernames()->Add()->assign(username);
    }

    auto responseReader = _stub->AsyncAddFriends(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::deleteFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::DeleteFriendsRequest req;

    for (auto& id : ids)
    {
        req.mutable_ids()->Add()->assign(id);
    }

    for (auto& username : usernames)
    {
        req.mutable_usernames()->Add()->assign(username);
    }

    auto responseReader = _stub->AsyncDeleteFriends(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::blockFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::BlockFriendsRequest req;

    for (auto& id : ids)
    {
        req.mutable_ids()->Add()->assign(id);
    }

    for (auto& username : usernames)
    {
        req.mutable_usernames()->Add()->assign(username);
    }

    auto responseReader = _stub->AsyncBlockFriends(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listFriends(NSessionPtr session, std::function<void(NFriendsPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::Friends>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NFriendsPtr friends(new NFriends());
            assign(*friends, *data);
            successCallback(friends);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    auto responseReader = _stub->AsyncListFriends(&rpcRequest->context, {}, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::createGroup(
    NSessionPtr session,
    const std::string & name,
    const std::string & description,
    const std::string & avatarUrl,
    const std::string & langTag,
    bool open,
    std::function<void(const NGroup&)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto groupData(make_shared<nakama::api::Group>());

    if (successCallback)
    {
        rpcRequest->successCallback = [groupData, successCallback]()
        {
            NGroup group;
            assign(group, *groupData);
            successCallback(group);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::CreateGroupRequest req;

    req.set_name(name);

    if (!description.empty())
        req.set_description(description);

    if (!avatarUrl.empty())
        req.set_avatar_url(avatarUrl);

    if (!langTag.empty())
        req.set_lang_tag(langTag);

    req.set_open(open);

    auto responseReader = _stub->AsyncCreateGroup(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*groupData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::deleteGroup(
    NSessionPtr session,
    const std::string & groupId,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::DeleteGroupRequest req;

    req.set_group_id(groupId);

    auto responseReader = _stub->AsyncDeleteGroup(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::addGroupUsers(
    NSessionPtr session,
    const std::string & groupId,
    const std::vector<std::string>& ids,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AddGroupUsersRequest req;

    req.set_group_id(groupId);

    for (auto& id : ids)
    {
        req.add_user_ids(id);
    }

    auto responseReader = _stub->AsyncAddGroupUsers(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listGroupUsers(NSessionPtr session, const std::string & groupId, std::function<void(NGroupUserListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto groupData(make_shared<nakama::api::GroupUserList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [groupData, successCallback]()
        {
            NGroupUserListPtr users(new NGroupUserList());
            assign(*users, *groupData);
            successCallback(users);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListGroupUsersRequest req;

    req.set_group_id(groupId);

    auto responseReader = _stub->AsyncListGroupUsers(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*groupData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::kickGroupUsers(NSessionPtr session, const std::string & groupId, const std::vector<std::string>& ids, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::KickGroupUsersRequest req;

    req.set_group_id(groupId);

    for (auto& id : ids)
    {
        req.add_user_ids(id);
    }

    auto responseReader = _stub->AsyncKickGroupUsers(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::joinGroup(NSessionPtr session, const std::string & groupId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::JoinGroupRequest req;

    req.set_group_id(groupId);

    auto responseReader = _stub->AsyncJoinGroup(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::leaveGroup(NSessionPtr session, const std::string & groupId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::LeaveGroupRequest req;

    req.set_group_id(groupId);

    auto responseReader = _stub->AsyncLeaveGroup(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listGroups(NSessionPtr session, const std::string & name, int limit, const std::string & cursor, std::function<void(NGroupListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto groupData(make_shared<nakama::api::GroupList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [groupData, successCallback]()
        {
            NGroupListPtr groups(new NGroupList());
            assign(*groups, *groupData);
            successCallback(groups);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListGroupsRequest req;

    req.set_name(name);

    if (limit > 0)
        req.mutable_limit()->set_value(limit);

    if (!cursor.empty())
        req.set_cursor(cursor);

    auto responseReader = _stub->AsyncListGroups(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*groupData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listUserGroups(NSessionPtr session, std::function<void(NUserGroupListPtr)> successCallback, ErrorCallback errorCallback)
{
    listUserGroups(session, "", successCallback, errorCallback);
}

void DefaultClient::listUserGroups(NSessionPtr session, const std::string & userId, std::function<void(NUserGroupListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto groupData(make_shared<nakama::api::UserGroupList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [groupData, successCallback]()
        {
            NUserGroupListPtr groups(new NUserGroupList());
            assign(*groups, *groupData);
            successCallback(groups);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListUserGroupsRequest req;

    if (!userId.empty())
        req.set_user_id(userId);

    auto responseReader = _stub->AsyncListUserGroups(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*groupData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::promoteGroupUsers(NSessionPtr session, const std::string & groupId, const std::vector<std::string>& ids, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::PromoteGroupUsersRequest req;

    req.set_group_id(groupId);

    for (auto& id : ids)
    {
        req.add_user_ids(id);
    }

    auto responseReader = _stub->AsyncPromoteGroupUsers(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::updateGroup(
    NSessionPtr session,
    const std::string & groupId,
    const opt::optional<std::string>& name,
    const opt::optional<std::string>& description,
    const opt::optional<std::string>& avatarUrl,
    const opt::optional<std::string>& langTag,
    const opt::optional<bool>& open,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::UpdateGroupRequest req;

    req.set_group_id(groupId);

    if (name) req.mutable_name()->set_value(*name);
    if (description) req.mutable_description()->set_value(*description);
    if (avatarUrl) req.mutable_avatar_url()->set_value(*avatarUrl);
    if (langTag) req.mutable_lang_tag()->set_value(*langTag);
    if (open) req.mutable_open()->set_value(*open);

    auto responseReader = _stub->AsyncUpdateGroup(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listLeaderboardRecords(
    NSessionPtr session,
    const std::string & leaderboardId,
    const std::vector<std::string>& ownerIds,
    const opt::optional<int>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NLeaderboardRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::LeaderboardRecordList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NLeaderboardRecordListPtr list(new NLeaderboardRecordList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListLeaderboardRecordsRequest req;

    req.set_leaderboard_id(leaderboardId);

    for (auto& id : ownerIds)
    {
        req.add_owner_ids(id);
    }

    if (limit) req.mutable_limit()->set_value(*limit);
    if (cursor) req.set_cursor(*cursor);

    auto responseReader = _stub->AsyncListLeaderboardRecords(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listLeaderboardRecordsAroundOwner(NSessionPtr session, const std::string & leaderboardId, const std::string & ownerId, const opt::optional<int>& limit, std::function<void(NLeaderboardRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::LeaderboardRecordList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NLeaderboardRecordListPtr list(new NLeaderboardRecordList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListLeaderboardRecordsAroundOwnerRequest req;

    req.set_leaderboard_id(leaderboardId);
    req.set_owner_id(ownerId);

    if (limit) req.mutable_limit()->set_value(*limit);

    auto responseReader = _stub->AsyncListLeaderboardRecordsAroundOwner(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::writeLeaderboardRecord(
    NSessionPtr session,
    const std::string & leaderboardId,
    int64_t score,
    const opt::optional<int64_t>& subscore,
    const opt::optional<std::string>& metadata,
    std::function<void(NLeaderboardRecord)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::LeaderboardRecord>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NLeaderboardRecord record;
            assign(record, *data);
            successCallback(record);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::WriteLeaderboardRecordRequest req;

    req.set_leaderboard_id(leaderboardId);
    req.mutable_record()->set_score(score);
    if (subscore) req.mutable_record()->set_subscore(*subscore);
    if (metadata) req.mutable_record()->set_metadata(*metadata);

    auto responseReader = _stub->AsyncWriteLeaderboardRecord(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::writeTournamentRecord(
    NSessionPtr session,
    const std::string & tournamentId,
    int64_t score,
    const opt::optional<int64_t>& subscore,
    const opt::optional<std::string>& metadata,
    std::function<void(NLeaderboardRecord)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::LeaderboardRecord>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NLeaderboardRecord record;
            assign(record, *data);
            successCallback(record);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::WriteTournamentRecordRequest req;

    req.set_tournament_id(tournamentId);
    req.mutable_record()->set_score(score);
    if (subscore) req.mutable_record()->set_subscore(*subscore);
    if (metadata) req.mutable_record()->set_metadata(*metadata);

    auto responseReader = _stub->AsyncWriteTournamentRecord(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::deleteLeaderboardRecord(NSessionPtr session, const std::string & leaderboardId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::DeleteLeaderboardRecordRequest req;

    req.set_leaderboard_id(leaderboardId);

    auto responseReader = _stub->AsyncDeleteLeaderboardRecord(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listMatches(
    NSessionPtr session,
    const opt::optional<int>& min_size,
    const opt::optional<int>& max_size,
    const opt::optional<int>& limit,
    const opt::optional<std::string>& label,
    const opt::optional<bool>& authoritative,
    std::function<void(NMatchListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::MatchList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NMatchListPtr match_list(new NMatchList());
            assign(*match_list, *data);
            successCallback(match_list);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListMatchesRequest req;

    if (min_size) req.mutable_min_size()->set_value(*min_size);
    if (max_size) req.mutable_max_size()->set_value(*max_size);
    if (limit) req.mutable_limit()->set_value(*limit);
    if (label) req.mutable_label()->set_value(*label);
    if (authoritative) req.mutable_authoritative()->set_value(*authoritative);

    auto responseReader = _stub->AsyncListMatches(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listNotifications(
    NSessionPtr session,
    const opt::optional<int>& limit,
    const opt::optional<std::string>& cacheableCursor,
    std::function<void(NNotificationListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::NotificationList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NNotificationListPtr list(new NNotificationList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListNotificationsRequest req;

    if (limit) req.mutable_limit()->set_value(*limit);
    if (cacheableCursor) req.set_cacheable_cursor(*cacheableCursor);

    auto responseReader = _stub->AsyncListNotifications(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::deleteNotifications(NSessionPtr session, const std::vector<std::string>& notificationIds, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::DeleteNotificationsRequest req;

    for (auto& id : notificationIds)
    {
        req.add_ids(id);
    }

    auto responseReader = _stub->AsyncDeleteNotifications(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listChannelMessages(
    NSessionPtr session,
    const std::string & channelId,
    const opt::optional<int>& limit,
    const opt::optional<std::string>& cursor,
    const opt::optional<bool>& forward,
    std::function<void(NChannelMessageListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::ChannelMessageList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NChannelMessageListPtr list(new NChannelMessageList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListChannelMessagesRequest req;

    req.set_channel_id(channelId);
    if (limit) req.mutable_limit()->set_value(*limit);
    if (cursor) req.set_cursor(*cursor);
    if (forward) req.mutable_forward()->set_value(*forward);

    auto responseReader = _stub->AsyncListChannelMessages(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listTournaments(
    NSessionPtr session,
    const opt::optional<uint32_t>& categoryStart,
    const opt::optional<uint32_t>& categoryEnd,
    const opt::optional<uint32_t>& startTime,
    const opt::optional<uint32_t>& endTime,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NTournamentListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::TournamentList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NTournamentListPtr list(new NTournamentList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListTournamentsRequest req;

    if (categoryStart) req.mutable_category_start()->set_value(*categoryStart);
    if (categoryEnd) req.mutable_category_end()->set_value(*categoryEnd);
    if (startTime) req.mutable_start_time()->set_value(*startTime);
    if (endTime) req.mutable_end_time()->set_value(*endTime);
    if (limit) req.mutable_limit()->set_value(*limit);
    if (cursor) req.set_cursor(*cursor);

    auto responseReader = _stub->AsyncListTournaments(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listTournamentRecords(
    NSessionPtr session,
    const std::string & tournamentId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const std::vector<std::string>& ownerIds,
    std::function<void(NTournamentRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::TournamentRecordList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NTournamentRecordListPtr list(new NTournamentRecordList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListTournamentRecordsRequest req;

    req.set_tournament_id(tournamentId);

    if (limit) req.mutable_limit()->set_value(*limit);
    if (cursor) req.set_cursor(*cursor);

    for (auto& id : ownerIds)
    {
        req.add_owner_ids(id);
    }

    auto responseReader = _stub->AsyncListTournamentRecords(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::listTournamentRecordsAroundOwner(
    NSessionPtr session,
    const std::string & tournamentId,
    const std::string & ownerId,
    const opt::optional<int32_t>& limit,
    std::function<void(NTournamentRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);
    auto data(make_shared<nakama::api::TournamentRecordList>());

    if (successCallback)
    {
        rpcRequest->successCallback = [data, successCallback]()
        {
            NTournamentRecordListPtr list(new NTournamentRecordList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::ListTournamentRecordsAroundOwnerRequest req;

    req.set_tournament_id(tournamentId);
    req.set_owner_id(ownerId);

    if (limit) req.mutable_limit()->set_value(*limit);

    auto responseReader = _stub->AsyncListTournamentRecordsAroundOwner(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*data), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::joinTournament(NSessionPtr session, const std::string & tournamentId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    RpcRequest* rpcRequest = createRpcRequest(session);

    rpcRequest->successCallback = successCallback;
    rpcRequest->errorCallback = errorCallback;

    nakama::api::JoinTournamentRequest req;

    req.set_tournament_id(tournamentId);

    auto responseReader = _stub->AsyncJoinTournament(&rpcRequest->context, req, &_cq);

    responseReader->Finish(nullptr, &rpcRequest->status, (void*)rpcRequest);
}

}
