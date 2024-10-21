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
#include <unordered_map>
#include <ctime>

namespace Satori {
    struct SFromJsonInterface {
        virtual ~SFromJsonInterface() {}
        virtual bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) = 0;
    };

    // Log out a session, invalidate a refresh token, or log out all sessions/refresh tokens for a user.
    struct SAuthenticateLogoutRequest : public SFromJsonInterface {
        // Session token to log out.
        std::string token;
        // Refresh token to invalidate.
        std::string refresh_token;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // Authenticate against the server with a refresh token.
    struct SAuthenticateRefreshRequest : public SFromJsonInterface {
        // Refresh token.
        std::string refresh_token;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // Authentication request
    struct SAuthenticateRequest : public SFromJsonInterface {
        // Identity ID. Must be between eight and 128 characters (inclusive).
        // Must be an alphanumeric string with only underscores and hyphens allowed.
        std::string id;
        // Optional default properties to update with this call.
        // If not set, properties are left as they are on the server.
        std::unordered_map<std::string,std::string> default_properties;
        // Optional custom properties to update with this call.
        // If not set, properties are left as they are on the server.
        std::unordered_map<std::string,std::string> custom_properties;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // A single event. Usually, but not necessarily, part of a batch.
    struct SEvent : public SFromJsonInterface {
        // Event name.
        std::string name;
        // Optional event ID assigned by the client, used to de-duplicate in retransmission scenarios.
        // If not supplied the server will assign a randomly generated unique event identifier.
        std::string id;
        // Event metadata, if any.
        std::unordered_map<std::string,std::string> metadata;
        // Optional value.
        std::string value;
        // The time when the event was triggered on the producer side.
        time_t timestamp;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // Publish an event to the server
    struct SEventRequest : public SFromJsonInterface {
        // Some number of events produced by a client.
        std::vector<SEvent> events;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // An experiment that this user is partaking.
    struct SExperiment : public SFromJsonInterface {
        // Experiment name
        std::string name;
        // Value associated with this Experiment.
        std::string value;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // All experiments that this identity is involved with.
    struct SExperimentList : public SFromJsonInterface {
        // All experiments for this identity.
        std::vector<SExperiment> experiments;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // Feature flag available to the identity.
    struct SFlag : public SFromJsonInterface {
        // Flag name
        std::string name;
        // Value associated with this flag.
        std::string value;
        // Whether the value for this flag has conditionally changed from the default state.
        bool condition_changed;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // All flags available to the identity
    struct SFlagList : public SFromJsonInterface {
        // All flags
        std::vector<SFlag> flags;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // Request to get all experiments data.
    struct SGetExperimentsRequest : public SFromJsonInterface {
        // Experiment names; if empty string all experiments are returned.
        std::vector<std::string> names;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // Request to get all flags data.
    struct SGetFlagsRequest : public SFromJsonInterface {
        // Flag names; if empty string all flags are returned.
        std::vector<std::string> names;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // Request to get all live events.
    struct SGetLiveEventsRequest : public SFromJsonInterface {
        // Live event names; if empty string all live events are returned.
        std::vector<std::string> names;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // Enrich/replace the current session with a new ID.
    struct SIdentifyRequest : public SFromJsonInterface {
        // Identity ID to enrich the current session and return a new session. Old session will no longer be usable.
        std::string id;
        // Optional default properties to update with this call.
        // If not set, properties are left as they are on the server.
        std::unordered_map<std::string,std::string> default_properties;
        // Optional custom properties to update with this call.
        // If not set, properties are left as they are on the server.
        std::unordered_map<std::string,std::string> custom_properties;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // A single live event.
    struct SLiveEvent : public SFromJsonInterface {
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

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // List of Live events.
    struct SLiveEventList : public SFromJsonInterface {
        // Live events.
        std::vector<SLiveEvent> live_events;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // Properties associated with an identity.
    struct SProperties : public SFromJsonInterface {
        // Event default properties.
        std::unordered_map<std::string,std::string> default_properties;
        // Event computed properties.
        std::unordered_map<std::string,std::string> computed_properties;
        // Event custom properties.
        std::unordered_map<std::string,std::string> custom_properties;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // A session.
    struct SSession : public SFromJsonInterface {
        // Token credential.
        std::string token;
        // Refresh token.
        std::string refresh_token;
        // Properties associated with this identity.
        SProperties properties;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // Update Properties associated with this identity.
    struct SUpdatePropertiesRequest : public SFromJsonInterface {
        // Event default properties.
        std::unordered_map<std::string,std::string> default_properties;
        // Event custom properties.
        std::unordered_map<std::string,std::string> custom_properties;
        // Informs the server to recompute the audience membership of the identity.
        bool recompute;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };


    struct SGetMessageListRequest : public SFromJsonInterface {
        // Max number of messages to return. Between 1 and 100.
        int32_t limit;
        // True if listing should be older messages to newer, false if reverse.
        bool forward;
        // A pagination cursor, if any.
        std::string cursor;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // A scheduled message.
    struct SMessage : public SFromJsonInterface {
        // The identifier of the schedule.
        std::string schedule_id;
        // The send time for the message.
        int64_t send_time;
        // A key-value pairs of metadata.
        std::unordered_map<std::string, std::string> metadata;
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

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // A response containing all the messages for an identity.
    struct SGetMessageListResponse : public SFromJsonInterface {
        // The list of messages.
        std::vector<SMessage> messages;
        // The cursor to send when retrieving the next page, if any.
        std::string next_cursor;
        // The cursor to send when retrieving the previous page, if any.
        std::string prev_cursor;
        // Cacheable cursor to list newer messages. Durable and designed to be stored, unlike next/prev cursors.
        std::string cacheable_cursor;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // The request to update the status of a message.
    struct SUpdateMessageRequest : public SFromJsonInterface {
        // The identifier of the messages.
        std::string id;
        // The time the message was read at the client.
        int64_t read_time;
        // The time the message was consumed by the identity.
        int64_t consume_time;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };

    // The request to delete a scheduled message.
    struct SDeleteMessageRequest : public SFromJsonInterface {
        // The identifier of the message.
        std::string id;

        bool jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) override;
    };
}
