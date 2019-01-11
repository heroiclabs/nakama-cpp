#include "nakama-cpp/StrUtil.h"
#include <google/protobuf/stubs/strutil.h>

namespace Nakama {

using namespace std;

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

std::string getJsonFieldValue(const std::string & json, const std::string & field_name)
{
    // e.g. json: {"exp":1489862293,"uid":"3c01e3ee-878a-4ec4-8923-40d51a86f91f"}
    string result;

    auto pos = json.find("\"" + field_name + "\"");
    if (pos != string::npos)
    {
        pos += field_name.size() + 2;

        pos = json.find(":", pos);
        if (pos != string::npos)
        {
            ++pos;

            // skip spaces
            for (; pos < json.size() && json[pos] == ' '; ++pos) {}

            if (pos < json.size())
            {
                bool isString = (json[pos] == '\"');
                if (isString) ++pos;

                for (; pos < json.size(); ++pos)
                {
                    char c = json[pos];

                    if (isString)
                    {
                        if (c == '\"')
                        {
                            // end of field
                            break;
                        }
                    }
                    else if (c == ' ' || c == ',' || c == '}')
                    {
                        // end of field
                        break;
                    }

                    result.push_back(c);
                }
            }
        }
    }

    return result;
}

} // namespace nakama
