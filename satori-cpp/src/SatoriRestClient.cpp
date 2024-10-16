/*
* Copyright 2024 The Nakama Authors
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

#include <iostream>
#include <string>
#include <memory>

#include "SatoriRestClient.h"
#include "DataHelper.h"
#include "nakama-cpp/NakamaVersion.h"
#include "nakama-cpp/log/NLogger.h"
#include "StrUtil.h"
#include "RapidjsonHelper.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "DefaultSession.h"

namespace Satori {

	void AddBoolArg(Nakama::NHttpQueryArgs& args, std::string&& name, bool value)
	{
		value ? args.emplace(name, "true") : args.emplace(name, "false");
	}

	std::string jsonDocToStr(Nakama::rapidjson::Document& document)
	{
		Nakama::rapidjson::StringBuffer buffer;
		Nakama::rapidjson::Writer<Nakama::rapidjson::StringBuffer> writer(buffer);
		document.Accept(writer);
		return buffer.GetString();
	}

	void addVarsToJsonDoc(Nakama::rapidjson::Document& document, const Nakama::NStringMap& vars)
	{
		if (!vars.empty())
		{
			Nakama::rapidjson::Value jsonObj;
			jsonObj.SetObject();

			for (auto& p : vars)
			{
				jsonObj.AddMember(Nakama::rapidjson::Value::StringRefType(p.first.c_str()), p.second, document.GetAllocator());
			}

			document.AddMember("vars", std::move(jsonObj), document.GetAllocator());
		}
	}

	SatoriRestClient::SatoriRestClient(const Nakama::NClientParameters &parameters, Nakama::NHttpTransportPtr httpClient)
	: _httpClient(std::move(httpClient)) {
		NLOG(Nakama::NLogLevel::Info, "Created Satori Client. NakamaSdkVersion: %s", Nakama::getNakamaSdkVersion());

		_host = parameters.host;
		_ssl = parameters.ssl;
		_platformParams = parameters.platformParams;
		_port = parameters.port;
		std::string baseUrl;


		if (_port == Nakama::DEFAULT_PORT)
		{
			_port = parameters.ssl ? 443 : 7350;
			NLOG(Nakama::NLogLevel::Info, "using default port %d", _port);
		}

		_ssl ? baseUrl.append("https") : baseUrl.append("http");
		baseUrl.append("://").append(parameters.host).append(":").append(std::to_string(_port));

		_httpClient->setBaseUri(baseUrl);

		_basicAuthMetadata = "Basic " + Nakama::base64Encode(parameters.serverKey + ":");
	}

	SatoriRestClient::~SatoriRestClient() {
		disconnect();
	}

	void SatoriRestClient::disconnect() {
		_httpClient->cancelAllRequests();
	}

	void SatoriRestClient::tick() {
		_httpClient->tick();
	}

	void SatoriRestClient::authenticate(
		std::string id,
		std::function<void(SSessionPtr)> successCallback,
		Nakama::ErrorCallback errorCallback
	) {
		/*
		try {
			NLOG_INFO("...");

			auto sessionData(std::make_shared<SSession>());
			RestReqContext* ctx = nullptr; return;
			setBasicAuth(ctx);

			if (successCallback)
			{
				ctx->successCallback = [sessionData, successCallback]() {
					SSessionPtr session(new SSession{sessionData->token, sessionData->refresh_token});
					successCallback(session);
				};
			}
			ctx->errorCallback = errorCallback;

			Nakama::NHttpQueryArgs args;

			args.emplace("id", Nakama::encodeURIComponent(id));

			sendReq(ctx, Nakama::NHttpReqMethod::POST, "/v1/authenticate", "", std::move(args));
		}
		catch (std::exception& e)
		{
			NLOG_ERROR("exception: " + std::string(e.what()));
		}
		*/
	}

	void SatoriRestClient::getLiveEvents(
		SSessionPtr session,
		const std::vector<std::string> &liveEventNames,
		std::function<void(const SLiveEventList &)> successCallback,
		Nakama::ErrorCallback errorCallback
	) {
		try {
			NLOG_INFO("...");

			std::string auth(_basicAuthMetadata.append("Bearer ").append(session->token));

			Nakama::NHttpQueryArgs args;

			for (auto& liveEventName : liveEventNames) {
				args.emplace("names", liveEventName);
			}

			Nakama::NHttpRequest req;

			req.method    = Nakama::NHttpReqMethod::GET;
			req.path      = "/v1/live-event";
			req.body      = "";
			req.queryArgs = std::move(args);

			req.headers.emplace("Accept", "application/json");
			req.headers.emplace("Content-Type", "application/json");
			if (!auth.empty()) {
				req.headers.emplace("Authorization", std::move(auth));
			}

			_httpClient->request(req, [this, &successCallback, &errorCallback](Nakama::NHttpResponsePtr response) {
				std::shared_ptr<SLiveEventList> liveEventsData(std::make_shared<SLiveEventList>());
				auto requestSuccessCallback = [liveEventsData, successCallback]()
				{
					successCallback(*liveEventsData);
				};
				// TODO: Convert this boilerplate lambda back into a function that can be used from within Satori cpp. Boilerplate begins here	============
				[&]()//void RestClient::onResponse(RestReqContext* reqContext, NHttpResponsePtr response)
				{
			        if (response->statusCode == 200) // OK
			        {
			            if (successCallback)
			            {
			                bool ok = true;

			                if (reqContext->data)
			                {
			                    google::protobuf::util::JsonParseOptions options;
			                    options.ignore_unknown_fields = true;
								// TODO: Implement parse function from json to SLiveEventList and call it instead of the current google::protobuf::util::JsonStringToMessage(...). Something like this [SLiveEventList obj = onResponse::<SLiveEventList>(response);]
			                    auto status = google::protobuf::util::JsonStringToMessage(response->body, reqContext->data, options);
			                    ok = status.ok();

			                    if (!ok)
			                    {
			                        reqError(reqContext, Nakama::NError("Parse JSON failed. HTTP body: " + response->body + " error: " + status.ToString(), Nakama::ErrorCode::InternalError));
			                    }
			                }

			                if (ok)
			                {
			                    successCallback(*liveEventsData);
			                }
			            }
			        }
			        else
			        {
			            std::string errMessage;
			            Nakama::ErrorCode code = Nakama::ErrorCode::Unknown;

			            if (response->statusCode == Nakama::InternalStatusCodes::CONNECTION_ERROR)
			            {
			                code = Nakama::ErrorCode::ConnectionError;
			                errMessage.append("message: ").append(response->errorMessage);
			            }
			            else if (response->statusCode == Nakama::InternalStatusCodes::CANCELLED_BY_USER)
			            {
			                code = Nakama::ErrorCode::CancelledByUser;
			                errMessage.append("message: ").append(response->errorMessage);
			            }
			            else if (response->statusCode == Nakama::InternalStatusCodes::INTERNAL_TRANSPORT_ERROR)
			            {
			                code = Nakama::ErrorCode::InternalError;
			                errMessage.append("message: ").append(response->errorMessage);
			            }
			            else if (!response->body.empty() && response->body[0] == '{') // have to be JSON
			            {
			                try {
			                    rapidjson::Document document;

			                    if (document.Parse(response->body).HasParseError())
			                    {
			                        errMessage = "Parse JSON failed: " + response->body;
			                        code = Nakama::ErrorCode::InternalError;
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
			                catch (std::exception& e)
			                {
			                    NLOG_ERROR("exception: " + std::string(e.what()));
			                }
			            }

			            if (errMessage.empty())
			            {
			                errMessage.append("message: ").append(response->errorMessage);
			                errMessage.append("\nHTTP status: ").append(std::to_string(response->statusCode));
			                errMessage.append("\nbody: ").append(response->body);
			            }

			            reqError(reqContext, Nakama::NError(std::move(errMessage), code));
			        }
				}();
				// TODO: Convert this boilerplate lambda back into a function that can be used from within Satori cpp. Boilerplate ends here	============
			});
		}
		catch (std::exception& e)
		{
			NLOG_ERROR("exception: " + std::string(e.what()));
		}
	}
}
