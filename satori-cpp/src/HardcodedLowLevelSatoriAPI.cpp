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

#include "nakama-cpp/NError.h"
#include "nakama-cpp/log/NLogger.h"
#include "nakama-cpp/satori/HardcodedLowLevelSatoriAPI.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

namespace Satori{
	bool jsonValueToStringVector(const rapidjson::Value& input, std::vector<std::string>& output) {
		for (rapidjson::Value::ConstMemberIterator iter = input.MemberBegin(); iter != input.MemberEnd(); ++iter){
			output.emplace_back(iter->value.GetString());
		}
		return true;
	}

	bool jsonValueToStringMap(const rapidjson::Value& input, std::unordered_map<std::string, std::string>& output) {
		for (rapidjson::Value::ConstMemberIterator iter = input.MemberBegin(); iter != input.MemberEnd(); ++iter){
			output[iter->name.GetString()] = iter->value.GetString();
		}
		return true;
	}

	bool SAuthenticateLogoutRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool SAuthenticateRefreshRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool SAuthenticateRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool SEvent::fromJson(std::string jsonString) {

		return false;
	}

	bool SEventRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool SExperiment::fromJson(std::string jsonString) {

		return false;
	}

	bool SExperimentList::fromJson(std::string jsonString) {

		return false;
	}

	bool jsonValueToSFlag(const rapidjson::Value& input, SFlag& output){
		if(input.HasMember("name") && !input["name"].IsString()) {
			return false;
		}
		output.name = input["name"].GetString();
		if(input.HasMember("value") && !input["value"].IsString()) {
			return false;
		}
		output.value = input["value"].GetString();
		// TODO: Figure out how to obtain this value and set it here if it can be obtained from the json we have
		bool condition_changed = false;
		return true;
	}

