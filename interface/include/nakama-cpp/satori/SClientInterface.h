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
#include "google/protobuf/message.h"

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

    	virtual void authenticateLogout(
            SSessionPtr session,
    		std::function<void()> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual void deleteIdentity(
            SSessionPtr session,
			std::function<void()> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual void postEvent(
            SSessionPtr session,
    		const std::vector<SEvent>& events,
    		std::function<void()> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual void getExperiments(
            SSessionPtr session,
    		const std::vector<std::string>& names,
			std::function<void(SExperimentList)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual void getFlags(
            SSessionPtr session,
			const std::vector<std::string>& names = {},
			std::function<void(SFlagList)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<SFlagList> getFlagsAsync(
			SSessionPtr session,
			const std::vector<std::string>& names = {}) = 0;

    	/**
		 * Request to get all live events.
		 *
		 * @param session The session of the user.
		 * @param liveEventNames Live event names; if empty string all live events are returned.
		 */
    	virtual void getLiveEvents(
            SSessionPtr session,
			const std::vector<std::string>& liveEventNames = {},
			std::function<void(SLiveEventList)> successCallback = nullptr,
		    Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	/**
		 * Fetch one or more users by id, usernames, and Facebook ids.
		 *
		 * @param session The session of the user.
		 * @param liveEventNames Live event names; if empty string all live events are returned.
		 */
    	virtual std::future<SLiveEventList> getLiveEventsAsync(
    		SSessionPtr session,
			const std::vector<std::string>& liveEventNames = {}) = 0;

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

    	virtual void updateProperties(
    		SSessionPtr session,
    		const std::unordered_map<std::string,std::string>& defaultProperties = {},
    		const std::unordered_map<std::string,std::string>& customProperties = {},
    		std::function<void()> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual std::future<void> updatePropertiesAsync(
			SSessionPtr session,
			const std::unordered_map<std::string,std::string>& defaultProperties = {},
			const std::unordered_map<std::string,std::string>& customProperties = {}) = 0;

    	virtual void getMessages(
			SSessionPtr session,
			int32_t limit,
			bool forward,
			const std::string& cursor,
			std::function<void(SGetMessageListResponse)> successCallback = nullptr,
			Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual void updateMessage(
    		SSessionPtr session,
    		const std::string& messageId,
    		const std::chrono::time_point<std::chrono::system_clock>& readTime,
    		const std::chrono::time_point<std::chrono::system_clock>& consumeTime,
    		std::function<void()> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;

    	virtual void deleteMessage(
    		SSessionPtr session,
    		const std::string& messageId,
    		std::function<void()> successCallback = nullptr,
    		Nakama::ErrorCallback errorCallback = nullptr) = 0;



    	    /*
// List available live events.
rpc GetLiveEvents (GetLiveEventsRequest) returns (LiveEventList) {
option (google.api.http).get = "/v1/live-event";
}
* */

/*
rpc Authenticate (AuthenticateRequest) returns (Session) {
option (google.api.http) = {
post: "/v1/authenticate",
body: "*"
};
option (grpc.gateway.protoc_gen_openapiv2.options.openapiv2_operation) = {
security: {
security_requirement: {
key: "BasicAuth";
value: {};
}
}
};
}

// Log out a session, invalidate a refresh token, or log out all sessions/refresh tokens for a user.
rpc AuthenticateLogout (AuthenticateLogoutRequest) returns (google.protobuf.Empty) {
option (google.api.http) = {
post: "/v1/authenticate/logout",
body: "*"
};
}

// Refresh a user's session using a refresh token retrieved from a previous authentication request.
rpc AuthenticateRefresh (AuthenticateRefreshRequest) returns (Session) {
option (google.api.http) = {
post: "/v1/authenticate/refresh",
body: "*"
};
option (grpc.gateway.protoc_gen_openapiv2.options.openapiv2_operation) = {
security: {
security_requirement: {
key: "BasicAuth";
value: {};
}
}
};
}

// Delete the caller's identity and associated data.
rpc DeleteIdentity(google.protobuf.Empty) returns (google.protobuf.Empty) {
option (google.api.http).delete = "/v1/identity";
}

// Publish an event for this session.
rpc Event(EventRequest) returns (google.protobuf.Empty) {
option (google.api.http) = {
post: "/v1/event",
body: "*"
};
}

// Get or list all available experiments for this identity.
rpc GetExperiments (GetExperimentsRequest) returns (ExperimentList) {
option (google.api.http).get = "/v1/experiment";
}

// List all available flags for this identity.
rpc GetFlags (GetFlagsRequest) returns (FlagList) {
option (google.api.http).get = "/v1/flag";
option (grpc.gateway.protoc_gen_openapiv2.options.openapiv2_operation) = {
// Either HTTP key in query param or Bearer authentication.
security: {
security_requirement: {
key: "HttpKeyAuth";
value: {};
}
security_requirement: {
key: "BearerJwt";
value: {};
}
}
};
}

// List available live events.
rpc GetLiveEvents (GetLiveEventsRequest) returns (LiveEventList) {
option (google.api.http).get = "/v1/live-event";
}

// A healthcheck which load balancers can use to check the service.
rpc Healthcheck (google.protobuf.Empty) returns (google.protobuf.Empty) {
option (google.api.http).get = "/healthcheck";
}

// Enrich/replace the current session with new identifier.
rpc Identify(IdentifyRequest) returns (Session) {
option (google.api.http) = {
put: "/v1/identify",
body: "*"
};
}

// List properties associated with this identity.
rpc ListProperties (google.protobuf.Empty) returns (Properties) {
option (google.api.http).get = "/v1/properties";
}

// A readycheck which load balancers can use to check the service.
rpc Readycheck (google.protobuf.Empty) returns (google.protobuf.Empty) {
option (google.api.http).get = "/readycheck";
}

// Update identity properties.
rpc UpdateProperties(UpdatePropertiesRequest) returns (google.protobuf.Empty) {
option (google.api.http) = {
put: "/v1/properties",
body: "*"
};
}

// Get the list of messages for the identity.
rpc GetMessageList (GetMessageListRequest) returns (GetMessageListResponse) {
option (google.api.http).get = "/v1/message";
}

// Updates a message for an identity.
rpc UpdateMessage (UpdateMessageRequest) returns (google.protobuf.Empty) {
option (google.api.http) = {
put: "/v1/message/{id}",
body: "*"
};
}

// Deletes a message for an identity.
rpc DeleteMessage (DeleteMessageRequest) returns (google.protobuf.Empty) {
option (google.api.http).delete = "/v1/message/{id}";
}
*/
    };

    using SClientPtr = std::shared_ptr<SClientInterface>;

}
