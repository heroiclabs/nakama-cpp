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
#include <vector>
#include <map>
#include <ctime>

// Log out a session, invalidate a refresh token, or log out all sessions/refresh tokens for a user.
struct AuthenticateLogoutRequest {
  // Session token to log out.
  std::string token;
  // Refresh token to invalidate.
  std::string refresh_token;
};

// Authenticate against the server with a refresh token.
struct AuthenticateRefreshRequest {
  // Refresh token.
  std::string refresh_token;
};

// Authentication request
struct AuthenticateRequest {
  // Identity ID. Must be between eight and 128 characters (inclusive).
  // Must be an alphanumeric string with only underscores and hyphens allowed.
  std::string id;
  // Optional default properties to update with this call.
  // If not set, properties are left as they are on the server.
  std::map<std::string,std::string> default_properties;
  // Optional custom properties to update with this call.
  // If not set, properties are left as they are on the server.
  std::map<std::string,std::string> custom_properties;
};

// A single event. Usually, but not necessarily, part of a batch.
struct Event {
  // Event name.
  std::string name;
  // Optional event ID assigned by the client, used to de-duplicate in retransmission scenarios.
  // If not supplied the server will assign a randomly generated unique event identifier.
  std::string id;
  // Event metadata, if any.
  std::map<std::string,std::string> metadata;
  // Optional value.
  std::string value;
  // The time when the event was triggered on the producer side.
  time_t timestamp;
};

// Publish an event to the server
struct EventRequest {
  // Some number of events produced by a client.
  std::vector<Event> events;
};

// An experiment that this user is partaking.
struct Experiment {
  // Experiment name
  std::string name;
  // Value associated with this Experiment.
  std::string value;
};

// All experiments that this identity is involved with.
struct ExperimentList {
  // All experiments for this identity.
  std::vector<Experiment> experiments;
};

// Feature flag available to the identity.
struct Flag {
  // Flag name
  std::string name;
  // Value associated with this flag.
  std::string value;
  // Whether the value for this flag has conditionally changed from the default state.
  bool condition_changed;
};

// All flags available to the identity
struct FlagList {
  // All flags
  std::vector<Flag> flags;
};

// Request to get all experiments data.
struct GetExperimentsRequest {
  // Experiment names; if empty string all experiments are returned.
  std::vector<std::string> names;
};

// Request to get all flags data.
struct GetFlagsRequest {
  // Flag names; if empty string all flags are returned.
  std::vector<std::string> names;
};

// Request to get all live events.
struct GetLiveEventsRequest {
  // Live event names; if empty string all live events are returned.
  std::vector<std::string> names;
};

// Enrich/replace the current session with a new ID.
struct IdentifyRequest {
  // Identity ID to enrich the current session and return a new session. Old session will no longer be usable.
  std::string id;
  // Optional default properties to update with this call.
  // If not set, properties are left as they are on the server.
  std::map<std::string,std::string> default_properties;
  // Optional custom properties to update with this call.
  // If not set, properties are left as they are on the server.
  std::map<std::string,std::string> custom_properties;
};

// A single live event.
struct LiveEvent {
  // Name.
  std::string name;
  // Description.
  std::string description;
  // Event value.
  std::string value;
  // Start time of current event run.
  int64_t active_start_time_sec;
  // End time of current event run.
  int64_t active_end_time_sec;
  // The live event identifier.
  std::string id;
  // Start time.
  int64_t start_time_sec;
  // End time, 0 if it repeats forever.
  int64_t end_time_sec;
  // Duration in seconds.
  int64_t duration_sec;
  // Reset CRON schedule, if configured.
  std::string reset_cron;
};

// List of Live events.
struct LiveEventList {
  // Live events.
  std::vector<LiveEvent> live_events;
};

// Properties associated with an identity.
struct Properties {
  // Event default properties.
  std::map<std::string,std::string> default_properties;
  // Event computed properties.
  std::map<std::string,std::string> computed_properties;
  // Event custom properties.
  std::map<std::string,std::string> custom_properties;
};

// A session.
struct Session {
  // Token credential.
  std::string token;
  // Refresh token.
  std::string refresh_token;
  // Properties associated with this identity.
  Properties properties;
};

// Update Properties associated with this identity.
struct UpdatePropertiesRequest {
  // Event default properties.
  std::map<std::string,std::string> default_properties;
  // Event custom properties.
  std::map<std::string,std::string> custom_properties;
  // Informs the server to recompute the audience membership of the identity.
  bool recompute;
};


struct GetMessageListRequest {
  // Max number of messages to return. Between 1 and 100.
  int32_t limit;
  // True if listing should be older messages to newer, false if reverse.
  bool forward;
  // A pagination cursor, if any.
  std::string cursor;
};

// A scheduled message.
struct Message {
  // The identifier of the schedule.
  std::string schedule_id;
  // The send time for the message.
  int64_t send_time;
  // A key-value pairs of metadata.
  std::map<std::string, std::string> metadata;
  // The time the message was created.
  int64_t create_time;
  // The time the message was updated.
  int64_t update_time;
  // The time the message was read by the client.
  int64_t read_time;
  // The time the message was consumed by the identity.
  int64_t consume_time;
  // The message's text.
  std::string text;
  // The message's unique identifier.
  std::string id;
  // The message's title.
  std::string title;
  // The message's image url.
  std::string image_url;
};

// A response containing all the messages for an identity.
struct GetMessageListResponse {
  // The list of messages.
  std::vector<Message> messages;
  // The cursor to send when retrieving the next page, if any.
  std::string next_cursor;
  // The cursor to send when retrieving the previous page, if any.
  std::string prev_cursor;
  // Cacheable cursor to list newer messages. Durable and designed to be stored, unlike next/prev cursors.
  std::string cacheable_cursor;
};

// The request to update the status of a message.
struct UpdateMessageRequest {
  // The identifier of the messages.
  std::string id;
  // The time the message was read at the client.
  int64_t read_time;
  // The time the message was consumed by the identity.
  int64_t consume_time;
};

// The request to delete a scheduled message.
struct DeleteMessageRequest {
  // The identifier of the message.
  std::string id;
};

class HardcodedLowLevelSatoriAPI {
  GetLiveEventsRequest liveEventsRequest;
/*
    // List available live events.
    rpc GetLiveEvents (GetLiveEventsRequest) returns (LiveEventList) {
        option (google.api.http).get = "/v1/live-event";
    }
 * */
};

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
