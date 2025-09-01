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

#include "InternalLowLevelSatoriAPI.h"

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "nakama-cpp/NError.h"
#include "nakama-cpp/log/NLogger.h"
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

namespace Satori {
bool jsonValueToStringVector(const rapidjson::Value& input, std::vector<std::string>& output) {
  for (rapidjson::Value::ConstMemberIterator iter = input.MemberBegin(); iter != input.MemberEnd(); ++iter) {
    output.emplace_back(iter->value.GetString());
  }
  return true;
}

bool jsonValueToStringMap(const rapidjson::Value& input, std::unordered_map<std::string, std::string>& output) {
  for (rapidjson::Value::ConstMemberIterator iter = input.MemberBegin(); iter != input.MemberEnd(); ++iter) {
    output[iter->name.GetString()] = iter->value.GetString();
  }
  return true;
}

bool SInternalAuthenticateLogoutRequest::fromJson(std::string jsonString) { return false; }

bool SInternalAuthenticateRefreshRequest::fromJson(std::string jsonString) { return false; }

bool SInternalAuthenticateRequest::fromJson(std::string jsonString) { return false; }

bool jsonValueToSEvent(const rapidjson::Value& input, SEvent& output) {
  if (input.HasMember("name")) {
    if (!input["name"].IsString()) {
      return false;
    }
    output.name = input["name"].GetString();
  }
  if (input.HasMember("id")) {
    if (!input["id"].IsString()) {
      return false;
    }
    output.id = input["id"].GetString();
  }
  if (input.HasMember("metadata") && !jsonValueToStringMap(input["metadata"], output.metadata)) {
    return false;
  }
  if (input.HasMember("value")) {
    if (!input["value"].IsString()) {
      return false;
    }
    output.value = input["value"].GetString();
  }
  if (input.HasMember("timestamp")) {
    if (!input["timestamp"].IsInt64()) {
      return false;
    }
    output.timestamp = input["timestamp"].GetInt64();
  }
  if (input.HasMember("identity_id")) {
    if (!input["identity_id"].IsString()) {
      return false;
    }
    output.identity_id = input["identity_id"].GetString();
  }
  if (input.HasMember("session_id")) {
    if (!input["session_id"].IsString()) {
      return false;
    }
    output.session_id = input["session_id"].GetString();
  }
  if (input.HasMember("session_issued_at")) {
    if (!input["session_issued_at"].IsInt64()) {
      return false;
    }
    output.session_issued_at = input["session_issued_at"].GetInt64();
  }
  if (input.HasMember("session_expires_at")) {
    if (!input["session_expires_at"].IsInt64()) {
      return false;
    }
    output.session_expires_at = input["session_expires_at"].GetInt64();
  }
  return true;
}

bool SInternalEvent::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SEvent JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  return jsonValueToSEvent(d, *this);
}

bool SInternalEventRequest::fromJson(std::string jsonString) { return false; }

bool jsonValueToSExperiment(const rapidjson::Value& input, SExperiment& output) {
  if (input.HasMember("name")) {
    if (!input["name"].IsString()) {
      return false;
    }
    output.name = input["name"].GetString();
  }
  if (input.HasMember("value")) {
    if (!input["value"].IsString()) {
      return false;
    }
    output.value = input["value"].GetString();
  }
  return true;
}

bool SInternalExperiment::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SExperiment JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  return jsonValueToSExperiment(d, *this);
}

bool SInternalExperimentList::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SExperimentList JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }

  if (d.HasMember("experiments")) {
    if (!d["experiments"].IsArray()) {
      return false;
    }
    for (auto& jsonExperiment : d["experiments"].GetArray()) {
      SExperiment experiment;
      if (!jsonValueToSExperiment(jsonExperiment, experiment)) {
        return false;
      }
      this->experiments.emplace_back(experiment);
    }
  }
  return true;
}

