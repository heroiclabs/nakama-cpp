#pragma once

#include <string>

namespace Nakama {

    typedef std::string Base64Buffer;

    std::string base64_encode(const Base64Buffer& buffer);

    Base64Buffer base64_decode(const std::string& base64str);

    std::string getJsonFieldValue(const std::string& json, const std::string& field_name);

} // namespace nakama
