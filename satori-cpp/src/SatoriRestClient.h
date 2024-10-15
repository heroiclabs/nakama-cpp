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
#include <google/protobuf/message.h>

#include "SatoriBaseClient.h"
#include "HardcodedLowLevelSatoriAPI.h"

namespace Satori {
	struct RestReqContext
	{
		std::string auth;
		std::function<void()> successCallback;
		Nakama::ErrorCallback errorCallback;
		google::protobuf::Message* data = nullptr;
	};

	class SatoriRestClient : public SatoriBaseClient {
	public:
		explicit SatoriRestClient(const Nakama::NClientParameters& parameters, Nakama::NHttpTransportPtr httpClient);
		~SatoriRestClient();
		void disconnect();
		void tick();

	private:
		RestReqContext* createReqContext(google::protobuf::Message* data);
		void setBasicAuth(RestReqContext* ctx);
		void setSessionAuth(RestReqContext* ctx, Nakama::NSessionPtr session);

		void sendReq(
			RestReqContext* ctx,
			Nakama::NHttpReqMethod method,
			std::string&& path,
			std::string&& body,
			Nakama::NHttpQueryArgs&& args = Nakama::NHttpQueryArgs());

		void sendRpc(
			RestReqContext* ctx,
			const std::string& id,
			const std::string& payload,
			Nakama::NHttpQueryArgs&& args
		);

		void onResponse(RestReqContext* reqContext, Nakama::NHttpResponsePtr response);
		void reqError(RestReqContext* reqContext, const Nakama::NError& error);

	private:
		std::set<RestReqContext*> _reqContexts;
		Nakama::NHttpTransportPtr _httpClient;

	};
}