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

#include <memory>
#include <future>

#include "HardcodedLowLevelSatoriAPI.h"
#include "nakama-cpp/NClientInterface.h"

namespace Satori {

    using SSessionPtr = std::shared_ptr<SSession>;

    /**
     * A client interface to interact with Satori server.
     */
    class NAKAMA_API SClientInterface
    {
    public:
    	virtual ~SClientInterface() {}

    	virtual void disconnect() = 0;
    	virtual void tick() = 0;

    	virtual void authenticate(
    		const std::string& id,
    		const std::unordered_map<std::string,std::string>& defaultProperties = {},
    		const std::unordered_map<std::string,std::string>& customProperties = {},
    		std::function<void (SSessionPtr)> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SSessionPtr> authenticateAsync(
    		const std::string& id,
			const std::unordered_map<std::string,std::string>& defaultProperties = {},
			const std::unordered_map<std::string,std::string>& customProperties = {}) = 0;

    	virtual void authenticateRefresh(
            SSessionPtr session,
    		std::function<void (SSessionPtr)> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SSessionPtr> authenticateRefreshAsync(
			SSessionPtr session) = 0;

    	virtual void authenticateLogout(
            SSessionPtr session,
    		std::function<void()> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<void> authenticateLogoutAsync(
			SSessionPtr session) = 0;

    	virtual void deleteIdentity(
            SSessionPtr session,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<void> deleteIdentityAsync(
    	SSessionPtr session) = 0;

    	virtual void postEvent(
			SSessionPtr session,
			const std::vector<SEvent>& events,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<void> postEventAsync(
			SSessionPtr session,
			const std::vector<SEvent>& events) = 0;

    	virtual void serverEvent(
			const std::vector<SEvent>& events,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<void> serverEventAsync(
			const std::vector<SEvent>& events) = 0;

    	virtual void getExperiments(
            SSessionPtr session,
    		const SGetExperimentsRequest& request,
			std::function<void(SExperimentList)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SExperimentList> getExperimentsAsync(
			SSessionPtr session,
    		const SGetExperimentsRequest& request) = 0;

    	virtual void getFlags(
    		const std::string& httpKey,
			const SGetFlagsRequest &request,
			std::function<void(SFlagList)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SFlagList> getFlagsAsync(
    		const std::string& httpKey,
			const SGetFlagsRequest &request) = 0;

    	virtual void getFlags(
            SSessionPtr session,
			const SGetFlagsRequest &request,
			std::function<void(SFlagList)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SFlagList> getFlagsAsync(
			SSessionPtr session,
			const SGetFlagsRequest &request) = 0;

    	virtual void getFlagOverrides(
    		const std::string& httpKey,
			const SGetFlagsRequest &request,
			std::function<void(SFlagOverrideList)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SFlagOverrideList> getFlagOverridesAsync(
    		const std::string& httpKey,
			const SGetFlagsRequest &request) = 0;

    	virtual void getFlagOverrides(
			SSessionPtr session,
			const SGetFlagsRequest &request,
			std::function<void(SFlagOverrideList)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SFlagOverrideList> getFlagOverridesAsync(
			SSessionPtr session,
			const SGetFlagsRequest &request) = 0;

    	virtual void getLiveEvents(
            SSessionPtr session,
			const SGetLiveEventsRequest& request,
			std::function<void(SLiveEventList)> successCallback = nullptr,
		    Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SLiveEventList> getLiveEventsAsync(
    		SSessionPtr session,
			const SGetLiveEventsRequest& request) = 0;

    	virtual void joinLiveEvent(
			SSessionPtr session,
			const std::string& id,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr
		) = 0;

    	virtual std::future<void> joinLiveEventAsync(
			SSessionPtr session,
			const std::string& id
		) = 0;

    	virtual void identify(
            SSessionPtr session,
    		const std::string& id,
    		const std::unordered_map<std::string,std::string>& defaultProperties = {},
    		const std::unordered_map<std::string,std::string>& customProperties = {},
    		std::function<void (SSessionPtr)> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SSessionPtr> identifyAsync(
			SSessionPtr session,
			const std::string& id,
			const std::unordered_map<std::string,std::string>& defaultProperties = {},
			const std::unordered_map<std::string,std::string>& customProperties = {}) = 0;

    	virtual void listIdentityProperties(
            SSessionPtr session,
    		std::function<void (SProperties)> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SProperties> listIdentityPropertiesAsync(
			SSessionPtr session) = 0;

    	virtual void updateProperties(
    		SSessionPtr session,
    		const std::unordered_map<std::string,std::string>& defaultProperties = {},
    		const std::unordered_map<std::string,std::string>& customProperties = {},
			const bool recompute = false,
    		std::function<void()> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<void> updatePropertiesAsync(
			SSessionPtr session,
			const std::unordered_map<std::string,std::string>& defaultProperties = {},
			const std::unordered_map<std::string,std::string>& customProperties = {},
			const bool recompute = false) = 0;

    	virtual void getMessages(
			SSessionPtr session,
			int32_t limit,
			bool forward,
			const std::string& cursor,
			std::function<void(SGetMessageListResponse)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SGetMessageListResponse> getMessagesAsync(
			SSessionPtr session,
			int32_t limit,
			bool forward,
			const std::string& cursor) = 0;

    	virtual void updateMessage(
    		SSessionPtr session,
    		const std::string& messageId,
    		const Nakama::NTimestamp readTime,
    		const Nakama::NTimestamp consumeTime,
    		std::function<void()> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<void> updateMessageAsync(
			SSessionPtr session,
			const std::string& messageId,
			const Nakama::NTimestamp readTime,
			const Nakama::NTimestamp consumeTime) = 0;

    	virtual void deleteMessage(
    		SSessionPtr session,
    		const std::string& messageId,
    		std::function<void()> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<void> deleteMessageAsync(
			SSessionPtr session,
			const std::string& messageId) = 0;
    };

    using SClientPtr = std::shared_ptr<SClientInterface>;

}
