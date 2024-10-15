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

#include <string>
#include <iostream>

#include "nakama-cpp/ClientFactory.h"
#include "../src/SatoriRestClient.h"
#include "../../impl/httpCurl/NHttpClientLibCurl.h"

Nakama::NHttpTransportPtr createDefaultHttpTransport(const Nakama::NPlatformParameters& platformParams)
{
	(void)platformParams;  // silence unused variable warning on some platforms
	return Nakama::NHttpTransportPtr(new Nakama::NHttpClientLibCurl(platformParams));
}

int main(int argc, char** argv) {
	std::cout << "Hello, World, I'm Satori cpp interface!\n";

	Nakama::NClientParameters parameters = Nakama::NClientParameters();
	Satori::SatoriRestClient client = Satori::SatoriRestClient(parameters, createDefaultHttpTransport(parameters.platformParams));

	Satori::GetLiveEventsRequest livEvReq;
	Satori::GetLiveEvents(livEvReq);

	return 0;
}