	bool SFlag::fromJson(std::string jsonString) {
		rapidjson::Document d;
		if(d.ParseInsitu(jsonString.data()).HasParseError()) {
			NLOG_ERROR(Nakama::NError("Parse SFlag JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " + std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.", Nakama::ErrorCode::InternalError));
			return false;
		}
		return jsonValueToSFlag(d, *this);
	}

	bool SFlagList::fromJson(std::string jsonString) {
		rapidjson::Document d;
		if(d.ParseInsitu(jsonString.data()).HasParseError()) {
			NLOG_ERROR(Nakama::NError("Parse SFlagList JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " + std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.", Nakama::ErrorCode::InternalError));
			return false;
		}

		if(d.HasMember("flags")) {
			if(!d["flags"].IsArray()) {
				return false;
			}
			for (auto& jsonFlag : d["flags"].GetArray()) {
				SFlag flag;
				if(!jsonValueToSFlag(jsonFlag, flag)) {
					return false;
				}
				this->flags.emplace_back(flag);
			}
			return true;
		}
	}

	bool SGetExperimentsRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool SGetFlagsRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool SGetLiveEventsRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool SIdentifyRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool jsonValueToSLiveEvent(const rapidjson::Value& input, SLiveEvent& output){
		if(input.HasMember("name") && !input["name"].IsString()) {
			return false;
		}
		output.name = input["name"].GetString();
		if(input.HasMember("description") && !input["description"].IsString()) {
			return false;
		}
		output.description = input["description"].GetString();
		if(input.HasMember("value") && !input["value"].IsString()) {
			return false;
		}
		output.value = input["value"].GetString();
		if(input.HasMember("active_start_time_sec") && !input["active_start_time_sec"].IsInt64()) {
			return false;
		}
		output.active_start_time_sec = input["active_start_time_sec"].GetInt64();
		if(input.HasMember("active_end_time_sec") && !input["active_end_time_sec"].IsInt64()) {
			return false;
		}
		output.active_end_time_sec = input["active_end_time_sec"].GetInt64();
		if(input.HasMember("id") && !input["id"].IsString()) {
			return false;
		}
		output.id = input["id"].GetString();
		if(input.HasMember("start_time_sec") && !input["start_time_sec"].IsInt64()) {
			return false;
		}
		output.start_time_sec = input["start_time_sec"].GetInt64();
		if(input.HasMember("end_time_sec") && !input["end_time_sec"].IsInt64()) {
			return false;
		}
		output.end_time_sec = input["end_time_sec"].GetInt64();
		if(input.HasMember("duration_sec") && !input["duration_sec"].IsInt64()) {
			return false;
		}
		output.duration_sec = input["duration_sec"].GetInt64();
		if(input.HasMember("reset_cron") && !input["reset_cron"].IsString()) {
			return false;
		}
		output.reset_cron = input["reset_cron"].GetString();
		// TODO: Figure out how to obtain this value and set it here if it can be obtained from the json we have
		bool condition_changed = false;
		return true;
	}

	bool SLiveEvent::fromJson(std::string jsonString) {
		rapidjson::Document d;
		if(d.ParseInsitu(jsonString.data()).HasParseError()) {
			NLOG_ERROR(Nakama::NError("Parse SLiveEvent JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " + std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.", Nakama::ErrorCode::InternalError));
			return false;
		}
		return jsonValueToSLiveEvent(d, *this);
	}

	bool SLiveEventList::fromJson(std::string jsonString) {
		rapidjson::Document d;
		if(d.ParseInsitu(jsonString.data()).HasParseError()) {
			NLOG_ERROR(Nakama::NError("Parse SLiveEventList JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " + std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.", Nakama::ErrorCode::InternalError));
			return false;
		}

		if(d.HasMember("live_events")) {
			if(!d["live_events"].IsArray()) {
				return false;
			}
			for (auto& jsonLiveEvent : d["live_events"].GetArray()) {
				SLiveEvent liveEvent;
				if(!jsonValueToSLiveEvent(jsonLiveEvent, liveEvent)) {
					return false;
				}
				this->live_events.emplace_back(liveEvent);
			}
		}
		return true;
	}

	bool jsonValueToSProperties(const rapidjson::Value& input, SProperties& output){
		if(input.HasMember("default") && !jsonValueToStringMap(input["default"], output.default_properties)) {
			return false;
		}
		if(input.HasMember("computed") && !jsonValueToStringMap(input["computed"], output.computed_properties)) {
			return false;
		}
		if(input.HasMember("custom") && !jsonValueToStringMap(input["custom"], output.custom_properties)) {
			return false;
		}
		return true;
	}

	bool SProperties::fromJson(std::string jsonString) {
		rapidjson::Document d;
		if(d.ParseInsitu(jsonString.data()).HasParseError()) {
			NLOG_ERROR(Nakama::NError("Parse SProperties JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " + std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.", Nakama::ErrorCode::InternalError));
			return false;
		}
		return jsonValueToSProperties(d, *this);
	}

	bool SSession::fromJson(std::string jsonString) {
		rapidjson::Document d;
		if(d.ParseInsitu(jsonString.data()).HasParseError()) {
			NLOG_ERROR(Nakama::NError("Parse SSession JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " + std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.", Nakama::ErrorCode::InternalError));
			return false;
		}
		// TODO: Error handling! Now if some field is not as expected, it just crashes. Example at: https://github.com/Tencent/rapidjson/blob/master/example/tutorial/tutorial.cpp
		this->token = d["token"].GetString();
		this->refresh_token = d["refresh_token"].GetString();
		return jsonValueToSProperties(d["properties"], this->properties);
	}

	bool SUpdatePropertiesRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool SGetMessageListRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool SMessage::fromJson(std::string jsonString) {

		return false;
	}

	bool SGetMessageListResponse::fromJson(std::string jsonString) {

		return false;
	}

	bool SUpdateMessageRequest::fromJson(std::string jsonString) {

		return false;
	}

	bool SDeleteMessageRequest::fromJson(std::string jsonString) {

		return false;
	}
}
