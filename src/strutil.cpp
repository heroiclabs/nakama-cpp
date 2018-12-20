#include "nakama-cpp/strutil.h"
#include <google/protobuf/stubs/strutil.h>

namespace nakama {

std::string base64_encode(const Base64Buffer& buffer)
{
    std::string base64str;

    google::protobuf::Base64Escape((const unsigned char *)buffer.data(), buffer.size(), &base64str, true);

    return base64str;
}

Base64Buffer base64_decode(const std::string& base64str)
{
    Base64Buffer buffer;

    google::protobuf::Base64Unescape(base64str, &buffer);

    return buffer;
}

} // namespace nakama
