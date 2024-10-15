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

#include "SatoriRestClient.h"

#include "nakama-cpp/NakamaVersion.h"
#include "nakama-cpp/log/NLogger.h"

namespace Satori {
	SatoriRestClient::SatoriRestClient(const Nakama::NClientParameters &parameters, Nakama::NHttpTransportPtr httpClient)
	: _httpClient(std::move(httpClient)) {
		NLOG(Nakama::NLogLevel::Info, "Created. NakamaSdkVersion: %s", Nakama::getNakamaSdkVersion());

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

		_basicAuthMetadata = "Basic " + base64Encode(parameters.serverKey + ":");
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
}
