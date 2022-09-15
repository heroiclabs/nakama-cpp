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

#include "DefaultSession.h"
#include "StrUtil.h"
#include "nakama-cpp/NUtils.h"
#include "nakama-cpp/log/NLogger.h"
#include "rapidjson/document.h"


#undef NMODULE_NAME
#define NMODULE_NAME "Nakama::DefaultSession"

namespace Nakama {

using namespace std;

DefaultSession::DefaultSession(const std::string& token, const std::string& refreshToken, bool created)
    : _token(token),
    _created(created),
    _refresh_token(refreshToken)
{
    _create_time = getUnixTimestampMs();

    std::string tokenJson = this->jwtUnpack(token);

    rapidjson::Document document;

    // now we have some json to parse.
    // e.g.: {"exp":1489862293,"uid":"3c01e3ee-878a-4ec4-8923-40d51a86f91f"}
    if (document.Parse(tokenJson).HasParseError())
    {
        NLOG_ERROR("Parse JSON failed for token.");
    }
    else
    {
        if (document.HasMember("exp"))
        {
            auto& jsonExp = document["exp"];
            if (jsonExp.IsNumber())
            {
                _expire_time = (jsonExp.GetUint64()) * 1000ULL;
            }
        }

        if (document.HasMember("usn"))
        {
            auto& jsonUsn = document["usn"];
            if (jsonUsn.IsString()) _username = jsonUsn.GetString();
        }

        if (document.HasMember("uid"))
        {
            auto& jsonUid = document["uid"];
            if (jsonUid.IsString()) _user_id = jsonUid.GetString();
        }

        if (document.HasMember("vrs"))
        {
            auto& jsonVrs = document["vrs"];
            if (jsonVrs.IsObject())
            {
                auto object = jsonVrs.GetObject();
                for (auto it = object.begin(); it != object.end(); ++it)
                {
                    if (it->value.IsString())
                        _variables.emplace(it->name.GetString(), it->value.GetString());
                    else
                    {
                        NLOG_WARN("Non-string value is ignored: " + string(it->name.GetString()));
                    }
                }
            }
        }


        // Check if empty in case server has not updated to use refresh tokens yet.
        if (!refreshToken.empty())
        {
            std::string refreshTokenJson = this->jwtUnpack(refreshToken);

            rapidjson::Document doc;

            if (doc.Parse(refreshTokenJson).HasParseError())
            {
                NLOG_ERROR("Parse JSON failed for refresh token.");
                return;
            }

            if (doc.HasMember("exp"))
            {
                auto& jsonExp = doc["exp"];
                if (jsonExp.IsNumber())
                {
                    _refresh_expire_time = (jsonExp.GetUint64()) * 1000ULL;
                }
            }
            else
            {
                NLOG_ERROR("Could not find expiry on refresh token.");
            }
        }
    }
}

const std::string & DefaultSession::getAuthToken() const
{
    return _token;
}

const std::string & DefaultSession::getRefreshToken() const
{
    return _refresh_token;
}

bool DefaultSession::isCreated() const
{
    return _created;
}

const std::string & DefaultSession::getUsername() const
{
    return _username;
}

const std::string & DefaultSession::getUserId() const
{
    return _user_id;
}

NTimestamp DefaultSession::getCreateTime() const
{
    return _create_time;
}

NTimestamp DefaultSession::getExpireTime() const
{
    return _expire_time;
}

bool DefaultSession::isExpired() const
{
    return isExpired(getUnixTimestampMs());
}

bool DefaultSession::isExpired(NTimestamp now) const
{
    return now >= _expire_time;
}

bool DefaultSession::isRefreshExpired() const
{
    return isRefreshExpired(getUnixTimestampMs());
}

bool DefaultSession::isRefreshExpired(NTimestamp now) const
{
    return now >= _refresh_expire_time;
}

const NStringMap& DefaultSession::getVariables() const
{
    return _variables;
}

std::string DefaultSession::getVariable(const std::string& name) const
{
    auto it = _variables.find(name);

    if (it != _variables.end())
        return it->second;

    return {};
}

NSessionPtr restoreSession(const std::string& token, const std::string& refreshToken)
{
    return NSessionPtr(new DefaultSession(token, refreshToken, false));
}

std::string DefaultSession::jwtUnpack(std::string token)
{
    // Hack decode JSON payload from JWT
    // first we narrow down to the segment between the first two '.'
    size_t dotIndex1 = token.find('.');
    if (dotIndex1 != string::npos)
    {
        ++dotIndex1;
        size_t dotIndex2 = token.find('.', dotIndex1);

        if (dotIndex2 != string::npos)
        {
            std::string payload = token.substr(dotIndex1, dotIndex2 - dotIndex1);

            // the segment is base64 encoded, so decode it...
            std::string json = base64DecodeUrl(payload);
            return json;
        }
    }

    NLOG_ERROR("Could not unpack JWT.");
    return "";
}

}
