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

#include "RestClient.h"
#include "realtime/NRtClient.h"
#include "nakama-cpp/realtime/NWebsockets.h"
#include "nakama-cpp/log/NLogger.h"
#include "nakama-cpp/StrUtil.h"
#include "nakama-cpp/NakamaVersion.h"
#include "DataHelper.h"
#include "DefaultSession.h"
#include "google/rpc/status.pb.h"
#include "grpc/include/grpcpp/impl/codegen/status_code_enum.h"
#include "CppRestUtils.h"
#include "cpprest/json.h"

#undef NMODULE_NAME
#define NMODULE_NAME "Nakama::RestClient"

#include "google/protobuf/util/json_util.h"

using namespace std;

namespace Nakama {

void AddBoolArg(NHttpQueryArgs& args, string&& name, bool value)
{
    value ? args.emplace(name, "true") : args.emplace(name, "false");
}

RestClient::RestClient(const DefaultClientParameters& parameters, NHttpClientPtr httpClient)
    : _host(parameters.host)
    , _ssl(parameters.ssl)
    , _httpClient(httpClient)
{
    std::string baseUrl;

    _ssl ? baseUrl.append("https") : baseUrl.append("http");
    baseUrl.append("://").append(parameters.host).append(":").append(std::to_string(parameters.port));

    _httpClient->setBaseUri(baseUrl);

    _basicAuthMetadata = "Basic " + base64Encode(parameters.serverKey + ":");
    
    NLOG(NLogLevel::Info, "Created. NakamaSdkVersion: %s", getNakamaSdkVersion());
}

RestClient::~RestClient()
{
    disconnect();

    if (_reqContexts.size() > 0)
    {
        NLOG(NLogLevel::Warn, "Not handled %u request(s) detected.", _reqContexts.size());

        for (RestReqContext* reqContext : _reqContexts)
        {
            delete reqContext;
        }

        _reqContexts.clear();
    }
}

void RestClient::disconnect()
{
}

void RestClient::tick()
{
    _httpClient->tick();
}

NRtClientPtr RestClient::createRtClient(int32_t port, NRtTransportPtr transport)
{
    RtClientParameters parameters;
    
    parameters.host = _host;
    parameters.port = port;
    parameters.ssl  = _ssl;
    
    return createRtClient(parameters, transport);
}

NRtClientPtr RestClient::createRtClient(const RtClientParameters& parameters, NRtTransportPtr transport)
{
    if (!transport)
    {
        transport = createDefaultWebsocket();

        if (!transport)
        {
            NLOG_ERROR("No default websockets transport available. Please set transport.");
            return nullptr;
        }
    }

    NRtClientPtr client(new NRtClient(transport, parameters.host, parameters.port, parameters.ssl));
    return client;
}

RestReqContext * RestClient::createReqContext(NSessionPtr session, google::protobuf::Message* data)
{
    RestReqContext* ctx = new RestReqContext();

    if (session)
    {
        ctx->auth.append("Bearer ").append(session->getAuthToken());
    }
    else
    {
        ctx->auth.append(_basicAuthMetadata);
    }

    ctx->data = data;

    _reqContexts.emplace(ctx);
    return ctx;
}

void RestClient::sendReq(
    RestReqContext* ctx,
    NHttpReqMethod method,
    std::string&& path,
    std::string&& body,
    NHttpQueryArgs&& args)
{
    NHttpRequest req;

    req.method    = method;
    req.path      = std::move(path);
    req.body      = std::move(body);
    req.queryArgs = std::move(args);

    req.headers.emplace("Accept", "application/json");
    req.headers.emplace("Content-Type", "application/json");
    req.headers.emplace("Authorization", std::move(ctx->auth));

    _httpClient->request(req, [this, ctx](NHttpResponsePtr response)
    {
        onResponse(ctx, response);
    });
}

void RestClient::onResponse(RestReqContext* reqContext, NHttpResponsePtr response)
{
    auto it = _reqContexts.find(reqContext);

    if (it != _reqContexts.end())
    {
        if (response->statusCode == 200) // OK
        {
            if (reqContext->successCallback)
            {
                bool ok = true;

                if (reqContext->data)
                {
                    auto status = google::protobuf::util::JsonStringToMessage(response->body, reqContext->data);
                    ok = status.ok();

                    if (!ok)
                    {
                        reqError(reqContext, NError("Parse JSON failed. HTTP body: " + response->body, ErrorCode::InternalError));
                    }
                }

                if (ok)
                {
                    reqContext->successCallback();
                }
            }
        }
        else
        {
            std::string errMessage;
            ErrorCode code = ErrorCode::Unknown;

            if (response->statusCode == InternalStatusCodes::CONNECTION_ERROR)
            {
                code = ErrorCode::ConnectionError;
                errMessage.append("message: ").append(response->errorMessage);
            }
            else if (!response->body.empty() && response->body[0] == '{') // have to be JSON
            {
                try {
                    utility::stringstream_t ss;
                    ss << FROM_STD_STR(response->body);
                    web::json::value jsonRoot = web::json::value::parse(ss);
                    web::json::value jsonMessage = jsonRoot.at(FROM_STD_STR("message"));
                    web::json::value jsonCode = jsonRoot.at(FROM_STD_STR("code"));

                    if (jsonMessage.is_string())
                    {
                        errMessage.append("message: ").append(TO_STD_STR(jsonMessage.as_string()));
                    }

                    if (jsonCode.is_integer())
                    {
                        int serverErrCode = jsonCode.as_integer();

                        switch (serverErrCode)
                        {
                        case grpc::StatusCode::UNAVAILABLE      : code = ErrorCode::ConnectionError; break;
                        case grpc::StatusCode::INTERNAL         : code = ErrorCode::InternalError; break;
                        case grpc::StatusCode::NOT_FOUND        : code = ErrorCode::NotFound; break;
                        case grpc::StatusCode::ALREADY_EXISTS   : code = ErrorCode::AlreadyExists; break;
                        case grpc::StatusCode::INVALID_ARGUMENT : code = ErrorCode::InvalidArgument; break;
                        case grpc::StatusCode::UNAUTHENTICATED  : code = ErrorCode::Unauthenticated; break;
                        case grpc::StatusCode::PERMISSION_DENIED: code = ErrorCode::PermissionDenied; break;

                        default:
                            errMessage.append("\ncode: ").append(std::to_string(serverErrCode));
                            break;
                        }
                    }
                }
                catch (exception& e)
                {
                    NLOG_ERROR("exception: " + string(e.what()));
                }
            }

            if (errMessage.empty())
            {
                errMessage.append("message: ").append(response->errorMessage);
                errMessage.append("\nHTTP status: ").append(std::to_string(response->statusCode));
                errMessage.append("\nbody: ").append(response->body);
            }

            reqError(reqContext, NError(std::move(errMessage), code));
        }

        delete reqContext;
        _reqContexts.erase(it);
    }
    else
    {
        reqError(nullptr, NError("Not found request context.", ErrorCode::InternalError));
    }
}

void RestClient::reqError(RestReqContext * reqContext, const NError & error)
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

void RestClient::authenticateDevice(
    const std::string& id,
    const opt::optional<std::string>& username,
    const opt::optional<bool>& create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    try {
        NLOG_INFO("...");

        auto sessionData(make_shared<nakama::api::Session>());
        RestReqContext* ctx = createReqContext(nullptr, sessionData.get());

        if (successCallback)
        {
            ctx->successCallback = [sessionData, successCallback]()
            {
                NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
                successCallback(session);
            };
        }
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        if (username)
            args.emplace("username", *username);

        if (create)
        {
            AddBoolArg(args, "create", *create);
        }

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("id")] = web::json::value(FROM_STD_STR(id));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/authenticate/device", std::move(body), std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::authenticateEmail(
    const std::string & email,
    const std::string & password,
    const std::string & username,
    bool create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    try {
        NLOG_INFO("...");

        auto sessionData(make_shared<nakama::api::Session>());
        RestReqContext* ctx = createReqContext(nullptr, sessionData.get());

        if (successCallback)
        {
            ctx->successCallback = [sessionData, successCallback]()
            {
                NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
                successCallback(session);
            };
        }
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        args.emplace("username", username);
        AddBoolArg(args, "create", create);

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("email")] = web::json::value(FROM_STD_STR(email));
        jsonRoot[FROM_STD_STR("password")] = web::json::value(FROM_STD_STR(password));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/authenticate/email", std::move(body), std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::authenticateFacebook(
    const std::string & accessToken,
    const std::string & username,
    bool create,
    bool importFriends,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    try {
        NLOG_INFO("...");

        auto sessionData(make_shared<nakama::api::Session>());
        RestReqContext* ctx = createReqContext(nullptr, sessionData.get());

        if (successCallback)
        {
            ctx->successCallback = [sessionData, successCallback]()
            {
                NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
                successCallback(session);
            };
        }
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        args.emplace("username", username);
        AddBoolArg(args, "create", create);
        AddBoolArg(args, "import", importFriends);

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("token")] = web::json::value(FROM_STD_STR(accessToken));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/authenticate/facebook", std::move(body), std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::authenticateGoogle(
    const std::string & accessToken,
    const std::string & username,
    bool create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    try {
        NLOG_INFO("...");

        auto sessionData(make_shared<nakama::api::Session>());
        RestReqContext* ctx = createReqContext(nullptr, sessionData.get());

        if (successCallback)
        {
            ctx->successCallback = [sessionData, successCallback]()
            {
                NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
                successCallback(session);
            };
        }
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        args.emplace("username", username);
        AddBoolArg(args, "create", create);

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("token")] = web::json::value(FROM_STD_STR(accessToken));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/authenticate/google", std::move(body), std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::authenticateGameCenter(
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
    try {
        NLOG_INFO("...");

        auto sessionData(make_shared<nakama::api::Session>());
        RestReqContext* ctx = createReqContext(nullptr, sessionData.get());

        if (successCallback)
        {
            ctx->successCallback = [sessionData, successCallback]()
            {
                NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
                successCallback(session);
            };
        }
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        args.emplace("username", username);
        AddBoolArg(args, "create", create);

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("player_id")] = web::json::value(FROM_STD_STR(playerId));
        jsonRoot[FROM_STD_STR("bundle_id")] = web::json::value(FROM_STD_STR(bundleId));
        jsonRoot[FROM_STD_STR("timestamp_seconds")] = web::json::value(timestampSeconds);
        jsonRoot[FROM_STD_STR("salt")] = web::json::value(FROM_STD_STR(salt));
        jsonRoot[FROM_STD_STR("signature")] = web::json::value(FROM_STD_STR(signature));
        jsonRoot[FROM_STD_STR("public_key_url")] = web::json::value(FROM_STD_STR(publicKeyUrl));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/authenticate/gamecenter", std::move(body), std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::authenticateCustom(
    const std::string & id,
    const std::string & username,
    bool create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    try {
        NLOG_INFO("...");

        auto sessionData(make_shared<nakama::api::Session>());
        RestReqContext* ctx = createReqContext(nullptr, sessionData.get());

        if (successCallback)
        {
            ctx->successCallback = [sessionData, successCallback]()
            {
                NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
                successCallback(session);
            };
        }
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        args.emplace("username", username);
        AddBoolArg(args, "create", create);

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("id")] = web::json::value(FROM_STD_STR(id));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/authenticate/custom", std::move(body), std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::authenticateSteam(
    const std::string & token,
    const std::string & username,
    bool create,
    std::function<void(NSessionPtr)> successCallback,
    ErrorCallback errorCallback
)
{
    try {
        NLOG_INFO("...");

        auto sessionData(make_shared<nakama::api::Session>());
        RestReqContext* ctx = createReqContext(nullptr, sessionData.get());

        if (successCallback)
        {
            ctx->successCallback = [sessionData, successCallback]()
            {
                NSessionPtr session(new DefaultSession(sessionData->token(), sessionData->created()));
                successCallback(session);
            };
        }
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        args.emplace("username", username);
        AddBoolArg(args, "create", create);

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("token")] = web::json::value(FROM_STD_STR(token));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/authenticate/steam", std::move(body), std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::linkFacebook(
    NSessionPtr session,
    const std::string & accessToken,
    const opt::optional<bool>& importFriends,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        if (importFriends) AddBoolArg(args, "import", *importFriends);

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("token")] = web::json::value(FROM_STD_STR(accessToken));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/link/facebook", std::move(body), std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::linkEmail(
    NSessionPtr session,
    const std::string & email,
    const std::string & password,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("email")] = web::json::value(FROM_STD_STR(email));
        jsonRoot[FROM_STD_STR("password")] = web::json::value(FROM_STD_STR(password));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/link/email", std::move(body), std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::linkDevice(
    NSessionPtr session,
    const std::string & id,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("id")] = web::json::value(FROM_STD_STR(id));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/link/device", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::linkGoogle(
    NSessionPtr session,
    const std::string & accessToken,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("token")] = web::json::value(FROM_STD_STR(accessToken));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/link/google", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::linkGameCenter(
    NSessionPtr session,
    const std::string & playerId,
    const std::string & bundleId,
    NTimestamp timestampSeconds,
    const std::string & salt,
    const std::string & signature,
    const std::string & publicKeyUrl,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("player_id")] = web::json::value(FROM_STD_STR(playerId));
        jsonRoot[FROM_STD_STR("bundle_id")] = web::json::value(FROM_STD_STR(bundleId));
        jsonRoot[FROM_STD_STR("timestamp_seconds")] = web::json::value(timestampSeconds);
        jsonRoot[FROM_STD_STR("salt")] = web::json::value(FROM_STD_STR(salt));
        jsonRoot[FROM_STD_STR("signature")] = web::json::value(FROM_STD_STR(signature));
        jsonRoot[FROM_STD_STR("public_key_url")] = web::json::value(FROM_STD_STR(publicKeyUrl));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/link/gamecenter", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::linkSteam(
    NSessionPtr session,
    const std::string & token,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("token")] = web::json::value(FROM_STD_STR(token));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/link/steam", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::linkCustom(NSessionPtr session, const std::string & id, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("id")] = web::json::value(FROM_STD_STR(id));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/link/custom", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::unlinkFacebook(NSessionPtr session, const std::string & accessToken, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("token")] = web::json::value(FROM_STD_STR(accessToken));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/unlink/facebook", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::unlinkEmail(NSessionPtr session, const std::string & email, const std::string & password, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("email")] = web::json::value(FROM_STD_STR(email));
        jsonRoot[FROM_STD_STR("password")] = web::json::value(FROM_STD_STR(password));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/unlink/email", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::unlinkGoogle(NSessionPtr session, const std::string & accessToken, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("token")] = web::json::value(FROM_STD_STR(accessToken));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/unlink/google", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::unlinkGameCenter(NSessionPtr session,
    const std::string & playerId,
    const std::string & bundleId,
    NTimestamp timestampSeconds,
    const std::string & salt,
    const std::string & signature,
    const std::string & publicKeyUrl,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("player_id")] = web::json::value(FROM_STD_STR(playerId));
        jsonRoot[FROM_STD_STR("bundle_id")] = web::json::value(FROM_STD_STR(bundleId));
        jsonRoot[FROM_STD_STR("timestamp_seconds")] = web::json::value(timestampSeconds);
        jsonRoot[FROM_STD_STR("salt")] = web::json::value(FROM_STD_STR(salt));
        jsonRoot[FROM_STD_STR("signature")] = web::json::value(FROM_STD_STR(signature));
        jsonRoot[FROM_STD_STR("public_key_url")] = web::json::value(FROM_STD_STR(publicKeyUrl));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/unlink/gamecenter", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::unlinkSteam(NSessionPtr session, const std::string & token, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("token")] = web::json::value(FROM_STD_STR(token));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/unlink/steam", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::unlinkDevice(NSessionPtr session, const std::string & id, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("id")] = web::json::value(FROM_STD_STR(id));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/unlink/device", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::unlinkCustom(NSessionPtr session, const std::string & id, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("id")] = web::json::value(FROM_STD_STR(id));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/unlink/custom", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::importFacebookFriends(
    NSessionPtr session,
    const std::string& token,
    const opt::optional<bool>& reset,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        if (reset) AddBoolArg(args, "reset", *reset);

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        jsonRoot[FROM_STD_STR("token")] = web::json::value(FROM_STD_STR(token));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::POST, "/v2/friend/facebook", std::move(body), std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::getAccount(
    NSessionPtr session,
    std::function<void(const NAccount&)> successCallback,
    ErrorCallback errorCallback
)
{
    try {
        NLOG_INFO("...");

        auto accoutData(make_shared<nakama::api::Account>());
        RestReqContext* ctx = createReqContext(session, accoutData.get());

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

        sendReq(ctx, NHttpReqMethod::GET, "/v2/account", "");
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::updateAccount(
    NSessionPtr session,
    const opt::optional<std::string>& username,
    const opt::optional<std::string>& displayName,
    const opt::optional<std::string>& avatarUrl,
    const opt::optional<std::string>& langTag,
    const opt::optional<std::string>& location,
    const opt::optional<std::string>& timezone,
    std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        utility::stringstream_t ss;
        web::json::value jsonRoot = web::json::value::object();

        if (username) jsonRoot[FROM_STD_STR("username")] = web::json::value(FROM_STD_STR(*username));
        if (displayName) jsonRoot[FROM_STD_STR("display_name")] = web::json::value(FROM_STD_STR(*displayName));
        if (avatarUrl) jsonRoot[FROM_STD_STR("avatar_url")] = web::json::value(FROM_STD_STR(*avatarUrl));
        if (langTag) jsonRoot[FROM_STD_STR("lang_tag")] = web::json::value(FROM_STD_STR(*langTag));
        if (location) jsonRoot[FROM_STD_STR("location")] = web::json::value(FROM_STD_STR(*location));
        if (timezone) jsonRoot[FROM_STD_STR("timezone")] = web::json::value(FROM_STD_STR(*timezone));

        jsonRoot.serialize(ss);

        string body = TO_STD_STR(ss.str());

        sendReq(ctx, NHttpReqMethod::PUT, "/v2/account", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}
/*
void RestClient::getUsers(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    const std::vector<std::string>& facebookIds,
    std::function<void(const NUsers&)> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::addFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

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

void RestClient::deleteFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

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

void RestClient::blockFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

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

void RestClient::listFriends(NSessionPtr session, std::function<void(NFriendsPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
    auto data(make_shared<nakama::api::Friends>());

    if (successCallback)
    {
        ctx->successCallback = [data, successCallback]()
        {
            NFriendsPtr friends(new NFriends());
            assign(*friends, *data);
            successCallback(friends);
        };
    }
    ctx->errorCallback = errorCallback;

    auto responseReader = _stub->AsyncListFriends(&ctx->context, {}, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}

void RestClient::createGroup(
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
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

    if (!description.empty())
        req.set_description(description);

    if (!avatarUrl.empty())
        req.set_avatar_url(avatarUrl);

    if (!langTag.empty())
        req.set_lang_tag(langTag);

    req.set_open(open);

    auto responseReader = _stub->AsyncCreateGroup(&ctx->context, req, &_cq);

    responseReader->Finish(&(*groupData), &ctx->status, (void*)ctx);
}

void RestClient::deleteGroup(
    NSessionPtr session,
    const std::string & groupId,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::DeleteGroupRequest req;

    req.set_group_id(groupId);

    auto responseReader = _stub->AsyncDeleteGroup(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void RestClient::addGroupUsers(
    NSessionPtr session,
    const std::string & groupId,
    const std::vector<std::string>& ids,
    std::function<void()> successCallback,
    ErrorCallback errorCallback
)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

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

void RestClient::listGroupUsers(NSessionPtr session, const std::string & groupId, std::function<void(NGroupUserListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

    auto responseReader = _stub->AsyncListGroupUsers(&ctx->context, req, &_cq);

    responseReader->Finish(&(*groupData), &ctx->status, (void*)ctx);
}

void RestClient::kickGroupUsers(NSessionPtr session, const std::string & groupId, const std::vector<std::string>& ids, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

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

void RestClient::joinGroup(NSessionPtr session, const std::string & groupId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::JoinGroupRequest req;

    req.set_group_id(groupId);

    auto responseReader = _stub->AsyncJoinGroup(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void RestClient::leaveGroup(NSessionPtr session, const std::string & groupId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::LeaveGroupRequest req;

    req.set_group_id(groupId);

    auto responseReader = _stub->AsyncLeaveGroup(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void RestClient::listGroups(NSessionPtr session, const std::string & name, int32_t limit, const std::string & cursor, std::function<void(NGroupListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::listUserGroups(NSessionPtr session, std::function<void(NUserGroupListPtr)> successCallback, ErrorCallback errorCallback)
{
    if (session)
    {
        listUserGroups(session, session->getUserId(), successCallback, errorCallback);
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

void RestClient::listUserGroups(NSessionPtr session, const std::string & userId, std::function<void(NUserGroupListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

    auto responseReader = _stub->AsyncListUserGroups(&ctx->context, req, &_cq);

    responseReader->Finish(&(*groupData), &ctx->status, (void*)ctx);
}

void RestClient::promoteGroupUsers(NSessionPtr session, const std::string & groupId, const std::vector<std::string>& ids, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

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

void RestClient::updateGroup(
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

    RestReqContext* ctx = createReqContext(session);

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

void RestClient::listLeaderboardRecords(
    NSessionPtr session,
    const std::string & leaderboardId,
    const std::vector<std::string>& ownerIds,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NLeaderboardRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::listLeaderboardRecordsAroundOwner(NSessionPtr session, const std::string & leaderboardId, const std::string & ownerId, const opt::optional<int32_t>& limit, std::function<void(NLeaderboardRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::writeLeaderboardRecord(
    NSessionPtr session,
    const std::string & leaderboardId,
    int64_t score,
    const opt::optional<int64_t>& subscore,
    const opt::optional<std::string>& metadata,
    std::function<void(NLeaderboardRecord)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::writeTournamentRecord(
    NSessionPtr session,
    const std::string & tournamentId,
    int64_t score,
    const opt::optional<int64_t>& subscore,
    const opt::optional<std::string>& metadata,
    std::function<void(NLeaderboardRecord)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::deleteLeaderboardRecord(NSessionPtr session, const std::string & leaderboardId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::DeleteLeaderboardRecordRequest req;

    req.set_leaderboard_id(leaderboardId);

    auto responseReader = _stub->AsyncDeleteLeaderboardRecord(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void RestClient::listMatches(
    NSessionPtr session,
    const opt::optional<int32_t>& min_size,
    const opt::optional<int32_t>& max_size,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& label,
    const opt::optional<bool>& authoritative,
    std::function<void(NMatchListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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
    if (authoritative) req.mutable_authoritative()->set_value(*authoritative);

    auto responseReader = _stub->AsyncListMatches(&ctx->context, req, &_cq);

    responseReader->Finish(&(*data), &ctx->status, (void*)ctx);
}
*/
void RestClient::listNotifications(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cacheableCursor,
    std::function<void(NNotificationListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    auto data(make_shared<nakama::api::NotificationList>());
    RestReqContext* ctx = createReqContext(session, data.get());

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

    NHttpQueryArgs args;

    if (limit) args.emplace("limit", std::to_string(*limit));
    if (cacheableCursor) args.emplace("cursor", *cacheableCursor);

    sendReq(ctx, NHttpReqMethod::GET, "/v2/notification", "", std::move(args));
}

void RestClient::deleteNotifications(NSessionPtr session, const std::vector<std::string>& notificationIds, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session, nullptr);

    if (successCallback)
    {
        ctx->successCallback = [successCallback]()
        {
            successCallback();
        };
    }
    ctx->errorCallback = errorCallback;

    NHttpQueryArgs args;

    for (auto& id : notificationIds)
    {
        args.emplace("ids", id);
    }

    sendReq(ctx, NHttpReqMethod::DEL, "/v2/notification", "", std::move(args));
}
/*
void RestClient::listChannelMessages(
    NSessionPtr session,
    const std::string & channelId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const opt::optional<bool>& forward,
    std::function<void(NChannelMessageListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::listTournaments(
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

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::listTournamentRecords(
    NSessionPtr session,
    const std::string & tournamentId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const std::vector<std::string>& ownerIds,
    std::function<void(NTournamentRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::listTournamentRecordsAroundOwner(
    NSessionPtr session,
    const std::string & tournamentId,
    const std::string & ownerId,
    const opt::optional<int32_t>& limit,
    std::function<void(NTournamentRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::joinTournament(NSessionPtr session, const std::string & tournamentId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

    ctx->successCallback = successCallback;
    ctx->errorCallback = errorCallback;

    nakama::api::JoinTournamentRequest req;

    req.set_tournament_id(tournamentId);

    auto responseReader = _stub->AsyncJoinTournament(&ctx->context, req, &_cq);

    responseReader->Finish(&_emptyData, &ctx->status, (void*)ctx);
}

void RestClient::listStorageObjects(
    NSessionPtr session,
    const std::string & collection,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NStorageObjectListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::listUsersStorageObjects(
    NSessionPtr session,
    const std::string & collection,
    const std::string & userId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NStorageObjectListPtr)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::writeStorageObjects(
    NSessionPtr session,
    const std::vector<NStorageObjectWrite>& objects,
    std::function<void(const NStorageObjectAcks&)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::readStorageObjects(
    NSessionPtr session,
    const std::vector<NReadStorageObjectId>& objectIds,
    std::function<void(const NStorageObjects&)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);
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

void RestClient::deleteStorageObjects(NSessionPtr session, const std::vector<NDeleteStorageObjectId>& objectIds, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    RestReqContext* ctx = createReqContext(session);

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
*/
void RestClient::rpc(
    NSessionPtr session,
    const std::string & id,
    const opt::optional<std::string>& payload,
    std::function<void(const NRpc&)> successCallback, ErrorCallback errorCallback)
{
    NLOG_INFO("...");

    auto data(make_shared<nakama::api::Rpc>());
    RestReqContext* ctx = createReqContext(session, data.get());

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

    NHttpQueryArgs args;
    string body;
    string path("/v2/rpc/");

    path.append(id);

    if (payload)
        body = *payload;

    sendReq(ctx, NHttpReqMethod::POST, std::move(path), std::move(body), std::move(args));
}

}