bool jsonValueToSFlag(const rapidjson::Value& input, SFlag& output) {
  if (input.HasMember("name")) {
    if (!input["name"].IsString()) {
      return false;
    }
    output.name = input["name"].GetString();
  }
  if (input.HasMember("value")) {
    if (!input["value"].IsString()) {
      return false;
    }
    output.value = input["value"].GetString();
  }
  if (input.HasMember("condition_changed")) {
    if (!input["condition_changed"].IsBool()) {
      return false;
    }
    output.condition_changed = input["condition_changed"].GetBool();
  }
  if (input.HasMember("change_reason")) {
    if (!input["change_reason"].IsObject()) {
      return false;
    }
    const auto change_object = input["change_reason"].GetObject();
    SFlag::SValueChangeReason change_reason;
    if (change_object.HasMember("name")) {
      if (!change_object["name"].IsString()) {
        return false;
      }
      change_reason.name = change_object["name"].GetString();
    }
    if (change_object.HasMember("variant_name")) {
      if (!change_object["variant_name"].IsString()) {
        return false;
      }
      change_reason.variant_name = change_object["variant_name"].GetString();
    }
    if (change_object.HasMember("type")) {
      if (!change_object["type"].IsInt()) {
        return false;
      }
      int type_int = change_object["type"].GetInt();
      if (type_int < SFlag::SValueChangeReason::SType::UNKNOWN ||
          type_int > SFlag::SValueChangeReason::SType::EXPERIMENT) {
        return false;
      }
      change_reason.type = static_cast<SFlag::SValueChangeReason::SType>(type_int);
    }
    output.change_reason = change_reason;
  }
  return true;
}

bool SInternalFlag::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SFlag JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  return jsonValueToSFlag(d, *this);
}

bool SInternalFlagList::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SFlagList JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  if (d.HasMember("flags")) {
    if (!d["flags"].IsArray()) {
      return false;
    }
    for (auto& jsonFlag : d["flags"].GetArray()) {
      SFlag flag;
      if (!jsonValueToSFlag(jsonFlag, flag)) {
        return false;
      }
      this->flags.emplace_back(flag);
    }
  }

  return true;
}

bool jsonValueToSFlagOverride(const rapidjson::Value& input, SFlagOverride& output) {
  if (!input.HasMember("flag_name") || !input["flag_name"].IsString()) {
    return false;
  }
  output.flag_name = input["flag_name"].GetString();
  if (!input.HasMember("overrides") || !input["overrides"].IsArray()) {
    return false;
  }
  for (auto& jsonFlagOverride : input["overrides"].GetArray()) {
    SFlagOverride::SValue value;
    if (!jsonFlagOverride.HasMember("name") || !jsonFlagOverride["name"].IsString()) {
      return false;
    }
    value.name = jsonFlagOverride["name"].GetString();
    if (!jsonFlagOverride.HasMember("value") || !jsonFlagOverride["value"].IsString()) {
      return false;
    }
    value.value = jsonFlagOverride["value"].GetString();
    if (jsonFlagOverride.HasMember("variant_name")) {
      if (!jsonFlagOverride["variant_name"].IsString()) {
        return false;
      }
      value.variant_name = jsonFlagOverride["variant_name"].GetString();
    }
    if (jsonFlagOverride.HasMember("type")) {
      if (!jsonFlagOverride["type"].IsInt()) {
        return false;
      }
      int type_int = jsonFlagOverride["type"].GetInt();
      if (type_int < SFlagOverride::SType::FLAG || type_int > SFlagOverride::SType::EXPERIMENT_PHASE_VARIANT_FLAG) {
        return false;
      }
      value.type = static_cast<SFlagOverride::SType>(type_int);
    }
    if (jsonFlagOverride.HasMember("create_time_sec")) {
      if (!jsonFlagOverride["create_time_sec"].IsInt64()) {
        return false;
      }
      value.create_time_sec = jsonFlagOverride["create_time_sec"].GetInt64();
    }
    output.overrides.emplace_back(value);
  }
  return true;
}

bool SInternalFlagOverride::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SFlagOverride JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  return jsonValueToSFlagOverride(d, *this);
}

bool SInternalFlagOverrideList::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SFlagOverrideList JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  if (d.HasMember("flags")) {
    if (!d["flags"].IsArray()) {
      return false;
    }
    for (auto& jsonFlagOverride : d["flags"].GetArray()) {
      SFlagOverride flagOverride;
      if (!jsonValueToSFlagOverride(jsonFlagOverride, flagOverride)) {
        return false;
      }
      this->flags.emplace_back(flagOverride);
    }
  }

  return true;
}

bool SInternalGetExperimentsRequest::fromJson(std::string jsonString) { return false; }

bool SInternalGetFlagsRequest::fromJson(std::string jsonString) { return false; }

bool SInternalGetLiveEventsRequest::fromJson(std::string jsonString) { return false; }

bool SInternalJoinLiveEventRequest::fromJson(std::string jsonString) { return false; }

bool SInternalIdentifyRequest::fromJson(std::string jsonString) { return false; }

