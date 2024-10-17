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

#pragma once

#include <set>

#include "SatoriBaseClient.h"
#include "nakama-cpp/satori/HardcodedLowLevelSatoriAPI.h"
#include "nakama-cpp/log/NLogger.h"

namespace Satori {
	template <typename T>
    struct RestReqContext
    {
        std::string auth;
        std::function<void()> successCallback;
        Nakama::ErrorCallback errorCallback;
        T* data = nullptr;
		std::function<bool(std::string, T*)> jsonToType;
    };

	class SatoriRestClient : public SatoriBaseClient {
	public:
		explicit SatoriRestClient(const Nakama::NClientParameters& parameters, Nakama::NHttpTransportPtr httpClient);
		~SatoriRestClient();
		void disconnect();
		void tick();

		void authenticate(
			std::string id,
			std::map<std::string, std::string> defaultProperties,
			std::map<std::string, std::string> customProperties,
			std::function<void(SSessionPtr)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override;

		void authenticateRefresh(
			SSession session,
			std::function<void (SSessionPtr)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void authenticateLogout(
			SSessionPtr session,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void deleteIdentity(
			SSessionPtr session,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void postEvent(
			SSessionPtr session,
			std::vector <SEvent> events,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void getExperiments(
			SSessionPtr session,
			std::vector<std::string> names,
			std::function<void(const SExperimentList&)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void getFlags(
			SSessionPtr session,
			std::vector<std::string> names,
			std::function<void(std::vector<SFlag>)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void getLiveEvents(
			SSessionPtr session,
			const std::vector<std::string>& liveEventNames,
			std::function<void(SLiveEventList)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override;

		void identify(
			SSessionPtr session,
			std::string id,
			std::map<std::string,std::string> defaultProperties,
			std::map<std::string,std::string> customProperties,
			std::function<void (SSession)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void listIdentityProperties(
			SSessionPtr session,
			std::function<void (SProperties)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void updateProperties(
			SSessionPtr session,
			std::map<std::string,std::string> defaultProperties,
			std::map<std::string,std::string> customProperties,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void getMessages(
			SSessionPtr session,
			int32_t limit,
			bool forward,
			std::string cursor,
			std::function<void(SGetMessageListResponse)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void updateMessage(
			SSessionPtr session,
			std::string messageId,
			std::chrono::time_point<std::chrono::system_clock> readTime,
			std::chrono::time_point<std::chrono::system_clock> consumeTime,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

		void deleteMessage(
			SSessionPtr session,
			std::string messageId,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override {}

	private:
		Nakama::NHttpTransportPtr _httpClient;

		// Takes ownership of ctx pointer
		template <typename T>
        void sendReq(
            RestReqContext<T>* ctx,
            Nakama::NHttpReqMethod method,
            std::string&& path,
            std::string&& body,
            Nakama::NHttpQueryArgs&& args = Nakama::NHttpQueryArgs()){
			if(ctx == nullptr) {
				reqError<T>(nullptr, Nakama::NError("Satori request context not found.", Nakama::ErrorCode::InternalError));
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
			            	assert(ctx->jsonToType);
			                if (ctx->data && !ctx->jsonToType(response->body, ctx->data)) {
		                        reqError<T>(ctx, Nakama::NError("Parse JSON failed fro Satori. HTTP body: " + response->body, Nakama::ErrorCode::InternalError));
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
				}();
				// TODO: Convert this boilerplate lambda back into a function that can be used from within Satori cpp. Boilerplate ends here	============
			});

			delete ctx;
	}

		// Does not take ownership of ctx pointer
		template <typename T>
		void reqError(RestReqContext<T>* ctx, const Nakama::NError &error) const{
			NLOG_ERROR(error);

			if (ctx && ctx->errorCallback) {
				ctx->errorCallback(error);
			} else if (_defaultErrorCallback) {
				_defaultErrorCallback(error);
			} else {
				NLOG_WARN("^ error not handled");
			}
		}
	};
}

