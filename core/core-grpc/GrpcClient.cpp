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


#include "GrpcClient.h"
#include "nakama-cpp/log/NLogger.h"
#include "StrUtil.h"
#include "nakama-cpp/NakamaVersion.h"
#include "DefaultSession.h"
#include "DataHelper.h"
#include "grpcpp/create_channel.h"

#ifdef NAKAMA_SSL_ENABLED
    #include "roots_pem.h"
#endif

#undef NMODULE_NAME
#define NMODULE_NAME "Nakama::GrpcClient"

using namespace std;

namespace Nakama {

GrpcClient::GrpcClient(const NClientParameters& parameters)
{
    NLOG(NLogLevel::Info, "Created. NakamaSdkVersion: %s", getNakamaSdkVersion());

    _host = parameters.host;
    _ssl = parameters.ssl;
    _platformParams = parameters.platformParams;

    _port = parameters.port;

    if (port == DEFAULT_PORT)
    {
        port = parameters.ssl ? 443 : 7349;
        NLOG(NLogLevel::Info, "using default port %d", port);
    }

    std::string target = parameters.host + ":" + std::to_string(port);

    std::shared_ptr<grpc::ChannelCredentials> creds;

#if defined(NAKAMA_SSL_ENABLED)
    if (_ssl)
    {
        grpc::SslCredentialsOptions options;
        options.pem_root_certs = g_roots_pem;
        creds = grpc::SslCredentials(options);
    }
    else
#endif
    {
#if !defined(NAKAMA_SSL_ENABLED)
        if (_ssl)
        {
            NLOG_WARN("SSL is not supported");
        }
#endif

        creds = grpc::InsecureChannelCredentials();
    }

    auto channel = grpc::CreateChannel(target, creds);

    _stub = nakama::api::Nakama::NewStub(channel);

    _basicAuthMetadata = "Basic " + base64Encode(parameters.serverKey + ":");
}

GrpcClient::~GrpcClient()
{
    disconnect();

    if (_reqContexts.size() > 0)
    {
        NLOG(NLogLevel::Warn, "Not handled %u request(s) detected.", _reqContexts.size());

        for (ReqContext* reqContext : _reqContexts)
        {
            delete reqContext;
        }

        _reqContexts.clear();
    }
}

void GrpcClient::disconnect()
{
    _cq.Shutdown();
}

void GrpcClient::tick()
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
            NLOG_DEBUG("completion queue is stopped");
            continueLoop = false;
            break;

        case grpc::CompletionQueue::GOT_EVENT:
            onResponse(tag, ok);
            break;

        case grpc::CompletionQueue::TIMEOUT:
        default:
            continueLoop = false;
            break;
        }
    } while (continueLoop);
}

ReqContext * GrpcClient::createReqContext()
{
    ReqContext* ctx = new ReqContext();
    _reqContexts.emplace(ctx);
    return ctx;
}

void GrpcClient::setBasicAuth(ReqContext* ctx)
{
    ctx->context.AddMetadata("authorization", _basicAuthMetadata);
}

void GrpcClient::setSessionAuth(ReqContext* ctx, NSessionPtr session)
{
    ctx->context.AddMetadata("authorization", "Bearer " + session->getAuthToken());
}

void GrpcClient::onResponse(void * tag, bool ok)
{
    ReqContext* reqContext = static_cast<ReqContext*>(tag);

    auto it = _reqContexts.find(reqContext);

    if (it != _reqContexts.end())
    {
        if (ok)
        {
            if (reqContext->status.ok())
            {
                if (reqContext->successCallback)
                {
                    reqContext->successCallback();
                }
            }
            else
            {
                std::string errMessage;

                errMessage.append("message: ").append(reqContext->status.error_message());

                if (!reqContext->status.error_details().empty())
                {
                    errMessage.append("\ndetails: ").append(reqContext->status.error_details());
                }

                ErrorCode code = ErrorCode::Unknown;

                switch (reqContext->status.error_code())
                {
                    case grpc::StatusCode::UNAVAILABLE      : code = ErrorCode::ConnectionError; break;
                    case grpc::StatusCode::INTERNAL         : code = ErrorCode::InternalError; break;
                    case grpc::StatusCode::NOT_FOUND        : code = ErrorCode::NotFound; break;
                    case grpc::StatusCode::ALREADY_EXISTS   : code = ErrorCode::AlreadyExists; break;
                    case grpc::StatusCode::INVALID_ARGUMENT : code = ErrorCode::InvalidArgument; break;
                    case grpc::StatusCode::UNAUTHENTICATED  : code = ErrorCode::Unauthenticated; break;
                    case grpc::StatusCode::PERMISSION_DENIED: code = ErrorCode::PermissionDenied; break;

                default:
                    errMessage.append("\ngrpc code: ").append(std::to_string(reqContext->status.error_code()));
                    break;
                }

                reqError(reqContext, NError(std::move(errMessage), code));
            }
        }
        else
        {
            reqError(reqContext, NError("Communication failed. Please check connection.", ErrorCode::ConnectionError));
        }

        delete reqContext;
        _reqContexts.erase(it);
    }
    else
    {
        reqError(nullptr, NError("Not found request context.", ErrorCode::InternalError));
    }
}

