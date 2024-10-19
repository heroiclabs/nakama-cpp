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
    struct RestReqContext
    {
        std::string auth;
        std::function<void()> successCallback;
        Nakama::ErrorCallback errorCallback;
        std::shared_ptr<SFromJsonInterface> data = nullptr;
    };

	class SatoriRestClient : public SatoriBaseClient {
	public:
		explicit SatoriRestClient(const Nakama::NClientParameters& parameters, Nakama::NHttpTransportPtr httpClient);
		~SatoriRestClient();
		void disconnect() override;
		void tick() override;

		void authenticate(
			std::string id,
			std::map<std::string, std::string> defaultProperties,
			std::map<std::string, std::string> customProperties,
			std::function<void(const SSessionPtr&)> successCallback,
			Nakama::ErrorCallback errorCallback
		) override;

		void authenticateRefresh(
			SSession session,
			std::function<void (const SSessionPtr&)> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}

		void authenticateLogout(
			SSessionPtr session,
			std::function<void()> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}

		void deleteIdentity(
			SSessionPtr session,
			std::function<void()> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}

		void postEvent(
			SSessionPtr session,
			std::vector <SEvent> events,
			std::function<void()> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}

		void getExperiments(
			SSessionPtr session,
			std::vector<std::string> names,
			std::function<void(const SExperimentList&)> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}

		void getFlags(
			SSessionPtr session,
			std::vector<std::string> names,
			std::function<void(std::vector<SFlag>)> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}

		void getLiveEvents(
			SSessionPtr session,
			const std::vector<std::string>& liveEventNames,
			std::function<void(SLiveEventList)> successCallback,
			Nakama::ErrorCallback errorCallback
		) override;

		void identify(
			SSessionPtr session,
			std::string id,
			std::map<std::string,std::string> defaultProperties,
			std::map<std::string,std::string> customProperties,
			std::function<void (const SSessionPtr&)> successCallback,
			Nakama::ErrorCallback errorCallback
		) override;

		void listIdentityProperties(
			SSessionPtr session,
			std::function<void (SProperties)> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}

		void updateProperties(
			SSessionPtr session,
			std::map<std::string,std::string> defaultProperties,
			std::map<std::string,std::string> customProperties,
			std::function<void()> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}

		void getMessages(
			SSessionPtr session,
			int32_t limit,
			bool forward,
			std::string cursor,
			std::function<void(SGetMessageListResponse)> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}

		void updateMessage(
			SSessionPtr session,
			std::string messageId,
			std::chrono::time_point<std::chrono::system_clock> readTime,
			std::chrono::time_point<std::chrono::system_clock> consumeTime,
			std::function<void()> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}

		void deleteMessage(
			SSessionPtr session,
			std::string messageId,
			std::function<void()> successCallback,
			Nakama::ErrorCallback errorCallback
		) override {}
	private:
		RestReqContext* createReqContext(std::shared_ptr<SFromJsonInterface> data);
		void setBasicAuth(RestReqContext* ctx);
		void setSessionAuth(RestReqContext* ctx, const SSessionPtr session);
		// Takes ownership of ctx pointer
        void sendReq(
            RestReqContext* ctx,
            Nakama::NHttpReqMethod method,
            std::string&& path,
            std::string&& body,
            Nakama::NHttpQueryArgs&& args = Nakama::NHttpQueryArgs());

		// Does not take ownership of ctx pointer
		void reqError(RestReqContext* ctx, const Nakama::NError &error) const;

	private:
		//std::set<RestReqContext*> _reqContexts;
		Nakama::NHttpTransportPtr _httpClient;
	};
}

