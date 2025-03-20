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

#include "../interface/include/nakama-cpp/satori/SatoriClientFactory.h"
#include "../satori-cpp/src/SatoriRestClient.h"

namespace Satori {
	SClientPtr createRestClient(
		const Nakama::NClientParameters &parameters,
		Nakama::NHttpTransportPtr httpTransport
	) {
		if (!httpTransport)
		{
			httpTransport = Nakama::createDefaultHttpTransport(parameters.platformParams);
		}
		if (!httpTransport)
		{
			NLOG_ERROR("HTTP transport was null and a default one could not be constructed.");
			return nullptr;
		}
		if(parameters.timeout >= 0)
		{
			httpTransport->setTimeout(parameters.timeout);
		}
		return std::make_shared<SatoriRestClient>(parameters, httpTransport);
	}
}
