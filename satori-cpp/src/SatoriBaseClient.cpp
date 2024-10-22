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

#include "SatoriBaseClient.h"
#include "nakama-cpp/NException.h"

namespace Satori {
	std::future<SSessionPtr> SatoriBaseClient::authenticateAsync(
		const std::string& id,
		const std::unordered_map<std::string,std::string>& defaultProperties,
		const std::unordered_map<std::string,std::string>& customProperties
	) {
		auto promise = std::make_shared<std::promise<SSessionPtr>>();

		authenticate(id, defaultProperties, customProperties,
			[=](const SSessionPtr& session) {
				promise->set_value(session);
			},
			[=](const Nakama::NError& error) {
				promise->set_exception(std::make_exception_ptr<Nakama::NException>(error));
			});

		return promise->get_future();
	}

	std::future<SFlagList> SatoriBaseClient::getFlagsAsync(
		SSessionPtr session,
		const std::vector<std::string>& names
	) {
		std::shared_ptr<std::promise<SFlagList>> promise = std::make_shared<std::promise<SFlagList>>();

		getFlags(session, names,
			[=](const SFlagList& flags) {
				promise->set_value(flags);
			},
			[=](const Nakama::NError& error) {
				promise->set_exception(std::make_exception_ptr<Nakama::NException>(error));
			});

		return promise->get_future();
	}

	std::future<SLiveEventList> SatoriBaseClient::getLiveEventsAsync(
		SSessionPtr session,
		const std::vector<std::string>& liveEventNames
	) {
		std::shared_ptr<std::promise<SLiveEventList>> promise = std::make_shared<std::promise<SLiveEventList>>();

		getLiveEvents(session, liveEventNames,
			[=](const SLiveEventList& liveEvents) {
				promise->set_value(liveEvents);
			},
			[=](const Nakama::NError& error) {
				promise->set_exception(std::make_exception_ptr<Nakama::NException>(error));
			});

		return promise->get_future();
	}

	std::future<SSessionPtr> SatoriBaseClient::identifyAsync(
		SSessionPtr session,
		const std::string& id,
		const std::unordered_map<std::string, std::string>& defaultProperties,
		const std::unordered_map<std::string, std::string>& customProperties
	) {
		std::shared_ptr<std::promise<SSessionPtr>> promise = std::make_shared<std::promise<SSessionPtr>>();

		identify(session, id, defaultProperties, customProperties,
			[=](const SSessionPtr& session) {
				promise->set_value(session);
			},
			[=](const Nakama::NError& error) {
				promise->set_exception(std::make_exception_ptr<Nakama::NException>(error));
			});

		return promise->get_future();
	}

	std::future<void> SatoriBaseClient::updatePropertiesAsync(
		SSessionPtr session,
		const std::unordered_map<std::string, std::string>& defaultProperties,
		const std::unordered_map<std::string, std::string>& customProperties,
		const bool recompute
	) {
		std::shared_ptr<std::promise<void>> promise = std::make_shared<std::promise<void>>();

		updateProperties(session, defaultProperties, customProperties, recompute,
			[=]() {
				promise->set_value();
			},
			[=](const Nakama::NError& error) {
				promise->set_exception(std::make_exception_ptr<Nakama::NException>(error));
			});

		return promise->get_future();
	}
}
