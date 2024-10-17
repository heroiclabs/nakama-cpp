//
// Created by Ricard Rovira Cubeles on 17/10/24.
//

#include "nakama-cpp/satori/HardcodedLowLevelSatoriAPI.h"
#include "rapidjson/document.h"

namespace Satori{
	bool SLiveEvent::jsonToType(std::string jsonString, SFromJsonInterface *result) {
		rapidjson::Document d;
		d.Parse(jsonString.c_str());

		return false;
	}

	bool SLiveEventList::jsonToType(std::string jsonString, SFromJsonInterface *result) {
		rapidjson::Document d;
		d.Parse(jsonString.c_str());

		return false;
	}
}