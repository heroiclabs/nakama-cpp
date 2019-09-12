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

#ifdef BUILD_REST_CLIENT

#include "RestClient.h"
#include "realtime/NRtClient.h"
#include "nakama-cpp/realtime/NWebsocketsFactory.h"
#include "nakama-cpp/log/NLogger.h"
#include "nakama-cpp/StrUtil.h"
#include "nakama-cpp/NakamaVersion.h"
#include "DataHelper.h"
#include "DefaultSession.h"
#include "google/rpc/status.pb.h"
#include "google/protobuf/util/json_util.h"
#include "grpc/include/grpcpp/impl/codegen/status_code_enum.h"
#include "RapidjsonHelper.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#undef NMODULE_NAME
#define NMODULE_NAME "Nakama::RestClient"

using namespace std;

namespace Nakama {

void AddBoolArg(NHttpQueryArgs& args, string&& name, bool value)
{
    value ? args.emplace(name, "true") : args.emplace(name, "false");
}

string jsonDocToStr(rapidjson::Document& document)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}

void addVarsToJsonDoc(rapidjson::Document& document, const NStringMap& vars)
{
    if (!vars.empty())
    {
        rapidjson::Value jsonObj;
        jsonObj.SetObject();

        for (auto& p : vars)
        {
            jsonObj.AddMember(rapidjson::Value::StringRefType(p.first.c_str()), p.second, document.GetAllocator());
        }

        document.AddMember("vars", std::move(jsonObj), document.GetAllocator());
    }
}

