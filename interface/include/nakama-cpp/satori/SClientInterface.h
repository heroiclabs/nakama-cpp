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

#include "HardcodedLowLevelSatoriAPI.h"
#include "SExport.h"
#include "nakama-cpp/NClientInterface.h"

namespace Satori {

    /**
     * A client interface to interact with Satori server.
     */
    class SATORI_API SClientInterface
    {
    public:
        virtual ~SClientInterface() {}

    	/**
		 * Request to get all live events.
		 *
		 * @param session The session of the user.
		 * @param liveEventNames Live event names; if empty string all live events are returned.
		 */
    	virtual void getLiveEvents(
		    Nakama::NSessionPtr session,
			const std::vector<std::string>& liveEventNames = {},
			std::function<void(const SLiveEventList&)> successCallback = nullptr,
		    Nakama::ErrorCallback errorCallback = nullptr
		) = 0;

    	/**
		 * Fetch one or more users by id, usernames, and Facebook ids.
		 *
		 * @param session The session of the user.
		 * @param ids List of user IDs.
		 * @param usernames List of usernames.
		 * @param facebookIds List of Facebook IDs.
		 */
    	virtual std::future<SLiveEventList> getLiveEventsAsync(
		    Nakama::NSessionPtr session,
			const std::vector<std::string>& liveEventNames = {}
		) = 0;

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
