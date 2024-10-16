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

namespace Satori {
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

		void getLiveEvents(
			SSessionPtr session,
			const std::vector<std::string>& liveEventNames,
			std::function<void(SLiveEventList)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) override;

	private:
		Nakama::NHttpTransportPtr _httpClient;

	};
}

