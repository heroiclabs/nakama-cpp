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
#include "nakama-cpp/StrUtil.h"
#include "nakama-cpp/NUtils.h"

namespace Nakama {

using namespace std;

DefaultSession::DefaultSession(const std::string & token, bool created)
    : _token(token)
    , _created(created)
{
    _create_time = getUnixTimestampMs();

    // Hack decode JSON payload from JWT
    // first we narrow down to the segment between the first two '.'
    int dotIndex1 = token.find('.');
    int dotIndex2 = token.find('.', dotIndex1 + 1);
    std::string payload = token.substr(dotIndex1 + 1, dotIndex2 - dotIndex1 - 1);

    // the segment is base64 encoded, so decode it...
    std::string json = base64_decode(payload);

    // now we have some json to parse.
    // e.g.: {"exp":1489862293,"uid":"3c01e3ee-878a-4ec4-8923-40d51a86f91f"}
    string exp_str = getJsonFieldValue(json, "exp");
    if (!exp_str.empty())
    {
        _expire_time = ((NTimestamp)std::atol(exp_str.c_str())) * 1000ULL;
    }

    _username = getJsonFieldValue(json, "usn");
    _user_id = getJsonFieldValue(json, "uid");
}

const std::string & DefaultSession::getAuthToken() const
{
    return _token;
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

NSessionPtr restoreSession(const std::string& token)
{
    return NSessionPtr(new DefaultSession(token, false));
}

}
