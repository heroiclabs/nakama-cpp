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

	bool SAuthenticateLogoutRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SAuthenticateRefreshRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SAuthenticateRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SEvent::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SEventRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SExperiment::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SExperimentList::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

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

	bool SFlag::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {
		std::shared_ptr<SFlag> typedResult = std::dynamic_pointer_cast<SFlag>(result);
		if(typedResult == nullptr) {
			NLOG_ERROR(Nakama::NError("Incorrect empty result type passed to SFlag::jsonToType"));
			return false;
		}
		rapidjson::Document d;
		if(d.ParseInsitu(jsonString.data()).HasParseError()) {
			NLOG_ERROR(Nakama::NError("Parse SFlag JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " + std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.", Nakama::ErrorCode::InternalError));
			return false;
		}
		return jsonValueToSFlag(d, *typedResult);
	}

	bool SFlagList::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {
		std::shared_ptr<SFlagList> typedResult = std::dynamic_pointer_cast<SFlagList>(result);
		if(typedResult == nullptr) {
			NLOG_ERROR(Nakama::NError("Incorrect empty result type passed to SFlagList::jsonToType"));
			return false;
		}
		rapidjson::Document d;
		if(d.ParseInsitu(jsonString.data()).HasParseError()) {
			NLOG_ERROR(Nakama::NError("Parse SFlagList JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " + std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.", Nakama::ErrorCode::InternalError));
			return false;
		}

		// TODO: Error handling! Now if some field is not as expected, it just crashes. Example at: https://github.com/Tencent/rapidjson/blob/master/example/tutorial/tutorial.cpp
		for (auto& jsonFlag : d["flags"].GetArray()) {
			SFlag flag;
			if(!jsonValueToSFlag(jsonFlag, flag)) {
				return false;
			}
			typedResult->flags.emplace_back(flag);
		}
		return true;
	}

	bool SGetExperimentsRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SGetFlagsRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SGetLiveEventsRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SIdentifyRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SLiveEvent::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {
		rapidjson::Document d;
		d.Parse(jsonString.c_str());

		return false;
	}

	bool SLiveEventList::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {
		rapidjson::Document d;
		d.Parse(jsonString.c_str());

		return false;
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

	bool SProperties::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {
		std::shared_ptr<SProperties> typedResult = std::dynamic_pointer_cast<SProperties>(result);
		if(typedResult == nullptr) {
			NLOG_ERROR(Nakama::NError("Incorrect empty result type passed to SProperties::jsonToType"));
			return false;
		}
		rapidjson::Document d;
		if(d.ParseInsitu(jsonString.data()).HasParseError()) {
			NLOG_ERROR(Nakama::NError("Parse SProperties JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " + std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.", Nakama::ErrorCode::InternalError));
			return false;
		}
		return jsonValueToSProperties(d, *typedResult);
	}

	bool SSession::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {
		std::shared_ptr<SSession> typedResult = std::dynamic_pointer_cast<SSession>(result);
		if(typedResult == nullptr) {
			NLOG_ERROR(Nakama::NError("Incorrect empty result type passed to SSession::jsonToType"));
			return false;
		}
		rapidjson::Document d;
		if(d.ParseInsitu(jsonString.data()).HasParseError()) {
			NLOG_ERROR(Nakama::NError("Parse SSession JSON failed. Error at " + std::to_string(d.GetErrorOffset()) + ": " + std::string(GetParseError_En(d.GetParseError())) + " HTTP body:<< " + jsonString + " >>.", Nakama::ErrorCode::InternalError));
			return false;
		}
		// TODO: Error handling! Now if some field is not as expected, it just crashes. Example at: https://github.com/Tencent/rapidjson/blob/master/example/tutorial/tutorial.cpp
		typedResult->token = d["token"].GetString();
		typedResult->refresh_token = d["refresh_token"].GetString();
		return jsonValueToSProperties(d["properties"], typedResult->properties);
	}

	bool SUpdatePropertiesRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SGetMessageListRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SMessage::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SGetMessageListResponse::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SUpdateMessageRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}

	bool SDeleteMessageRequest::jsonToType(std::string jsonString, std::shared_ptr<SFromJsonInterface> result) {

		return false;
	}
}