bool jsonValueToSLiveEvent(const rapidjson::Value& input, SLiveEvent& output) {
  if (input.HasMember("name")) {
    if (!input["name"].IsString()) {
      return false;
    }
    output.name = input["name"].GetString();
  }
  if (input.HasMember("description")) {
    if (!input["description"].IsString()) {
      return false;
    }
    output.description = input["description"].GetString();
  }
  if (input.HasMember("value")) {
    if (!input["value"].IsString()) {
      return false;
    }
    output.value = input["value"].GetString();
  }
  if (input.HasMember("active_start_time_sec")) {
    if (!input["active_start_time_sec"].IsInt64()) {
      return false;
    }
    output.active_start_time_sec = input["active_start_time_sec"].GetInt64();
  }
  if (input.HasMember("active_end_time_sec")) {
    if (!input["active_end_time_sec"].IsInt64()) {
      return false;
    }
    output.active_end_time_sec = input["active_end_time_sec"].GetInt64();
  }
  if (input.HasMember("id")) {
    if (!input["id"].IsString()) {
      return false;
    }
    output.id = input["id"].GetString();
  }
  if (input.HasMember("start_time_sec")) {
    if (!input["start_time_sec"].IsInt64()) {
      return false;
    }
    output.start_time_sec = input["start_time_sec"].GetInt64();
  }
  if (input.HasMember("end_time_sec")) {
    if (!input["end_time_sec"].IsInt64()) {
      return false;
    }
    output.end_time_sec = input["end_time_sec"].GetInt64();
  }
  if (input.HasMember("duration_sec")) {
    if (!input["duration_sec"].IsInt64()) {
      return false;
    }
    output.duration_sec = input["duration_sec"].GetInt64();
  }
  if (input.HasMember("reset_cron")) {
    if (!input["reset_cron"].IsString()) {
      return false;
    }
    output.reset_cron = input["reset_cron"].GetString();
  }
  if (input.HasMember("status")) {
    if (!input["status"].IsInt()) {
      return false;
    }
    int type_int = input["status"].GetInt();
    if (type_int < SLiveEvent::SStatus::UNKNOWN || type_int > SLiveEvent::SStatus::TERMINATED) {
      return false;
    }
    output.status = static_cast<SLiveEvent::SStatus>(type_int);
  }
  return true;
}

bool SInternalLiveEvent::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SLiveEvent JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  return jsonValueToSLiveEvent(d, *this);
}

bool SInternalLiveEventList::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SLiveEventList JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }

  if (d.HasMember("live_events")) {
    if (!d["live_events"].IsArray()) {
      return false;
    }
    for (auto& jsonLiveEvent : d["live_events"].GetArray()) {
      SLiveEvent liveEvent;
      if (!jsonValueToSLiveEvent(jsonLiveEvent, liveEvent)) {
        return false;
      }
      this->live_events.emplace_back(liveEvent);
    }
  }
  if (d.HasMember("explicit_join_live_events")) {
    if (!d["explicit_join_live_events"].IsArray()) {
      return false;
    }
    for (auto& jsonExplicitLiveEvent : d["explicit_join_live_events"].GetArray()) {
      SLiveEvent explicitLiveEvent;
      if (!jsonValueToSLiveEvent(jsonExplicitLiveEvent, explicitLiveEvent)) {
        return false;
      }
      this->explicit_join_live_events.emplace_back(explicitLiveEvent);
    }
  }
  return true;
}

bool jsonValueToSProperties(const rapidjson::Value& input, SProperties& output) {
  if (input.HasMember("default") && !jsonValueToStringMap(input["default"], output.default_properties)) {
    return false;
  }
  if (input.HasMember("computed") && !jsonValueToStringMap(input["computed"], output.computed_properties)) {
    return false;
  }
  if (input.HasMember("custom") && !jsonValueToStringMap(input["custom"], output.custom_properties)) {
    return false;
  }
  return true;
}

bool SInternalProperties::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SProperties JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  return jsonValueToSProperties(d, *this);
}

bool SInternalSession::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SSession JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  if (d.HasMember("token")) {
    if (!d["token"].IsString()) {
      return false;
    }
    this->token = d["token"].GetString();
  }
  if (d.HasMember("refresh_token")) {
    if (!d["refresh_token"].IsString()) {
      return false;
    }
    this->refresh_token = d["refresh_token"].GetString();
  }
  return jsonValueToSProperties(d["properties"], this->properties);
}

