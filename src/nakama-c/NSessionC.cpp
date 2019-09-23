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

#include "nakama-c/NSession.h"
#include "nakama-cpp/NSessionInterface.h"

NAKAMA_NAMESPACE_BEGIN

static std::vector<Nakama::NSessionPtr> g_sessions;

::NStringMap saveNStringMap(const NStringMap& map);

void saveSession(NSessionPtr session)
{
    g_sessions.emplace_back(std::move(session));
}

NSessionPtr getSession(NSession session)
{
    using namespace Nakama;

    for (auto it = g_sessions.begin(); it != g_sessions.end(); ++it)
    {
        if (it->get() == session)
        {
            return *it;
        }
    }

    return nullptr;
}

NAKAMA_NAMESPACE_END

extern "C" {

const char* NSession_getAuthToken(NSession session)
{
    Nakama::NSessionInterface* cppSession = (Nakama::NSessionInterface*)session;

    return cppSession->getAuthToken().c_str();
}

bool NSession_isCreated(NSession session)
{
    Nakama::NSessionInterface* cppSession = (Nakama::NSessionInterface*)session;
    return cppSession->isCreated();
}

const char* NSession_getUsername(NSession session)
{
    Nakama::NSessionInterface* cppSession = (Nakama::NSessionInterface*)session;
    return cppSession->getUsername().c_str();
}

const char* NSession_getUserId(NSession session)
{
    Nakama::NSessionInterface* cppSession = (Nakama::NSessionInterface*)session;
    return cppSession->getUserId().c_str();
}

NTimestamp NSession_getCreateTime(NSession session)
{
    Nakama::NSessionInterface* cppSession = (Nakama::NSessionInterface*)session;
    return cppSession->getCreateTime();
}

NTimestamp NSession_getExpireTime(NSession session)
{
    Nakama::NSessionInterface* cppSession = (Nakama::NSessionInterface*)session;
    return cppSession->getExpireTime();
}

bool NSession_isExpired(NSession session)
{
    Nakama::NSessionInterface* cppSession = (Nakama::NSessionInterface*)session;
    return cppSession->isExpired();
}

bool NSession_isExpiredByTime(NSession session, NTimestamp now)
{
    Nakama::NSessionInterface* cppSession = (Nakama::NSessionInterface*)session;
    return cppSession->isExpired(now);
}

NStringMap NSession_getVariables(NSession session)
{
    Nakama::NSessionInterface* cppSession = (Nakama::NSessionInterface*)session;
    ::NStringMap map = Nakama::saveNStringMap(cppSession->getVariables());
    return map;
}

const char* NSession_getVariable(NSession session, const char* name)
{
    Nakama::NSessionInterface* cppSession = (Nakama::NSessionInterface*)session;
    static std::string valueCache = cppSession->getVariable(name);
    return valueCache.c_str();
}

NSession restoreNakamaSession(const char* token)
{
    auto session = Nakama::restoreSession(token);
    Nakama::saveSession(session);
    return session.get();
}

void NSession_destroy(NSession session)
{
    using namespace Nakama;

    for (auto it = g_sessions.begin(); it != g_sessions.end(); ++it)
    {
        if (it->get() == session)
        {
            g_sessions.erase(it);
            break;
        }
    }
}

} // extern "C"
