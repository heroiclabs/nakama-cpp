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

#include "nakama-cpp/satori/SatoriClientFactory.h"

template<typename R>
R getFromFuture(std::future<R> f, Satori::SClientPtr client) {
	if(!f.valid()) {
		return R();
	}
	while(!(f.wait_for(std::chrono::seconds(0)) == std::future_status::ready)) {
		client->tick();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	return f.get();
}

bool getFromFuture(std::future<void> f, Satori::SClientPtr client) {
	if(!f.valid()) {
		return false;
	}
	while(!(f.wait_for(std::chrono::seconds(0)) == std::future_status::ready)) {
		client->tick();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	return true;
}

int main(int argc, char** argv) {
	try {
		std::cout << "Hello, World, I'm Satori cpp interface!\n";

		bool done = false;

		Nakama::NClientParameters parameters = Nakama::NClientParameters();
		parameters.serverKey = "a76ae76b-3342-4cbd-9c54-f532c5c1a949";
		Satori::SClientPtr client = Satori::createRestClient(parameters, Nakama::createDefaultHttpTransport(parameters.platformParams));

		Satori::SSessionPtr session1 = getFromFuture(client->authenticateAsync("11111111-1111-0000-0000-000000000000",{},{{"pushTokenIos", "foo"}}), client);
		Satori::SSessionPtr session2 = getFromFuture(client->authenticateAsync("22222222-2222-0000-0000-000000000000",{},{{"pushTokenAndroid", "bar"}}), client);
		Satori::SSessionPtr session3 = getFromFuture(client->identifyAsync(session1, "22222222-2222-0000-0000-000000000000"), client);
		Satori::SFlagList testResult = getFromFuture(client->getFlagsAsync(session3), client);
		std::cout << "Flags:" << testResult.flags.size() << std::endl;
		Satori::SFlagList testResult2 = getFromFuture(client->getFlagsAsync(session3, {"Hiro-Event-Leaderboards", "Hiro-Inventory"}), client);
		Satori::SEvent testEvent;
		testEvent.name = "testEvent";
		testEvent.timestamp = 1337;
		std::vector<Satori::SEvent> testEvents = {testEvent};
		bool testResult3 = getFromFuture(client->postEventAsync(session3, testEvents), client);
		Satori::SLiveEventList testResult4 = getFromFuture(client->getLiveEventsAsync(session3), client);

		Satori::SLiveEventList liveEvents = getFromFuture(client->getLiveEventsAsync(session3), client);
		std::cout << "Live events num:" << liveEvents.live_events.size() << std::endl;
	} catch (const std::future_error& e) {
		std::cout << "Caught a future_error with code \"" << e.code()
				  << "\"\nMessage: \"" << e.what() << "\"\n";
	} catch (const std::exception& e) {
		std::cout << "Caught a exception with code \"" << e.what() << "\"\n";
	}
	return 0;
}