bool SInternalUpdatePropertiesRequest::fromJson(std::string jsonString) { return false; }

bool SInternalGetMessageListRequest::fromJson(std::string jsonString) { return false; }

bool jsonValueToSMessage(const rapidjson::Value& input, SMessage& output) {
  if (input.HasMember("schedule_id")) {
    if (!input["schedule_id"].IsString()) {
      return false;
    }
    output.schedule_id = input["schedule_id"].GetString();
  }
  if (input.HasMember("send_time")) {
    if (!input["send_time"].IsString()) {
      return false;
    }
    std::string time_string = input["send_time"].GetString();
    size_t string_length = input["send_time"].GetStringLength();
    output.send_time = std::stoull(time_string, &string_length);
  } else {
    output.send_time = 0;
  }
  if (input.HasMember("metadata") && !jsonValueToStringMap(input["metadata"], output.metadata)) {
    return false;
  }
  if (input.HasMember("create_time")) {
    if (!input["create_time"].IsString()) {
      return false;
    }
    std::string time_string = input["create_time"].GetString();
    size_t string_length = input["create_time"].GetStringLength();
    output.create_time = std::stoull(time_string, &string_length);
  } else {
    output.create_time = 0;
  }
  if (input.HasMember("update_time")) {
    if (!input["update_time"].IsString()) {
      return false;
    }
    std::string time_string = input["update_time"].GetString();
    size_t string_length = input["update_time"].GetStringLength();
    output.update_time = std::stoull(time_string, &string_length);
  } else {
    output.update_time = 0;
  }
  if (input.HasMember("read_time")) {
    if (!input["read_time"].IsString()) {
      return false;
    }
    std::string time_string = input["read_time"].GetString();
    size_t string_length = input["read_time"].GetStringLength();
    output.read_time = std::stoull(time_string, &string_length);
  } else {
    output.read_time = 0;
  }
  if (input.HasMember("consume_time")) {
    if (!input["consume_time"].IsString()) {
      return false;
    }
    std::string time_string = input["consume_time"].GetString();
    size_t string_length = input["consume_time"].GetStringLength();
    output.consume_time = std::stoull(time_string, &string_length);
  } else {
    output.consume_time = 0;
  }
  if (input.HasMember("text")) {
    if (!input["text"].IsString()) {
      return false;
    }
    output.text = input["text"].GetString();
  }
  if (input.HasMember("id")) {
    if (!input["id"].IsString()) {
      return false;
    }
    output.id = input["id"].GetString();
  }
  if (input.HasMember("title")) {
    if (!input["title"].IsString()) {
      return false;
    }
    output.title = input["title"].GetString();
  }
  if (input.HasMember("image_url")) {
    if (!input["image_url"].IsString()) {
      return false;
    }
    output.image_url = input["image_url"].GetString();
  }
  return true;
}

bool SInternalMessage::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SMessage JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  return jsonValueToSMessage(d, *this);
}

bool SInternalGetMessageListResponse::fromJson(std::string jsonString) {
  rapidjson::Document d;
  if (d.ParseInsitu(jsonString.data()).HasParseError()) {
    NLOG_ERROR(
        Nakama::NError(
            "Parse SGetMessageListResponse JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " +
                std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.",
            Nakama::ErrorCode::InternalError));
    return false;
  }
  if (d.HasMember("messages")) {
    if (!d["messages"].IsArray()) {
      return false;
    }
    for (auto& jsonMessage : d["messages"].GetArray()) {
      SMessage message;
      if (!jsonValueToSMessage(jsonMessage, message)) {
        return false;
      }
      this->messages.emplace_back(message);
    }
  }
  if (d.HasMember("next_cursor")) {
    if (!d["next_cursor"].IsString()) {
      return false;
    }
    this->next_cursor = d["next_cursor"].GetString();
  }
  if (d.HasMember("prev_cursor")) {
    if (!d["prev_cursor"].IsString()) {
      return false;
    }
    this->prev_cursor = d["prev_cursor"].GetString();
  }
  if (d.HasMember("cacheable_cursor")) {
    if (!d["cacheable_cursor"].IsString()) {
      return false;
    }
    this->cacheable_cursor = d["cacheable_cursor"].GetString();
  }
  return true;
}

bool SInternalUpdateMessageRequest::fromJson(std::string jsonString) { return false; }

bool SInternalDeleteMessageRequest::fromJson(std::string jsonString) { return false; }
} // namespace Satori
