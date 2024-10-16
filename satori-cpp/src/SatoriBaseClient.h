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

#include <string>

#include "nakama-cpp/satori/SClientInterface.h"
#include "nakama-cpp/satori/SatoriClientFactory.h"
#include "nakama-cpp/satori/HardcodedLowLevelSatoriAPI.h"

namespace Satori {
	class SatoriBaseClient : public SClientInterface {
	public:
		std::future<SSessionPtr> authenticateAsync(
			std::string id
		) override;
		std::future<SLiveEventList> getLiveEventsAsync(
			SSessionPtr session,
			const std::vector<std::string>& liveEventNames = {}
		) override;

	protected:
		int _port = 0;
		std::string _host;
		bool _ssl = false;
		std::string _basicAuthMetadata;
		Nakama::ErrorCallback _defaultErrorCallback;
		void* _userData = nullptr;
		Nakama::NPlatformParameters _platformParams;
	};
}