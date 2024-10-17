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
#include "StrUtil.h"
#include "RapidjsonHelper.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>



#include "DefaultSession.h"

namespace Satori {

	void AddBoolArg(Nakama::NHttpQueryArgs& args, std::string&& name, bool value) {
		value ? args.emplace(name, "true") : args.emplace(name, "false");
	}

	std::string jsonDocToStr(Nakama::rapidjson::Document& document) {
		Nakama::rapidjson::StringBuffer buffer;
		Nakama::rapidjson::Writer<Nakama::rapidjson::StringBuffer> writer(buffer);
		document.Accept(writer);
		return buffer.GetString();
	}

	void addVarsToJsonDoc(Nakama::rapidjson::Document& document, const Nakama::NStringMap& vars) {
		if (!vars.empty()) {
			Nakama::rapidjson::Value jsonObj;
			jsonObj.SetObject();

			for (auto& p : vars) {
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


		if (_port == Nakama::DEFAULT_PORT) {
			_port = parameters.ssl ? 443 : 7450;
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
		std::map<std::string, std::string> defaultProperties,
		std::map<std::string, std::string> computedProperties,
		std::function<void(const SSessionPtr&)> successCallback,
		Nakama::ErrorCallback errorCallback
	) {
		try {
			NLOG_INFO("...");

			auto sessionData(std::make_shared<SSession>());
			RestReqContext* ctx = createReqContext(sessionData);
			setBasicAuth(ctx);
			ctx->successCallback = [sessionData, successCallback]()
			{
				successCallback(sessionData);
			};
			ctx->errorCallback = std::move(errorCallback);

			Nakama::NHttpQueryArgs args;

			args.emplace("id", Nakama::encodeURIComponent(id));

			sendReq(ctx, Nakama::NHttpReqMethod::POST, "/v1/authenticate", "", std::move(args));

		} catch (std::exception& e) {
			NLOG_ERROR("exception: " + std::string(e.what()));
		}
	}

	void SatoriRestClient::getLiveEvents(
		SSessionPtr session,
		const std::vector<std::string>& liveEventNames,
		std::function<void(SLiveEventList)> successCallback,
		Nakama::ErrorCallback errorCallback
	) {
		try {
			NLOG_INFO("...");

			std::string auth(_basicAuthMetadata.append("Bearer ").append(session->token));

			Nakama::NHttpQueryArgs args;

			for (auto& liveEventName : liveEventNames) {
				args.emplace("names", liveEventName);
			}

			std::shared_ptr<SLiveEventList> liveEventsData(std::make_shared<SLiveEventList>());
			RestReqContext* ctx(createReqContext(liveEventsData));
			setSessionAuth(ctx, session);
			ctx->successCallback = [liveEventsData, successCallback]()
			{
				successCallback(*liveEventsData);
			};
			ctx->errorCallback = std::move(errorCallback);

			sendReq(ctx, Nakama::NHttpReqMethod::GET, "/v1/live-event", "", std::move(args));

		} catch (std::exception& e) {
			NLOG_ERROR("exception: " + std::string(e.what()));
		}
	}

	RestReqContext* SatoriRestClient::createReqContext(std::shared_ptr<SFromJsonInterface> data) {
		RestReqContext* ctx = new RestReqContext();
		ctx->data = data;
		//_reqContexts.emplace(ctx);
		return ctx;
	}

	void SatoriRestClient::setBasicAuth(RestReqContext *ctx) {
		ctx->auth.append(_basicAuthMetadata);
	}

	void SatoriRestClient::setSessionAuth(RestReqContext *ctx, const SSessionPtr session) {
		ctx->auth.append("Bearer ").append(session->token);
	}

	void SatoriRestClient::sendReq(
		RestReqContext *ctx,
		Nakama::NHttpReqMethod method,
		std::string &&path,
		std::string &&body,
		Nakama::NHttpQueryArgs &&args
	) {
		if(ctx == nullptr) {
			reqError(nullptr, Nakama::NError("Satori request context not found.", Nakama::ErrorCode::InternalError));
			return;
		}

		Nakama::NHttpRequest req;

		req.method    = method;
		req.path      = std::move(path);
		req.body      = std::move(body);
		req.queryArgs = std::move(args);

		req.headers.emplace("Accept", "application/json");
		req.headers.emplace("Content-Type", "application/json");
		if (!ctx->auth.empty()) {
			req.headers.emplace("Authorization", std::move(ctx->auth));
		}

		_httpClient->request(req, [this, ctx](Nakama::NHttpResponsePtr response) {
			// TODO: Convert this boilerplate lambda back into a function that can be used from within Satori cpp. Boilerplate begins here	============
			[&]()//void RestClient::onResponse(RestReqContext* reqContext, NHttpResponsePtr response)
			{
		        if (response->statusCode == 200) {// OK
		            if (ctx && ctx->successCallback) {
		                bool ok = true;
		                if (ctx->data && !ctx->data->jsonToType(response->body, ctx->data)) {
	                        reqError(ctx, Nakama::NError("Parse JSON failed fro Satori. HTTP body: " + response->body, Nakama::ErrorCode::InternalError));
	                    }

		                if (ok) {
		                    ctx->successCallback();
		                }
		            }
		        } else {
		            std::string errMessage;
		            Nakama::ErrorCode code = Nakama::ErrorCode::Unknown;

		            if (response->statusCode == Nakama::InternalStatusCodes::CONNECTION_ERROR) {
		                code = Nakama::ErrorCode::ConnectionError;
		                errMessage.append("message: ").append(response->errorMessage);
		            } else if (response->statusCode == Nakama::InternalStatusCodes::CANCELLED_BY_USER) {
		                code = Nakama::ErrorCode::CancelledByUser;
		                errMessage.append("message: ").append(response->errorMessage);
		            } else if (response->statusCode == Nakama::InternalStatusCodes::INTERNAL_TRANSPORT_ERROR) {
		                code = Nakama::ErrorCode::InternalError;
		                errMessage.append("message: ").append(response->errorMessage);
		            } else if (!response->body.empty() && response->body[0] == '{') {// have to be JSON
		                /*
		                 try {
		                    rapidjson::Document document;

		                    if (document.Parse(response->body).HasParseError()) {
		                        errMessage = "Parse JSON failed: " + response->body;
		                        code = Nakama::ErrorCode::InternalError;
		                    } else {
		                        auto& jsonMessage = document["message"];
		                        auto& jsonCode    = document["code"];

		                        if (jsonMessage.IsString()) {
		                            errMessage.append("message: ").append(jsonMessage.GetString());
		                        }

		                        if (jsonCode.IsNumber()) {
		                            int serverErrCode = jsonCode.GetInt();

		                            switch (serverErrCode) {
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
		                } catch (std::exception& e) {
		                    NLOG_ERROR("exception: " + std::string(e.what()));
		                }
		                */
		            }

		            if (errMessage.empty()) {
		                errMessage.append("message: ").append(response->errorMessage);
		                errMessage.append("\nHTTP status: ").append(std::to_string(response->statusCode));
		                errMessage.append("\nbody: ").append(response->body);
		            }

		            reqError(ctx, Nakama::NError(std::move(errMessage), code));
		        }

				delete ctx;
			}();
			// TODO: Convert this boilerplate lambda back into a function that can be used from within Satori cpp. Boilerplate ends here	============
		});
	}

	void SatoriRestClient::reqError(RestReqContext *ctx, const Nakama::NError &error) const {
		NLOG_ERROR(error);

		if (ctx && ctx->errorCallback) {
			ctx->errorCallback(error);
		} else if (_defaultErrorCallback) {
			_defaultErrorCallback(error);
		} else {
			NLOG_WARN("^ error not handled");
		}
	}
}
