/*
 * Copyright 2019 The Nakama Authors
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

string url_encode(const string & str)
{
    string result;
    char c;
    const char* chars = str.c_str();
    char bufHex[10];
    uint32_t len = str.size();

    for (uint32_t i = 0; i < len; i++)
    {
        c = chars[i];
        // uncomment this if you want to encode spaces with +
        /*if (c==' ') new_str += '+';
        else */if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            result += c;
        else
        {
            sprintf(bufHex, "%X", c);

            if ((uint8_t)c < 16)
                result += "%0";
            else
                result += "%";

            result += bufHex;
        }
    }
    return result;
}

} // namespace Nakama