void GrpcClient::reqError(ReqContext * reqContext, const NError & error)
{
    NLOG_ERROR(error);

    if (reqContext && reqContext->errorCallback)
    {
        reqContext->errorCallback(error);
    }
    else if (_defaultErrorCallback)
    {
        _defaultErrorCallback(error);
    }
    else
    {
        NLOG_WARN("^ error not handled");
    }
}

void GrpcClient::authenticateDevice(
    const std::string& id,
    const opt::optional<std::string>& username,
    const opt::optional<bool>& create,
    const NStringMap& vars,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setBasicAuth(ctx);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        ctx->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->refresh_token(), sessionData->created()));
            successCallback(session);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::AuthenticateDeviceRequest req;

    auto* account = req.mutable_account();
    account->set_id(id);
    if (username) req.set_username(*username);
    if (create) req.mutable_create()->set_value(*create);

    for (auto& p : vars)
    {
        (*account->mutable_vars())[p.first] = p.second;
    }

    auto responseReader = _stub->AsyncAuthenticateDevice(&ctx->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &ctx->status, (void*)ctx);
}

void GrpcClient::authenticateEmail(
    const std::string& email,
    const std::string& password,
    const std::string& username,
    bool create,
    const NStringMap& vars,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setBasicAuth(ctx);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        ctx->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->refresh_token(), sessionData->created()));
            successCallback(session);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::AuthenticateEmailRequest req;

    auto* account = req.mutable_account();
    account->set_email(email);
    account->set_password(password);
    if (!username.empty()) req.set_username(username);
    req.mutable_create()->set_value(create);

    for (auto& p : vars)
    {
        (*account->mutable_vars())[p.first] = p.second;
    }

    auto responseReader = _stub->AsyncAuthenticateEmail(&ctx->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &ctx->status, (void*)ctx);
}

void GrpcClient::authenticateFacebook(
    const std::string & accessToken,
    const std::string & username,
    bool create,
    bool importFriends,
    const NStringMap& vars,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setBasicAuth(ctx);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        ctx->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->refresh_token(), sessionData->created()));
            successCallback(session);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::AuthenticateFacebookRequest req;

    auto* account = req.mutable_account();
    account->set_token(accessToken);
    if (!username.empty()) req.set_username(username);
    req.mutable_create()->set_value(create);
    req.mutable_sync()->set_value(importFriends);

    for (auto& p : vars)
    {
        (*account->mutable_vars())[p.first] = p.second;
    }

    auto responseReader = _stub->AsyncAuthenticateFacebook(&ctx->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &ctx->status, (void*)ctx);
}

void GrpcClient::authenticateGoogle(
    const std::string & accessToken,
    const std::string & username,
    bool create,
    const NStringMap& vars,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setBasicAuth(ctx);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        ctx->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->refresh_token(), sessionData->created()));
            successCallback(session);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::AuthenticateGoogleRequest req;

    auto* account = req.mutable_account();
    account->set_token(accessToken);
    if (!username.empty()) req.set_username(username);
    req.mutable_create()->set_value(create);

    for (auto& p : vars)
    {
        (*account->mutable_vars())[p.first] = p.second;
    }

    auto responseReader = _stub->AsyncAuthenticateGoogle(&ctx->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &ctx->status, (void*)ctx);
}

void GrpcClient::authenticateGameCenter(
    const std::string & playerId,
    const std::string & bundleId,
    NTimestamp timestampSeconds,
    const std::string & salt,
    const std::string & signature,
    const std::string & publicKeyUrl,
    const std::string & username,
    bool create,
    const NStringMap& vars,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setBasicAuth(ctx);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        ctx->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->refresh_token(), sessionData->created()));
            successCallback(session);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::AuthenticateGameCenterRequest req;

    {
        auto* account = req.mutable_account();

        account->set_player_id(playerId);
        account->set_bundle_id(bundleId);
        account->set_timestamp_seconds(timestampSeconds);
        account->set_salt(salt);
        account->set_signature(signature);
        account->set_public_key_url(publicKeyUrl);

        for (auto& p : vars)
        {
            (*account->mutable_vars())[p.first] = p.second;
        }
    }

    if (!username.empty())
        req.set_username(username);

    req.mutable_create()->set_value(create);

    auto responseReader = _stub->AsyncAuthenticateGameCenter(&ctx->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &ctx->status, (void*)ctx);
}