RestClient::RestClient(const NClientParameters& parameters, NHttpTransportPtr httpClient)
    : _host(parameters.host)
    , _ssl(parameters.ssl)
    , _httpClient(httpClient)
{
    NLOG(NLogLevel::Info, "Created. NakamaSdkVersion: %s", getNakamaSdkVersion());

    std::string baseUrl;

    int32_t port = parameters.port;

    if (port == DEFAULT_PORT)
    {
        port = parameters.ssl ? 443 : 7350;
        NLOG(NLogLevel::Info, "using default port %d", port);
    }

    _ssl ? baseUrl.append("https") : baseUrl.append("http");
    baseUrl.append("://").append(parameters.host).append(":").append(std::to_string(port));

    _httpClient->setBaseUri(baseUrl);

    _basicAuthMetadata = "Basic " + base64Encode(parameters.serverKey + ":");
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
                    rapidjson::Document document;

                    if (document.Parse(response->body).HasParseError())
                    {
                        errMessage = "Parse JSON failed: " + response->body;
                        code = ErrorCode::InternalError;
                    }
                    else
                    {
                        auto& jsonMessage = document["message"];
                        auto& jsonCode    = document["code"];

                        if (jsonMessage.IsString())
                        {
                            errMessage.append("message: ").append(jsonMessage.GetString());
                        }

                        if (jsonCode.IsNumber())
                        {
                            int serverErrCode = jsonCode.GetInt();

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
    const NStringMap& vars,
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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("id", id, document.GetAllocator());
        addVarsToJsonDoc(document, vars);

        string body = jsonDocToStr(document);

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
    const NStringMap& vars,
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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("email", email, document.GetAllocator());
        document.AddMember("password", password, document.GetAllocator());
        addVarsToJsonDoc(document, vars);

        string body = jsonDocToStr(document);

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
    const NStringMap& vars,
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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("token", accessToken, document.GetAllocator());
        addVarsToJsonDoc(document, vars);

        string body = jsonDocToStr(document);

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
    const NStringMap& vars,
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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("token", accessToken, document.GetAllocator());
        addVarsToJsonDoc(document, vars);

        string body = jsonDocToStr(document);

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
    const NStringMap& vars,
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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("player_id", playerId, document.GetAllocator());
        document.AddMember("bundle_id", bundleId, document.GetAllocator());
        document.AddMember("timestamp_seconds", timestampSeconds, document.GetAllocator());
        document.AddMember("salt", salt, document.GetAllocator());
        document.AddMember("signature", signature, document.GetAllocator());
        document.AddMember("public_key_url", publicKeyUrl, document.GetAllocator());
        addVarsToJsonDoc(document, vars);

        string body = jsonDocToStr(document);

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
    const NStringMap& vars,
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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("id", id, document.GetAllocator());
        addVarsToJsonDoc(document, vars);

        string body = jsonDocToStr(document);

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
    const NStringMap& vars,
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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("token", token, document.GetAllocator());
        addVarsToJsonDoc(document, vars);

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("token", accessToken, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("email", email, document.GetAllocator());
        document.AddMember("password", password, document.GetAllocator());

        string body = jsonDocToStr(document);

        sendReq(ctx, NHttpReqMethod::POST, "/v2/account/link/email", std::move(body));
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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("id", id, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("token", accessToken, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("player_id", playerId, document.GetAllocator());
        document.AddMember("bundle_id", bundleId, document.GetAllocator());
        document.AddMember("timestamp_seconds", timestampSeconds, document.GetAllocator());
        document.AddMember("salt", salt, document.GetAllocator());
        document.AddMember("signature", signature, document.GetAllocator());
        document.AddMember("public_key_url", publicKeyUrl, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("token", token, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("id", id, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("token", accessToken, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("email", email, document.GetAllocator());
        document.AddMember("password", password, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("token", accessToken, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("player_id", playerId, document.GetAllocator());
        document.AddMember("bundle_id", bundleId, document.GetAllocator());
        document.AddMember("timestamp_seconds", timestampSeconds, document.GetAllocator());
        document.AddMember("salt", salt, document.GetAllocator());
        document.AddMember("signature", signature, document.GetAllocator());
        document.AddMember("public_key_url", publicKeyUrl, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("token", token, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("id", id, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("id", id, document.GetAllocator());

        string body = jsonDocToStr(document);

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("token", token, document.GetAllocator());

        string body = jsonDocToStr(document);

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
    ErrorCallback errorCallback)
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

        rapidjson::Document document;
        document.SetObject();

        if (username) document.AddMember("username", *username, document.GetAllocator());
        if (displayName) document.AddMember("display_name", *displayName, document.GetAllocator());
        if (avatarUrl) document.AddMember("avatar_url", *avatarUrl, document.GetAllocator());
        if (langTag) document.AddMember("lang_tag", *langTag, document.GetAllocator());
        if (location) document.AddMember("location", *location, document.GetAllocator());
        if (timezone) document.AddMember("timezone", *timezone, document.GetAllocator());

        string body = jsonDocToStr(document);

        sendReq(ctx, NHttpReqMethod::PUT, "/v2/account", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::getUsers(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    const std::vector<std::string>& facebookIds,
    std::function<void(const NUsers&)> successCallback,
    ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto usersData(make_shared<nakama::api::Users>());
        RestReqContext* ctx = createReqContext(session, usersData.get());

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

        NHttpQueryArgs args;

        for (auto& id : ids)
        {
            args.emplace("ids", id);
        }

        for (auto& username : usernames)
        {
            args.emplace("usernames", username);
        }

        for (auto& facebookId : facebookIds)
        {
            args.emplace("facebook_ids", facebookId);
        }

        sendReq(ctx, NHttpReqMethod::GET, "/v2/user", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::addFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        for (auto& id : ids)
        {
            args.emplace("ids", id);
        }

        for (auto& username : usernames)
        {
            args.emplace("usernames", username);
        }

        sendReq(ctx, NHttpReqMethod::POST, "/v2/friend", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::deleteFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        for (auto& id : ids)
        {
            args.emplace("ids", id);
        }

        for (auto& username : usernames)
        {
            args.emplace("usernames", username);
        }

        sendReq(ctx, NHttpReqMethod::DEL, "/v2/friend", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::blockFriends(
    NSessionPtr session,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& usernames,
    std::function<void()> successCallback,
    ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        for (auto& id : ids)
        {
            args.emplace("ids", id);
        }

        for (auto& username : usernames)
        {
            args.emplace("usernames", username);
        }

        sendReq(ctx, NHttpReqMethod::POST, "/v2/friend/block", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listFriends(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<NFriend::State>& state,
    const std::string& cursor,
    std::function<void(NFriendListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::FriendList>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        NHttpQueryArgs args;

        if (limit) args.emplace("limit", std::to_string(*limit));
        if (state) args.emplace("state", std::to_string((int32_t)*state));
        if (!cursor.empty()) args.emplace("cursor", cursor);

        sendReq(ctx, NHttpReqMethod::GET, "/v2/friend", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::createGroup(
    NSessionPtr session,
    const std::string & name,
    const std::string & description,
    const std::string & avatarUrl,
    const std::string & langTag,
    bool open,
    const opt::optional<int32_t>& maxCount,
    std::function<void(const NGroup&)> successCallback,
    ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto groupData(make_shared<nakama::api::Group>());
        RestReqContext* ctx = createReqContext(session, groupData.get());

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("name", name, document.GetAllocator());
        document.AddMember("description", description, document.GetAllocator());
        document.AddMember("avatar_url", avatarUrl, document.GetAllocator());
        document.AddMember("lang_tag", langTag, document.GetAllocator());
        document.AddMember("open", open, document.GetAllocator());
        if (maxCount) document.AddMember("max_count", *maxCount, document.GetAllocator());

        string body = jsonDocToStr(document);

        sendReq(ctx, NHttpReqMethod::POST, "/v2/group", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::deleteGroup(
    NSessionPtr session,
    const std::string & groupId,
    std::function<void()> successCallback,
    ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        sendReq(ctx, NHttpReqMethod::DEL, "/v2/group/" + groupId, "");
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::addGroupUsers(
    NSessionPtr session,
    const std::string & groupId,
    const std::vector<std::string>& ids,
    std::function<void()> successCallback,
    ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        for (auto& id : ids)
        {
            args.emplace("user_ids", id);
        }

        sendReq(ctx, NHttpReqMethod::POST, "/v2/group/" + groupId + "/add", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listGroupUsers(
    NSessionPtr session,
    const std::string & groupId,
    const opt::optional<int32_t>& limit,
    const opt::optional<NFriend::State>& state,
    const std::string& cursor,
    std::function<void(NGroupUserListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto groupData(make_shared<nakama::api::GroupUserList>());
        RestReqContext* ctx = createReqContext(session, groupData.get());

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

        NHttpQueryArgs args;

        if (limit) args.emplace("limit", std::to_string(*limit));
        if (state) args.emplace("state", std::to_string((int32_t)*state));
        if (!cursor.empty()) args.emplace("cursor", cursor);

        sendReq(ctx, NHttpReqMethod::GET, "/v2/group/" + groupId + "/user", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::kickGroupUsers(NSessionPtr session, const std::string & groupId, const std::vector<std::string>& ids, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        for (auto& id : ids)
        {
            args.emplace("user_ids", id);
        }

        sendReq(ctx, NHttpReqMethod::POST, "/v2/group/" + groupId + "/kick", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::joinGroup(NSessionPtr session, const std::string & groupId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        sendReq(ctx, NHttpReqMethod::POST, "/v2/group/" + groupId + "/join", "");
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::leaveGroup(NSessionPtr session, const std::string & groupId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        sendReq(ctx, NHttpReqMethod::POST, "/v2/group/" + groupId + "/leave", "");
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listGroups(NSessionPtr session, const std::string & name, int32_t limit, const std::string & cursor, std::function<void(NGroupListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto groupData(make_shared<nakama::api::GroupList>());
        RestReqContext* ctx = createReqContext(session, groupData.get());

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

        NHttpQueryArgs args;

        if (!name.empty()) args.emplace("name", name);
        if (!cursor.empty()) args.emplace("cursor", cursor);
        if (limit > 0) args.emplace("limit", std::to_string(limit));

        sendReq(ctx, NHttpReqMethod::GET, "/v2/group", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listUserGroups(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<NFriend::State>& state,
    const std::string& cursor,
    std::function<void(NUserGroupListPtr)> successCallback, ErrorCallback errorCallback)
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

void RestClient::listUserGroups(
    NSessionPtr session,
    const std::string & userId,
    const opt::optional<int32_t>& limit,
    const opt::optional<NFriend::State>& state,
    const std::string& cursor,
    std::function<void(NUserGroupListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto groupData(make_shared<nakama::api::UserGroupList>());
        RestReqContext* ctx = createReqContext(session, groupData.get());

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

        NHttpQueryArgs args;

        if (limit) args.emplace("limit", std::to_string(*limit));
        if (state) args.emplace("state", std::to_string((int32_t)*state));
        if (!cursor.empty()) args.emplace("cursor", cursor);

        sendReq(ctx, NHttpReqMethod::GET, "/v2/user/" + userId + "/group", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::promoteGroupUsers(NSessionPtr session, const std::string & groupId, const std::vector<std::string>& ids, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        NHttpQueryArgs args;

        for (auto& id : ids)
        {
            args.emplace("user_ids", id);
        }

        sendReq(ctx, NHttpReqMethod::POST, "/v2/group/" + groupId + "/promote", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
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
    ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("group_id", groupId, document.GetAllocator());
        if (name) document.AddMember("name", *name, document.GetAllocator());
        if (description) document.AddMember("description", *description, document.GetAllocator());
        if (avatarUrl) document.AddMember("avatar_url", *avatarUrl, document.GetAllocator());
        if (langTag) document.AddMember("lang_tag", *langTag, document.GetAllocator());
        if (open) document.AddMember("open", *open, document.GetAllocator());

        string body = jsonDocToStr(document);

        sendReq(ctx, NHttpReqMethod::PUT, "/v2/group/" + groupId, std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listLeaderboardRecords(
    NSessionPtr session,
    const std::string & leaderboardId,
    const std::vector<std::string>& ownerIds,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NLeaderboardRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::LeaderboardRecordList>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        NHttpQueryArgs args;

        for (auto& id : ownerIds)
        {
            args.emplace("owner_ids", id);
        }

        if (limit) args.emplace("limit", std::to_string(*limit));
        if (cursor) args.emplace("cursor", *cursor);

        sendReq(ctx, NHttpReqMethod::GET, "/v2/leaderboard/" + leaderboardId, "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listLeaderboardRecordsAroundOwner(
    NSessionPtr session,
    const std::string & leaderboardId,
    const std::string & ownerId,
    const opt::optional<int32_t>& limit,
    std::function<void(NLeaderboardRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::LeaderboardRecordList>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        NHttpQueryArgs args;

        if (limit) args.emplace("limit", std::to_string(*limit));

        sendReq(ctx, NHttpReqMethod::GET, "/v2/leaderboard/" + leaderboardId + "/owner/" + ownerId, "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::writeLeaderboardRecord(
    NSessionPtr session,
    const std::string & leaderboardId,
    int64_t score,
    const opt::optional<int64_t>& subscore,
    const opt::optional<std::string>& metadata,
    std::function<void(NLeaderboardRecord)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::LeaderboardRecord>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("score", std::to_string(score), document.GetAllocator());
        if (subscore) document.AddMember("subscore", std::to_string(*subscore), document.GetAllocator());
        if (metadata) document.AddMember("metadata", *metadata, document.GetAllocator());

        string body = jsonDocToStr(document);

        sendReq(ctx, NHttpReqMethod::POST, "/v2/leaderboard/" + leaderboardId, std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::writeTournamentRecord(
    NSessionPtr session,
    const std::string & tournamentId,
    int64_t score,
    const opt::optional<int64_t>& subscore,
    const opt::optional<std::string>& metadata,
    std::function<void(NLeaderboardRecord)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::LeaderboardRecord>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("score", std::to_string(score), document.GetAllocator());
        if (subscore) document.AddMember("subscore", std::to_string(*subscore), document.GetAllocator());
        if (metadata) document.AddMember("metadata", *metadata, document.GetAllocator());

        string body = jsonDocToStr(document);

        sendReq(ctx, NHttpReqMethod::PUT, "/v2/tournament/" + tournamentId, std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::deleteLeaderboardRecord(NSessionPtr session, const std::string & leaderboardId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        sendReq(ctx, NHttpReqMethod::DEL, "/v2/leaderboard/" + leaderboardId, "");
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
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
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::MatchList>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        NHttpQueryArgs args;

        if (min_size) args.emplace("min_size", std::to_string(*min_size));
        if (max_size) args.emplace("max_size", std::to_string(*max_size));
        if (limit) args.emplace("limit", std::to_string(*limit));
        if (label) args.emplace("label", *label);
        if (authoritative) AddBoolArg(args, "authoritative", *authoritative);

        sendReq(ctx, NHttpReqMethod::GET, "/v2/match", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listNotifications(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cacheableCursor,
    std::function<void(NNotificationListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
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
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::deleteNotifications(NSessionPtr session, const std::vector<std::string>& notificationIds, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
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
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listChannelMessages(
    NSessionPtr session,
    const std::string & channelId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const opt::optional<bool>& forward,
    std::function<void(NChannelMessageListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::ChannelMessageList>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        NHttpQueryArgs args;

        if (limit) args.emplace("limit", std::to_string(*limit));
        if (cursor) args.emplace("cursor", *cursor);
        if (forward) AddBoolArg(args, "forward", *forward);

        sendReq(ctx, NHttpReqMethod::GET, "/v2/channel/" + channelId, "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
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
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::TournamentList>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        NHttpQueryArgs args;

        if (categoryStart) args.emplace("category_start", std::to_string(*categoryStart));
        if (categoryEnd) args.emplace("category_end", std::to_string(*categoryEnd));
        if (startTime) args.emplace("start_time", std::to_string(*startTime));
        if (endTime) args.emplace("end_time", std::to_string(*endTime));
        if (limit) args.emplace("limit", std::to_string(*limit));
        if (cursor) args.emplace("cursor", *cursor);

        sendReq(ctx, NHttpReqMethod::GET, "/v2/tournament", "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listTournamentRecords(
    NSessionPtr session,
    const std::string & tournamentId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    const std::vector<std::string>& ownerIds,
    std::function<void(NTournamentRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::TournamentRecordList>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        NHttpQueryArgs args;

        if (limit) args.emplace("limit", std::to_string(*limit));
        if (cursor) args.emplace("cursor", *cursor);

        for (auto& id : ownerIds)
        {
            args.emplace("owner_ids", id);
        }

        sendReq(ctx, NHttpReqMethod::GET, "/v2/tournament/" + tournamentId, "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listTournamentRecordsAroundOwner(
    NSessionPtr session,
    const std::string & tournamentId,
    const std::string & ownerId,
    const opt::optional<int32_t>& limit,
    std::function<void(NTournamentRecordListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::TournamentRecordList>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        NHttpQueryArgs args;

        if (limit) args.emplace("limit", std::to_string(*limit));

        sendReq(ctx, NHttpReqMethod::GET, "/v2/tournament/" + tournamentId + "/owner/" + ownerId, "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::joinTournament(NSessionPtr session, const std::string & tournamentId, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        sendReq(ctx, NHttpReqMethod::POST, "/v2/tournament/" + tournamentId + "/join", "");
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listStorageObjects(
    NSessionPtr session,
    const std::string & collection,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NStorageObjectListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::StorageObjectList>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        NHttpQueryArgs args;

        if (limit) args.emplace("limit", std::to_string(*limit));
        if (cursor) args.emplace("cursor", *cursor);

        sendReq(ctx, NHttpReqMethod::GET, "/v2/storage/" + collection, "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::listUsersStorageObjects(
    NSessionPtr session,
    const std::string & collection,
    const std::string & userId,
    const opt::optional<int32_t>& limit,
    const opt::optional<std::string>& cursor,
    std::function<void(NStorageObjectListPtr)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::StorageObjectList>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        NHttpQueryArgs args;

        args.emplace("user_id", userId);
        if (limit) args.emplace("limit", std::to_string(*limit));
        if (cursor) args.emplace("cursor", *cursor);

        sendReq(ctx, NHttpReqMethod::GET, "/v2/storage/" + collection, "", std::move(args));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::writeStorageObjects(
    NSessionPtr session,
    const std::vector<NStorageObjectWrite>& objects,
    std::function<void(const NStorageObjectAcks&)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::StorageObjectAcks>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Value jsonObjects;
        jsonObjects.SetArray();

        for (auto& obj : objects)
        {
            rapidjson::Value jsonObj;
            jsonObj.SetObject();

            jsonObj.AddMember("collection", obj.collection, document.GetAllocator());
            jsonObj.AddMember("key", obj.key, document.GetAllocator());
            jsonObj.AddMember("value", obj.value, document.GetAllocator());
            jsonObj.AddMember("version", obj.version, document.GetAllocator());

            if (obj.permissionRead)
                jsonObj.AddMember("permission_read", static_cast<int32_t>(*obj.permissionRead), document.GetAllocator());

            if (obj.permissionWrite)
                jsonObj.AddMember("permission_write", static_cast<int32_t>(*obj.permissionWrite), document.GetAllocator());

            jsonObjects.PushBack(std::move(jsonObj), document.GetAllocator());
        }

        document.AddMember("objects", std::move(jsonObjects), document.GetAllocator());

        string body = jsonDocToStr(document);

        sendReq(ctx, NHttpReqMethod::PUT, "/v2/storage", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::readStorageObjects(
    NSessionPtr session,
    const std::vector<NReadStorageObjectId>& objectIds,
    std::function<void(const NStorageObjects&)> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        auto data(make_shared<nakama::api::StorageObjects>());
        RestReqContext* ctx = createReqContext(session, data.get());

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

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Value jsonObjects;
        jsonObjects.SetArray();

        for (auto& obj : objectIds)
        {
            rapidjson::Value jsonObj;
            jsonObj.SetObject();

            jsonObj.AddMember("collection", obj.collection, document.GetAllocator());
            jsonObj.AddMember("key", obj.key, document.GetAllocator());
            jsonObj.AddMember("user_id", obj.userId, document.GetAllocator());

            jsonObjects.PushBack(std::move(jsonObj), document.GetAllocator());
        }

        document.AddMember("object_ids", std::move(jsonObjects), document.GetAllocator());

        string body = jsonDocToStr(document);

        sendReq(ctx, NHttpReqMethod::POST, "/v2/storage", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::deleteStorageObjects(NSessionPtr session, const std::vector<NDeleteStorageObjectId>& objectIds, std::function<void()> successCallback, ErrorCallback errorCallback)
{
    try {
        NLOG_INFO("...");

        RestReqContext* ctx = createReqContext(session, nullptr);

        ctx->successCallback = successCallback;
        ctx->errorCallback = errorCallback;

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Value jsonObjects;
        jsonObjects.SetArray();

        for (auto& obj : objectIds)
        {
            rapidjson::Value jsonObj;
            jsonObj.SetObject();

            jsonObj.AddMember("collection", obj.collection, document.GetAllocator());
            jsonObj.AddMember("key", obj.key, document.GetAllocator());
            jsonObj.AddMember("version", obj.version, document.GetAllocator());

            jsonObjects.PushBack(std::move(jsonObj), document.GetAllocator());
        }

        document.AddMember("object_ids", std::move(jsonObjects), document.GetAllocator());

        string body = jsonDocToStr(document);

        sendReq(ctx, NHttpReqMethod::PUT, "/v2/storage/delete", std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

void RestClient::rpc(
    NSessionPtr session,
    const std::string & id,
    const opt::optional<std::string>& payload,
    std::function<void(const NRpc&)> successCallback, ErrorCallback errorCallback)
{
    try {
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

        string body;
        string path("/v2/rpc/");

        path.append(id);

        if (payload)
            body = *payload;

        sendReq(ctx, NHttpReqMethod::POST, std::move(path), std::move(body));
    }
    catch (exception& e)
    {
        NLOG_ERROR("exception: " + string(e.what()));
    }
}

}

#endif // BUILD_REST_CLIENT
