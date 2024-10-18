//
// Created by Ricard Rovira Cubeles on 17/10/24.
//

#include "nakama-cpp/NError.h"
#include "nakama-cpp/log/NLogger.h"
#include "nakama-cpp/satori/HardcodedLowLevelSatoriAPI.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

namespace Satori{
	bool jsonValueToStringMap(const rapidjson::Value& input, std::map<std::string, std::string>& output) {
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

	bool jsonValueToSProperty(const rapidjson::Value& input, SProperties& output){
		// TODO: Error handling! Now if some field is not as expected, it just crashes. https://github.com/Tencent/rapidjson/blob/master/example/tutorial/tutorial.cpp
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
		return jsonValueToSProperty(d, *typedResult);
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
		// TODO: Error handling! Now if some field is not as expected, it just crashes. https://github.com/Tencent/rapidjson/blob/master/example/tutorial/tutorial.cpp
		typedResult->token = d["token"].GetString();
		typedResult->refresh_token = d["refresh_token"].GetString();
		if(!jsonValueToSProperty(d["properties"], typedResult->properties)) {
			return false;
		}
		return true;
	}

	/*
	{
	"token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiJjYzlkYTQ3Ny1lYTg2LTRiYTMtYmQwMi0wZDIzMzE4N2M5ZjEiLCJpaWQiOiIxMTExMTExMS0xMTExLTAwMDAtMDAwMC0wMDAwMDAwMDAwMDAiLCJleHAiOjE3MjkzNzQ3ODgsImlhdCI6MTcyOTI0NTE4OCwiYXBpIjoiZGVmYXVsdCJ9.EuwJ-e140GdwTzCJY1B29JYQ_wblp4rV2qSxuKEcwbI",
	"refresh_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiJjYzlkYTQ3Ny1lYTg2LTRiYTMtYmQwMi0wZDIzMzE4N2M5ZjEiLCJpaWQiOiIxMTExMTExMS0xMTExLTAwMDAtMDAwMC0wMDAwMDAwMDAwMDAiLCJleHAiOjE3Mjk4NDk5ODgsImlhdCI6MTcyOTI0NTE4OH0.JwS-gu7Vb58KZT37SFM9lb9zoaD9o20BNTmhFoKgBpE",
	"properties": {
		"default": {
			"apiKeyName": "default",
			"city": "-",
			"countryCode": "",
			"firstOpen": "1729244142",
			"region": "",
			"state": "-",
			"subRegion": ""
		},
		"computed": {
			"_identityCreateCount": "1",
			"_identityCreateSeenFirst": "1729244142",
			"_identityCreateSeenLast": "1729244142",
			"_identityCreateValueFirst": "{
				\"default\":{
					\"apiKeyName\":\"default\",\"city\":\"-\",\"countryCode\":\"\",\"firstOpen\":\"1729244142\",\"region\":\"\",\"state\":\"-\",\"subRegion\":\"\"
				}
			}
			",
			"_identityCreateValueLast": "{
				\"default\":{
					\"apiKeyName\":\"default\",\"city\":\"-\",\"countryCode\":\"\",\"firstOpen\":\"1729244142\",\"region\":\"\",\"state\":\"-\",\"subRegion\":\"\"
				}
			}",
			"_sessionStartCount": "8", "_sessionStartSeenFirst": "1729244142", "_sessionStartSeenLast": "1729245145", "_sessionStartValueFirst": "0", "_sessionStartValueHigh": "0", "_sessionStartValueLast": "0", "_sessionStartValueLow": "0", "_sessionStartValueSum": "0", "seenLast": "1729245145"
		}
	}
}"

	 */
}