void GrpcClient::authenticateApple(
    const std::string& token,
    const std::string& username,
    bool create,
    const NStringMap& vars,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setBasicAuth(ctx);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        ctx->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->refresh_token(), sessionData->created()));
            successCallback(session);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::AuthenticateAppleRequest req;

    auto* account = req.mutable_account();
    account->set_token(token);
    if (!username.empty()) req.set_username(username);
    req.mutable_create()->set_value(create);

    for (auto& p : vars)
    {
        (*account->mutable_vars())[p.first] = p.second;
    }

    auto responseReader = _stub->AsyncAuthenticateApple(&ctx->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &ctx->status, (void*)ctx);
}

void GrpcClient::authenticateCustom(
    const std::string & id,
    const std::string & username,
    bool create,
    const NStringMap& vars,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setBasicAuth(ctx);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        ctx->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->refresh_token(), sessionData->created()));
            successCallback(session);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::AuthenticateCustomRequest req;

    auto* account = req.mutable_account();
    account->set_id(id);
    if (!username.empty()) req.set_username(username);
    req.mutable_create()->set_value(create);

    for (auto& p : vars)
    {
        (*account->mutable_vars())[p.first] = p.second;
    }

    auto responseReader = _stub->AsyncAuthenticateCustom(&ctx->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &ctx->status, (void*)ctx);
}

void GrpcClient::authenticateSteam(
    const std::string & token,
    const std::string & username,
    bool create,
    const NStringMap& vars,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setBasicAuth(ctx);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        ctx->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->refresh_token(), sessionData->created()));
            successCallback(session);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::AuthenticateSteamRequest req;

    auto* account = req.mutable_account();
    account->set_token(token);
    if (!username.empty()) req.set_username(username);
    req.mutable_create()->set_value(create);

    for (auto& p : vars)
    {
        (*account->mutable_vars())[p.first] = p.second;
    }

    auto responseReader = _stub->AsyncAuthenticateSteam(&ctx->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &ctx->status, (void*)ctx);
}

void GrpcClient::authenticateRefresh(
        NSessionPtr session,
        std::function<void(NSessionPtr)> successCallback,
        ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setBasicAuth(ctx);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        ctx->successCallback = [sessionData, successCallback]()
        {
            NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->refresh_token(), sessionData->created()));
            successCallback(session);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::SessionRefreshRequest req;

    req.set_token(session->getRefreshToken());

    auto responseReader = _stub->AsyncSessionRefresh(&ctx->context, req, &_cq);
    responseReader->Finish(&(*sessionData), &ctx->status, (void*)ctx);
}

