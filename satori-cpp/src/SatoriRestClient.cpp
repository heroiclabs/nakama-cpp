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

		if (_reqContexts.size() > 0)
		{
			NLOG(Nakama::NLogLevel::Warn, "Not handled %u request(s) detected.", _reqContexts.size());

			for (RestReqContext* reqContext : _reqContexts)
			{
				delete reqContext;
			}

			_reqContexts.clear();
		}
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
		try {
			NLOG_INFO("...");

			auto sessionData(std::make_shared<SSession>());
			RestReqContext* ctx = nullptr; return;// TODO: Critical! this must be a Message descendant: createReqContext(sessionData.get());
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
	}

	void SatoriRestClient::getLiveEvents(
		SSessionPtr session,
		const std::vector<std::string> &liveEventNames,
		std::function<void(const SLiveEventList &)> successCallback,
		Nakama::ErrorCallback errorCallback
	) {
		try {
			NLOG_INFO("...");

			auto liveEventsData(std::make_shared<SLiveEventList>());
			RestReqContext* ctx = nullptr;//createReqContext(liveEventsData.get());
			setSessionAuth(ctx, session);

			if (successCallback)
			{
				ctx->successCallback = [liveEventsData, successCallback]()
				{
					successCallback(*liveEventsData);
				};
			}
			ctx->errorCallback = errorCallback;

			Nakama::NHttpQueryArgs args;

			for (auto& liveEventName : liveEventNames)
			{
				args.emplace("names", liveEventName);
			}

			sendReq(ctx, Nakama::NHttpReqMethod::GET, "/v1/live-event", "", std::move(args));
		}
		catch (std::exception& e)
		{
			NLOG_ERROR("exception: " + std::string(e.what()));
		}
	}

	RestReqContext* SatoriRestClient::createReqContext(google::protobuf::Message *data) {
		RestReqContext* ctx = new RestReqContext();
		ctx->data = data;
		_reqContexts.emplace(ctx);
		return ctx;
	}

	void SatoriRestClient::setBasicAuth(RestReqContext *ctx) {
		ctx->auth.append(_basicAuthMetadata);
	}

	void SatoriRestClient::setSessionAuth(RestReqContext *ctx, SSessionPtr session) {
		ctx->auth.append("Bearer ").append(session->token);
	}

	void SatoriRestClient::sendReq(RestReqContext *ctx, Nakama::NHttpReqMethod method, std::string &&path,
		std::string &&body, Nakama::NHttpQueryArgs &&args) {
	}

	void SatoriRestClient::sendRpc(RestReqContext *ctx, const std::string &id, const std::string &payload,
		Nakama::NHttpQueryArgs &&args) {
	}

	void SatoriRestClient::onResponse(RestReqContext *reqContext, Nakama::NHttpResponsePtr response) {
	}

	void SatoriRestClient::reqError(RestReqContext *reqContext, const Nakama::NError &error) {
	}
}