void GrpcClient::linkFacebook(
    NSessionPtr session,
    const std::string & accessToken,
    const opt::optional<bool>& importFriends,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::LinkFacebookRequest req;

    req.mutable_account()->set_token(accessToken);
    if (importFriends) req.mutable_sync()->set_value(*importFriends);

    auto responseReader = _stub->AsyncLinkFacebook(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::linkEmail(
    NSessionPtr session,
    const std::string & email,
    const std::string & password,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountEmail req;

    req.set_email(email);
    req.set_password(password);

    auto responseReader = _stub->AsyncLinkEmail(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::linkDevice(
    NSessionPtr session,
    const std::string & id,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountDevice req;

    req.set_id(id);

    auto responseReader = _stub->AsyncLinkDevice(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::linkGoogle(
    NSessionPtr session,
    const std::string & accessToken,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountGoogle req;

    req.set_token(accessToken);

    auto responseReader = _stub->AsyncLinkGoogle(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::linkGameCenter(
    NSessionPtr session,
    const std::string & playerId,
    const std::string & bundleId,
    NTimestamp timestampSeconds,
    const std::string & salt,
    const std::string & signature,
    const std::string & publicKeyUrl,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountGameCenter req;

    req.set_player_id(playerId);
    req.set_bundle_id(bundleId);
    req.set_timestamp_seconds(timestampSeconds);
    req.set_salt(salt);
    req.set_signature(signature);
    req.set_public_key_url(publicKeyUrl);

    auto responseReader = _stub->AsyncLinkGameCenter(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::linkApple(
    NSessionPtr session,
    const std::string& token,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountApple req;

    req.set_token(token);

    auto responseReader = _stub->AsyncLinkApple(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::linkSteam(
    NSessionPtr session,
    const std::string & token,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::LinkSteamRequest req;
    req.mutable_account()->set_token(token);
    req.mutable_sync()->set_value(true);  // REST syncs when field is missing. Do the same in gRPC

    auto responseReader = _stub->AsyncLinkSteam(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::linkCustom(NSessionPtr session, const std::string & id, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountCustom req;

    req.set_id(id);

    auto responseReader = _stub->AsyncLinkCustom(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::unlinkFacebook(NSessionPtr session, const std::string & accessToken, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountFacebook req;

    req.set_token(accessToken);

    auto responseReader = _stub->AsyncUnlinkFacebook(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::unlinkEmail(NSessionPtr session, const std::string & email, const std::string & password, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountEmail req;

    req.set_email(email);
    req.set_password(password);

    auto responseReader = _stub->AsyncUnlinkEmail(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::unlinkGoogle(NSessionPtr session, const std::string & accessToken, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountGoogle req;

    req.set_token(accessToken);

    auto responseReader = _stub->AsyncUnlinkGoogle(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::unlinkGameCenter(NSessionPtr session, const std::string & playerId, const std::string & bundleId, NTimestamp timestampSeconds, const std::string & salt, const std::string & signature, const std::string & publicKeyUrl, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountGameCenter req;

    req.set_player_id(playerId);
    req.set_bundle_id(bundleId);
    req.set_timestamp_seconds(timestampSeconds);
    req.set_salt(salt);
    req.set_signature(signature);
    req.set_public_key_url(publicKeyUrl);

    auto responseReader = _stub->AsyncUnlinkGameCenter(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::unlinkApple(NSessionPtr session, const std::string& token, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountApple req;

    req.set_token(token);

    auto responseReader = _stub->AsyncUnlinkApple(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::unlinkSteam(NSessionPtr session, const std::string & token, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountSteam req;

    req.set_token(token);

    auto responseReader = _stub->AsyncUnlinkSteam(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::unlinkDevice(NSessionPtr session, const std::string & id, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountDevice req;

    req.set_id(id);

    auto responseReader = _stub->AsyncUnlinkDevice(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::unlinkCustom(NSessionPtr session, const std::string & id, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AccountCustom req;

    req.set_id(id);

    auto responseReader = _stub->AsyncUnlinkCustom(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::importFacebookFriends(
    NSessionPtr session,
    const std::string& token,
    const opt::optional<bool>& reset,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::ImportFacebookFriendsRequest req;

    req.mutable_account()->set_token(token);
    if (reset) req.mutable_reset()->set_value(*reset);

    auto responseReader = _stub->AsyncImportFacebookFriends(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::getAccount(
    NSessionPtr session,
    std::function<void(const NAccount&)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto accoutData(make_shared<nakama::api::Account>());

    if (successCallback)
    {
        ctx->successCallback = [accoutData, successCallback]()
        {
            NAccount account;
            assign(account, *accoutData);
            successCallback(account);
        };
    }
    ctx->errorCallback = errorCallback;

    auto responseReader = _stub->AsyncGetAccount(&ctx->context, {}, &_cq);

    responseReader->Finish(&(*accoutData), &ctx->status, (void*)ctx);
}

void GrpcClient::updateAccount(
    NSessionPtr session,
    const opt::optional<std::string>& username,
    const opt::optional<std::string>& displayName,
    const opt::optional<std::string>& avatarUrl,
    const opt::optional<std::string>& langTag,
    const opt::optional<std::string>& location,
    const opt::optional<std::string>& timezone,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::UpdateAccountRequest req;

    if (username) req.mutable_username()->set_value(*username);
    if (displayName) req.mutable_display_name()->set_value(*displayName);
    if (avatarUrl) req.mutable_avatar_url()->set_value(*avatarUrl);
    if (langTag) req.mutable_lang_tag()->set_value(*langTag);
    if (location) req.mutable_location()->set_value(*location);
    if (timezone) req.mutable_timezone()->set_value(*timezone);

    auto responseReader = _stub->AsyncUpdateAccount(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::getUsers(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    const std::vector<std::string>& facebookIds,
    std::function<void(const NUsers&)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto usersData(make_shared<nakama::api::Users>());

    if (successCallback)
    {
        ctx->successCallback = [usersData, successCallback]()
        {
            NUsers users;
            assign(users, *usersData);
            successCallback(users);
        };
    }
    ctx->errorCallback = errorCallback;

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

    auto responseReader = _stub->AsyncGetUsers(&ctx->context, req, &_cq);

    responseReader->Finish(&(*usersData), &ctx->status, (void*)ctx);
}

void GrpcClient::addFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AddFriendsRequest req;

    for (auto& id : ids)
    {
        req.mutable_ids()->Add()->assign(id);
    }

    for (auto& username : usernames)
    {
        req.mutable_usernames()->Add()->assign(username);
    }

    auto responseReader = _stub->AsyncAddFriends(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::deleteFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::DeleteFriendsRequest req;

    for (auto& id : ids)
    {
        req.mutable_ids()->Add()->assign(id);
    }

    for (auto& username : usernames)
    {
        req.mutable_usernames()->Add()->assign(username);
    }

    auto responseReader = _stub->AsyncDeleteFriends(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::blockFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::BlockFriendsRequest req;

    for (auto& id : ids)
    {
        req.mutable_ids()->Add()->assign(id);
    }

    for (auto& username : usernames)
    {
        req.mutable_usernames()->Add()->assign(username);
    }

    auto responseReader = _stub->AsyncBlockFriends(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::listFriends(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<NFriend::State>& state,
    const std::string& cursor,
    std::function<void(NFriendListPtr)> successCallback,
    ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::FriendList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NFriendListPtr friends(new NFriendList());
            assign(*friends, *data);
            successCallback(friends);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListFriendsRequest req;

    if (limit) req.mutable_limit()->set_value(*limit);
    if (state) req.mutable_state()->set_value((google::protobuf::int32)*state);
    if (!cursor.empty()) req.set_cursor(cursor);

    auto responseReader = _stub->AsyncListFriends(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::createGroup(
    NSessionPtr session,
    const std::string & name,
    const std::string & description,
    const std::string & avatarUrl,
    const std::string & langTag,
    bool open,
    const opt::optional<int32_t>& maxCount,
    std::function<void(const NGroup&)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto groupData(make_shared<nakama::api::Group>());

    if (successCallback)
    {
        ctx->successCallback = [groupData, successCallback]()
        {
            NGroup group;
            assign(group, *groupData);
            successCallback(group);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::CreateGroupRequest req;

    req.set_name(name);
    if (!description.empty()) req.set_description(description);
    if (!avatarUrl.empty()) req.set_avatar_url(avatarUrl);
    if (!langTag.empty()) req.set_lang_tag(langTag);
    if (maxCount) req.set_max_count(*maxCount);
    req.set_open(open);

    auto responseReader = _stub->AsyncCreateGroup(&ctx->context, req, &_cq);

    responseReader->Finish(&(*groupData), &ctx->status, (void*)ctx);
}

void GrpcClient::deleteGroup(
    NSessionPtr session,
    const std::string & groupId,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::DeleteGroupRequest req;

    req.set_group_id(groupId);

    auto responseReader = _stub->AsyncDeleteGroup(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::addGroupUsers(
    NSessionPtr session,
    const std::string & groupId,
    const std::vector<std::string>& ids,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::AddGroupUsersRequest req;

    req.set_group_id(groupId);

    for (auto& id : ids)
    {
        req.add_user_ids(id);
    }

    auto responseReader = _stub->AsyncAddGroupUsers(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::listGroupUsers(
    NSessionPtr session,
    const std::string& groupId,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor,
    std::function<void(NGroupUserListPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto groupData(make_shared<nakama::api::GroupUserList>());

    if (successCallback)
    {
        ctx->successCallback = [groupData, successCallback]()
        {
            NGroupUserListPtr users(new NGroupUserList());
            assign(*users, *groupData);
            successCallback(users);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListGroupUsersRequest req;

    req.set_group_id(groupId);
    if (limit) req.mutable_limit()->set_value(*limit);
    if (state) req.mutable_state()->set_value((google::protobuf::int32)*state);
    if (!cursor.empty()) req.set_cursor(cursor);

    auto responseReader = _stub->AsyncListGroupUsers(&ctx->context, req, &_cq);

    responseReader->Finish(&(*groupData), &ctx->status, (void*)ctx);
}

void GrpcClient::kickGroupUsers(NSessionPtr session, const std::string & groupId, const std::vector<std::string>& ids, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::KickGroupUsersRequest req;

    req.set_group_id(groupId);

    for (auto& id : ids)
    {
        req.add_user_ids(id);
    }

    auto responseReader = _stub->AsyncKickGroupUsers(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::joinGroup(NSessionPtr session, const std::string & groupId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::JoinGroupRequest req;

    req.set_group_id(groupId);

    auto responseReader = _stub->AsyncJoinGroup(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::leaveGroup(NSessionPtr session, const std::string & groupId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::LeaveGroupRequest req;

    req.set_group_id(groupId);

    auto responseReader = _stub->AsyncLeaveGroup(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::listGroups(NSessionPtr session, const std::string & name, int32_t limit, const std::string & cursor, std::function<void(NGroupListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto groupData(make_shared<nakama::api::GroupList>());

    if (successCallback)
    {
        ctx->successCallback = [groupData, successCallback]()
        {
            NGroupListPtr groups(new NGroupList());
            assign(*groups, *groupData);
            successCallback(groups);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListGroupsRequest req;

    req.set_name(name);

    if (limit > 0)
        req.mutable_limit()->set_value(limit);

    if (!cursor.empty())
        req.set_cursor(cursor);

    auto responseReader = _stub->AsyncListGroups(&ctx->context, req, &_cq);

    responseReader->Finish(&(*groupData), &ctx->status, (void*)ctx);
}

void GrpcClient::listUserGroups(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor,
    std::function<void(NUserGroupListPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    if (session)
    {
        listUserGroups(session, session->getUserId(), limit, state, cursor, successCallback, errorCallback);
    }
    else
    {
        NError error("No session", ErrorCode::InvalidArgument);

        NLOG_ERROR(toString(error));

        if (errorCallback)
        {
            errorCallback(error);
        }
    }
}

void GrpcClient::listUserGroups(
    NSessionPtr session,
    const std::string& userId,
    const opt::optional<int32_t>& limit,
    const opt::optional<NUserGroupState>& state,
    const std::string& cursor,
    std::function<void(NUserGroupListPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto groupData(make_shared<nakama::api::UserGroupList>());

    if (successCallback)
    {
        ctx->successCallback = [groupData, successCallback]()
        {
            NUserGroupListPtr groups(new NUserGroupList());
            assign(*groups, *groupData);
            successCallback(groups);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListUserGroupsRequest req;

    req.set_user_id(userId);
    if (limit) req.mutable_limit()->set_value(*limit);
    if (state) req.mutable_state()->set_value((google::protobuf::int32)*state);
    if (!cursor.empty()) req.set_cursor(cursor);

    auto responseReader = _stub->AsyncListUserGroups(&ctx->context, req, &_cq);

    responseReader->Finish(&(*groupData), &ctx->status, (void*)ctx);
}

void GrpcClient::promoteGroupUsers(NSessionPtr session, const std::string & groupId, const std::vector<std::string>& ids, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::PromoteGroupUsersRequest req;

    req.set_group_id(groupId);

    for (auto& id : ids)
    {
        req.add_user_ids(id);
    }

    auto responseReader = _stub->AsyncPromoteGroupUsers(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::demoteGroupUsers(
    NSessionPtr session,
    const std::string& groupId,
    const std::vector<std::string>& ids,
    std::function<void()> successCallback,
    ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::DemoteGroupUsersRequest req;

    req.set_group_id(groupId);

    for (auto& id : ids)
    {
        req.add_user_ids(id);
    }

    auto responseReader = _stub->AsyncDemoteGroupUsers(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::updateGroup(
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
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::UpdateGroupRequest req;

    req.set_group_id(groupId);

    if (name) req.mutable_name()->set_value(*name);
    if (description) req.mutable_description()->set_value(*description);
    if (avatarUrl) req.mutable_avatar_url()->set_value(*avatarUrl);
    if (langTag) req.mutable_lang_tag()->set_value(*langTag);
    if (open) req.mutable_open()->set_value(*open);

    auto responseReader = _stub->AsyncUpdateGroup(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::listLeaderboardRecords(
    NSessionPtr session,
    const std::string & leaderboardId,
    const std::vector<std::string>& ownerIds,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NLeaderboardRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::LeaderboardRecordList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NLeaderboardRecordListPtr list(new NLeaderboardRecordList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListLeaderboardRecordsRequest req;

    req.set_leaderboard_id(leaderboardId);

    for (auto& id : ownerIds)
    {
        req.add_owner_ids(id);
    }

    if (limit) req.mutable_limit()->set_value(*limit);
    if (cursor) req.set_cursor(*cursor);

    auto responseReader = _stub->AsyncListLeaderboardRecords(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::listLeaderboardRecordsAroundOwner(NSessionPtr session, const std::string & leaderboardId, const std::string & ownerId, const opt::optional<int32_t>& limit, std::function<void(NLeaderboardRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::LeaderboardRecordList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NLeaderboardRecordListPtr list(new NLeaderboardRecordList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListLeaderboardRecordsAroundOwnerRequest req;

    req.set_leaderboard_id(leaderboardId);
    req.set_owner_id(ownerId);

    if (limit) req.mutable_limit()->set_value(*limit);

    auto responseReader = _stub->AsyncListLeaderboardRecordsAroundOwner(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::writeLeaderboardRecord(
    NSessionPtr session,
    const std::string & leaderboardId,
    std::int64_t score,
    const opt::optional<std::int64_t>& subscore,
    const opt::optional<std::string>& metadata,
    std::function<void(NLeaderboardRecord)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::LeaderboardRecord>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NLeaderboardRecord record;
            assign(record, *data);
            successCallback(record);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::WriteLeaderboardRecordRequest req;

    req.set_leaderboard_id(leaderboardId);
    req.mutable_record()->set_score(score);
    if (subscore) req.mutable_record()->set_subscore(*subscore);
    if (metadata) req.mutable_record()->set_metadata(*metadata);

    auto responseReader = _stub->AsyncWriteLeaderboardRecord(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::writeTournamentRecord(
    NSessionPtr session,
    const std::string & tournamentId,
    std::int64_t score,
    const opt::optional<std::int64_t>& subscore,
    const opt::optional<std::string>& metadata,
    std::function<void(NLeaderboardRecord)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::LeaderboardRecord>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NLeaderboardRecord record;
            assign(record, *data);
            successCallback(record);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::WriteTournamentRecordRequest req;

    req.set_tournament_id(tournamentId);
    req.mutable_record()->set_score(score);
    if (subscore) req.mutable_record()->set_subscore(*subscore);
    if (metadata) req.mutable_record()->set_metadata(*metadata);

    auto responseReader = _stub->AsyncWriteTournamentRecord(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::deleteLeaderboardRecord(NSessionPtr session, const std::string & leaderboardId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::DeleteLeaderboardRecordRequest req;

    req.set_leaderboard_id(leaderboardId);

    auto responseReader = _stub->AsyncDeleteLeaderboardRecord(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::listMatches(
    NSessionPtr session,
    const opt::optional<int32_t>& min_size,
    const opt::optional<int32_t>& max_size,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& label,
    const opt::optional<std::string>& query,
    const opt::optional<bool>& authoritative,
    std::function<void(NMatchListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::MatchList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NMatchListPtr match_list(new NMatchList());
            assign(*match_list, *data);
            successCallback(match_list);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListMatchesRequest req;

    if (min_size) req.mutable_min_size()->set_value(*min_size);
    if (max_size) req.mutable_max_size()->set_value(*max_size);
    if (limit) req.mutable_limit()->set_value(*limit);
    if (label) req.mutable_label()->set_value(*label);
    if (query) req.mutable_query()->set_value(*query);
    if (authoritative) req.mutable_authoritative()->set_value(*authoritative);

    auto responseReader = _stub->AsyncListMatches(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::listNotifications(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cacheableCursor,
    std::function<void(NNotificationListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::NotificationList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NNotificationListPtr list(new NNotificationList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListNotificationsRequest req;

    if (limit) req.mutable_limit()->set_value(*limit);
    if (cacheableCursor) req.set_cacheable_cursor(*cacheableCursor);

    auto responseReader = _stub->AsyncListNotifications(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::deleteNotifications(NSessionPtr session, const std::vector<std::string>& notificationIds, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::DeleteNotificationsRequest req;

    for (auto& id : notificationIds)
    {
        req.add_ids(id);
    }

    auto responseReader = _stub->AsyncDeleteNotifications(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::listChannelMessages(
    NSessionPtr session,
    const std::string & channelId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const opt::optional<bool>& forward,
    std::function<void(NChannelMessageListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::ChannelMessageList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NChannelMessageListPtr list(new NChannelMessageList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListChannelMessagesRequest req;

    req.set_channel_id(channelId);
    if (limit) req.mutable_limit()->set_value(*limit);
    if (cursor) req.set_cursor(*cursor);
    if (forward) req.mutable_forward()->set_value(*forward);

    auto responseReader = _stub->AsyncListChannelMessages(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::listTournaments(
    NSessionPtr session,
    const opt::optional<uint32_t>& categoryStart,
    const opt::optional<uint32_t>& categoryEnd,
    const opt::optional<uint32_t>& startTime,
    const opt::optional<uint32_t>& endTime,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NTournamentListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::TournamentList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NTournamentListPtr list(new NTournamentList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListTournamentsRequest req;

    if (categoryStart) req.mutable_category_start()->set_value(*categoryStart);
    if (categoryEnd) req.mutable_category_end()->set_value(*categoryEnd);
    if (startTime) req.mutable_start_time()->set_value(*startTime);
    if (endTime) req.mutable_end_time()->set_value(*endTime);
    if (limit) req.mutable_limit()->set_value(*limit);
    if (cursor) req.set_cursor(*cursor);

    auto responseReader = _stub->AsyncListTournaments(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::listTournamentRecords(
    NSessionPtr session,
    const std::string & tournamentId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const std::vector<std::string>& ownerIds,
    std::function<void(NTournamentRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::TournamentRecordList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NTournamentRecordListPtr list(new NTournamentRecordList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListTournamentRecordsRequest req;

    req.set_tournament_id(tournamentId);

    if (limit) req.mutable_limit()->set_value(*limit);
    if (cursor) req.set_cursor(*cursor);

    for (auto& id : ownerIds)
    {
        req.add_owner_ids(id);
    }

    auto responseReader = _stub->AsyncListTournamentRecords(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::listTournamentRecordsAroundOwner(
    NSessionPtr session,
    const std::string & tournamentId,
    const std::string & ownerId,
    const opt::optional<int32_t>& limit,
    std::function<void(NTournamentRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::TournamentRecordList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NTournamentRecordListPtr list(new NTournamentRecordList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListTournamentRecordsAroundOwnerRequest req;

    req.set_tournament_id(tournamentId);
    req.set_owner_id(ownerId);

    if (limit) req.mutable_limit()->set_value(*limit);

    auto responseReader = _stub->AsyncListTournamentRecordsAroundOwner(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::joinTournament(NSessionPtr session, const std::string & tournamentId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::JoinTournamentRequest req;

    req.set_tournament_id(tournamentId);

    auto responseReader = _stub->AsyncJoinTournament(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::listStorageObjects(
    NSessionPtr session,
    const std::string & collection,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NStorageObjectListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::StorageObjectList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NStorageObjectListPtr list(new NStorageObjectList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListStorageObjectsRequest req;

    req.set_collection(collection);

    if (limit) req.mutable_limit()->set_value(*limit);
    if (cursor) req.set_cursor(*cursor);

    auto responseReader = _stub->AsyncListStorageObjects(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::listUsersStorageObjects(
    NSessionPtr session,
    const std::string & collection,
    const std::string & userId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NStorageObjectListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::StorageObjectList>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NStorageObjectListPtr list(new NStorageObjectList());
            assign(*list, *data);
            successCallback(list);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ListStorageObjectsRequest req;

    req.set_collection(collection);
    req.set_user_id(userId);

    if (limit) req.mutable_limit()->set_value(*limit);
    if (cursor) req.set_cursor(*cursor);

    auto responseReader = _stub->AsyncListStorageObjects(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::writeStorageObjects(
    NSessionPtr session,
    const std::vector<NStorageObjectWrite>& objects,
    std::function<void(const NStorageObjectAcks&)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::StorageObjectAcks>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NStorageObjectAcks acks;
            assign(acks, data->acks());
            successCallback(acks);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::WriteStorageObjectsRequest req;

    for (auto& obj : objects)
    {
        auto* write_obj = req.mutable_objects()->Add();

        write_obj->set_collection(obj.collection);
        write_obj->set_key(obj.key);
        write_obj->set_value(obj.value);
        write_obj->set_version(obj.version);

        if (obj.permissionRead)
            write_obj->mutable_permission_read()->set_value(static_cast<::google::protobuf::int32>(*obj.permissionRead));

        if (obj.permissionWrite)
            write_obj->mutable_permission_write()->set_value(static_cast<::google::protobuf::int32>(*obj.permissionWrite));
    }

    auto responseReader = _stub->AsyncWriteStorageObjects(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::readStorageObjects(
    NSessionPtr session,
    const std::vector<NReadStorageObjectId>& objectIds,
    std::function<void(const NStorageObjects&)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::StorageObjects>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NStorageObjects objects;
            assign(objects, data->objects());
            successCallback(objects);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::ReadStorageObjectsRequest req;

    for (auto& obj : objectIds)
    {
        auto* write_obj = req.mutable_object_ids()->Add();

        write_obj->set_collection(obj.collection);
        write_obj->set_key(obj.key);
        write_obj->set_user_id(obj.userId);
    }

    auto responseReader = _stub->AsyncReadStorageObjects(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::deleteStorageObjects(NSessionPtr session, const std::vector<NDeleteStorageObjectId>& objectIds, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::DeleteStorageObjectsRequest req;

    for (auto& obj : objectIds)
    {
        auto* write_obj = req.mutable_object_ids()->Add();

        write_obj->set_collection(obj.collection);
        write_obj->set_key(obj.key);
        write_obj->set_version(obj.version);
    }

    auto responseReader = _stub->AsyncDeleteStorageObjects(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void GrpcClient::rpc(
    NSessionPtr session,
    const std::string & id,
    const opt::optional<std::string>& payload,
    std::function<void(const NRpc&)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    setSessionAuth(ctx, session);
    auto data(make_shared<nakama::api::Rpc>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NRpc rpc;
            assign(rpc, *data);
            successCallback(rpc);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::Rpc req;

    req.set_id(id);

    if (payload)
        req.set_payload(*payload);

    auto responseReader = _stub->AsyncRpcFunc(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void GrpcClient::rpc(
    const std::string& http_key,
    const std::string& id,
    const opt::optional<std::string>& payload,
    std::function<void(const NRpc&)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    ReqContext* ctx = createReqContext();
    auto data(make_shared<nakama::api::Rpc>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NRpc rpc;
            assign(rpc, *data);
            successCallback(rpc);
        };
    }
    ctx->errorCallback = errorCallback;

    nakama::api::Rpc req;

    req.set_id(id);
    req.set_http_key(http_key);

    if (payload)
        req.set_payload(*payload);

    auto responseReader = _stub->AsyncRpcFunc(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

} // namespace Nakama

