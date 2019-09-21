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

#include "nakama-c/NClient.h"
#include "nakama-c/DataHelperC.h"
#include "nakama-cpp/NClientInterface.h"


NAKAMA_NAMESPACE_BEGIN

void saveSession(NSessionPtr session);
NSessionPtr getSession(::NSession session);
NStringMap* findNStringMap(::NStringMap map);

Nakama::ErrorCallback createErrorCallback(NClient client, NClientReqData reqData, NClientErrorCallback errorCallback)
{
    if (!errorCallback)
        return nullptr;

    return [client, reqData, errorCallback](const Nakama::NError& error)
    {
        tNError cError;
        assign(cError, error);
        errorCallback(client, reqData, &cError);
    };
}

std::function<void()> createOkEmptyCallback(NClient client, NClientReqData reqData, NSuccessEmptyCallback okCallback)
{
    if (!okCallback)
        return nullptr;

    return [client, reqData, okCallback]()
    {
        okCallback(client, reqData);
    };
}

std::function<void(Nakama::NSessionPtr)> createAuthSuccessCallback(NClient client, NClientReqData reqData, NSessionCallback successCallback)
{
    return [client, reqData, successCallback](Nakama::NSessionPtr session)
    {
        if (successCallback)
        {
            saveSession(session);
            successCallback(client, reqData, session.get());
        }
    };
}

NAKAMA_NAMESPACE_END

extern "C" {

Nakama::NClientInterface* getCppClient(NClient client)
{
    return (Nakama::NClientInterface*)client;
}

void NClient_setErrorCallback(NClient client, NClientDefaultErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);

    if (errorCallback)
    {
        cppClient->setErrorCallback([client, errorCallback](const Nakama::NError& error)
        {
            tNError cError;
            assign(cError, error);
            errorCallback(client, &cError);
        });
    }
    else
    {
        cppClient->setErrorCallback(nullptr);
    }
}

void NClient_setUserData(NClient client, void* userData)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);

    cppClient->setUserData(userData);
}

void* NClient_getUserData(NClient client)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);

    return cppClient->getUserData();
}

void NClient_disconnect(NClient client)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);

    cppClient->disconnect();
}

void NClient_tick(NClient client)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);

    cppClient->tick();
}

void NClient_authenticateDevice(
    NClient client,
    const char* id,
    const char* username,
    bool create,
    NStringMap vars,
    NClientReqData reqData,
    NSessionCallback successCallback, NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    Nakama::opt::optional<std::string> usernameOpt;
    Nakama::NStringMap* cppVars = Nakama::findNStringMap(vars);

    if (username)
        usernameOpt = std::string(username);

    cppClient->authenticateDevice(
        id,
        usernameOpt,
        create,
        cppVars ? *cppVars : Nakama::NStringMap(),
        Nakama::createAuthSuccessCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_authenticateEmail(
    NClient client,
    const char* email,
    const char* password,
    const char* username,
    bool create,
    NStringMap vars,
    NClientReqData reqData,
    NSessionCallback successCallback, NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    Nakama::NStringMap* cppVars = Nakama::findNStringMap(vars);

    cppClient->authenticateEmail(
        email,
        password,
        username ? username : "",
        create,
        cppVars ? *cppVars : Nakama::NStringMap(),
        Nakama::createAuthSuccessCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_authenticateFacebook(
    NClient client,
    const char* accessToken,
    const char* username,
    bool create,
    bool importFriends,
    NStringMap vars,
    NClientReqData reqData,
    NSessionCallback successCallback, NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    Nakama::NStringMap* cppVars = Nakama::findNStringMap(vars);

    cppClient->authenticateFacebook(
        accessToken,
        username ? username : "",
        create,
        importFriends,
        cppVars ? *cppVars : Nakama::NStringMap(),
        Nakama::createAuthSuccessCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_authenticateGoogle(
    NClient client,
    const char* accessToken,
    const char* username,
    bool create,
    NStringMap vars,
    NClientReqData reqData,
    NSessionCallback successCallback, NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    Nakama::NStringMap* cppVars = Nakama::findNStringMap(vars);

    cppClient->authenticateGoogle(
        accessToken,
        username ? username : "",
        create,
        cppVars ? *cppVars : Nakama::NStringMap(),
        Nakama::createAuthSuccessCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_authenticateGameCenter(
    NClient client,
    const char* playerId,
    const char* bundleId,
    NTimestamp timestampSeconds,
    const char* salt,
    const char* signature,
    const char* publicKeyUrl,
    const char* username,
    bool create,
    NStringMap vars,
    NClientReqData reqData,
    NSessionCallback successCallback, NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    Nakama::NStringMap* cppVars = Nakama::findNStringMap(vars);

    cppClient->authenticateGameCenter(
        playerId,
        bundleId,
        timestampSeconds,
        salt,
        signature,
        publicKeyUrl,
        username ? username : "",
        create,
        cppVars ? *cppVars : Nakama::NStringMap(),
        Nakama::createAuthSuccessCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_authenticateCustom(
    NClient client,
    const char* id,
    const char* username,
    bool create,
    NStringMap vars,
    NClientReqData reqData,
    NSessionCallback successCallback, NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    Nakama::NStringMap* cppVars = Nakama::findNStringMap(vars);

    cppClient->authenticateCustom(
        id,
        username ? username : "",
        create,
        cppVars ? *cppVars : Nakama::NStringMap(),
        Nakama::createAuthSuccessCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_authenticateSteam(
    NClient client,
    const char* token,
    const char* username,
    bool create,
    NStringMap vars,
    NClientReqData reqData,
    NSessionCallback successCallback, NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    Nakama::NStringMap* cppVars = Nakama::findNStringMap(vars);

    cppClient->authenticateSteam(
        token,
        username ? username : "",
        create,
        cppVars ? *cppVars : Nakama::NStringMap(),
        Nakama::createAuthSuccessCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_linkFacebook(NClient client, NSession session, const char* accessToken, bool importFriends, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);

    cppClient->linkFacebook(
        cppSession,
        accessToken,
        importFriends,
        Nakama::createOkEmptyCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_linkEmail(NClient client, NSession session, const char* email, const char* password, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_linkDevice(NClient client, NSession session, const char* id, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_linkGoogle(NClient client, NSession session, const char* accessToken, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_linkGameCenter(NClient client, NSession session, const char* playerId, const char* bundleId, NTimestamp timestampSeconds, const char* salt, const char* signature, const char* publicKeyUrl, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_linkSteam(NClient client, NSession session, const char* token, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_linkCustom(NClient client, NSession session, const char* id, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_unlinkFacebook(NClient client, NSession session, const char* accessToken, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_unlinkEmail(NClient client, NSession session, const char* email, const char* password, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_unlinkGoogle(NClient client, NSession session, const char* accessToken, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_unlinkGameCenter(NClient client, NSession session, const char* playerId, const char* bundleId, NTimestamp timestampSeconds, const char* salt, const char* signature, const char* publicKeyUrl, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_unlinkSteam(NClient client, NSession session, const char* token, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_unlinkDevice(NClient client, NSession session, const char* id, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_unlinkCustom(NClient client, NSession session, const char* id, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_importFacebookFriends(NClient client, NSession session, const char* token, bool reset, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_getAccount(
    NClient client,
    NSession session,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNAccount*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);

    cppClient->getAccount(
        cppSession,
        [client, reqData, successCallback](const Nakama::NAccount& account)
        {
            if (successCallback)
            {
                sNAccount cAccount;
                assign(cAccount, account);
                successCallback(client, reqData, &cAccount);
                Nakama::sNAccount_free(cAccount);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_updateAccount(NClient client, NSession session, const char* username, const char* displayName, const char* avatarUrl, const char* langTag, const char* location, const char* timezone, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_getUsers(
    NClient client,
    NSession session,
    const char** ids,
    uint16_t idsCount,
    const char** usernames,
    uint16_t usernamesCount,
    const char** facebookIds,
    uint16_t facebookIdsCount,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNUsers*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    std::vector<std::string> cppIds, cppUsernames, cppFacebookIds;

    Nakama::assign(cppIds, ids, idsCount);
    Nakama::assign(cppUsernames, usernames, usernamesCount);
    Nakama::assign(cppFacebookIds, facebookIds, facebookIdsCount);

    cppClient->getUsers(
        cppSession,
        cppIds,
        cppUsernames,
        cppFacebookIds,
        [client, reqData, successCallback](const Nakama::NUsers& users)
        {
            if (successCallback)
            {
                sNUsers cUsers;
                assign(cUsers, users);
                successCallback(client, reqData, &cUsers);
                Nakama::sNUsers_free(cUsers);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_addFriends(
    NClient client,
    NSession session,
    const char** ids, uint16_t idsCount,
    const char** usernames, uint16_t usernamesCount,
    NClientReqData reqData,
    void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_deleteFriends(
    NClient client,
    NSession session,
    const char** ids, uint16_t idsCount,
    const char** usernames, uint16_t usernamesCount,
    NClientReqData reqData,
    void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_blockFriends(NClient client,
    NSession session,
    const char** ids, uint16_t idsCount,
    const char** usernames, uint16_t usernamesCount,
    NClientReqData reqData,
    void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_listFriends(
    NClient client,
    NSession session,
    const int32_t* limit,
    const eFriendState* state,
    const char* cursor,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNFriendList*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    Nakama::opt::optional<int32_t> cppLimit;
    Nakama::opt::optional<Nakama::NFriend::State> cppState;

    if (limit) cppLimit = *limit;
    if (state) cppState = (Nakama::NFriend::State)*state;

    cppClient->listFriends(
        cppSession,
        cppLimit,
        cppState,
        cursor ? cursor : "",
        [client, reqData, successCallback](Nakama::NFriendListPtr friends)
        {
            if (successCallback)
            {
                sNFriendList cFriends;
                assign(cFriends, *friends);
                successCallback(client, reqData, &cFriends);
                Nakama::sNFriendList_free(cFriends);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_createGroup(
    NClient client,
    NSession session,
    const char* name,
    const char* description,
    const char* avatarUrl,
    const char* langTag,
    bool open,
    const int32_t* maxCount,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNGroup*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    Nakama::opt::optional<int32_t> cppMaxCount;

    if (maxCount) cppMaxCount = *maxCount;

    cppClient->createGroup(
        cppSession,
        name,
        description,
        avatarUrl,
        langTag,
        open,
        cppMaxCount,
        [client, reqData, successCallback](const Nakama::NGroup& group)
        {
            if (successCallback)
            {
                sNGroup cGroup;
                assign(cGroup, group);
                successCallback(client, reqData, &cGroup);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_deleteGroup(NClient client, NSession session, const char* groupId, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_addGroupUsers(NClient client, NSession session, const char* groupId, const char** ids, uint16_t idsCount, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_listGroupUsers(
    NClient client,
    NSession session,
    const char* groupId,
    const int32_t* limit,
    const eFriendState* state,
    const char* cursor,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNGroupUserList*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    Nakama::opt::optional<int32_t> cppLimit;
    Nakama::opt::optional<Nakama::NFriend::State> cppState;

    if (limit) cppLimit = *limit;
    if (state) cppState = (Nakama::NFriend::State)(*state);

    cppClient->listGroupUsers(
        cppSession,
        groupId,
        cppLimit,
        cppState,
        cursor ? cursor : "",
        [client, reqData, successCallback](const Nakama::NGroupUserListPtr& groupUserList)
        {
            if (successCallback)
            {
                sNGroupUserList cGroupUserList;
                assign(cGroupUserList, *groupUserList);
                successCallback(client, reqData, &cGroupUserList);
                Nakama::sNGroupUserList_free(cGroupUserList);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_kickGroupUsers(NClient client, NSession session, const char* groupId, const char** ids, uint16_t idsCount, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_joinGroup(NClient client, NSession session, const char* groupId, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_leaveGroup(NClient client, NSession session, const char* groupId, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_listGroups(
    NClient client,
    NSession session,
    const char* name,
    const int32_t* limit,
    const char* cursor,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNGroupList*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);

    cppClient->listGroups(
        cppSession,
        name,
        limit ? *limit : 0,
        cursor ? cursor : "",
        [client, reqData, successCallback](const Nakama::NGroupListPtr& groupList)
        {
            if (successCallback)
            {
                sNGroupList cGroupList;
                assign(cGroupList, *groupList);
                successCallback(client, reqData, &cGroupList);
                Nakama::sNGroupList_free(cGroupList);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_listOwnUserGroups(NClient client, NSession session, NClientReqData reqData, void(*successCallback)(const sNUserGroupList*), NClientErrorCallback errorCallback)
{
    
}

void NClient_listUserGroups(
    NClient client,
    NSession session,
    const char* userId,
    const int32_t* limit,
    const eFriendState* state,
    const char* cursor,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNUserGroupList*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    std::string cppUserId(userId ? userId : cppSession->getUserId());
    std::string cppCursor(cursor ? cursor : "");
    Nakama::opt::optional<int32_t> cppLimit;
    Nakama::opt::optional<Nakama::NFriend::State> cppState;

    if (limit) cppLimit = *limit;
    if (state) cppState = (Nakama::NFriend::State)*state;

    cppClient->listUserGroups(
        cppSession,
        cppUserId,
        cppLimit,
        cppState,
        cppCursor,
        [client, reqData, successCallback](const Nakama::NUserGroupListPtr& groupList)
        {
            if (successCallback)
            {
                sNUserGroupList cGroupList;
                assign(cGroupList, *groupList);
                successCallback(client, reqData, &cGroupList);
                Nakama::sNUserGroupList_free(cGroupList);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_promoteGroupUsers(NClient client, NSession session, const char* groupId, const char** ids, uint16_t idsCount, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_updateGroup(NClient client, NSession session, const char* groupId, const char* name, const char* description, const char* avatarUrl, const char* langTag, const bool* open, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_listLeaderboardRecords(
    NClient client,
    NSession session,
    const char* leaderboardId,
    const char** ownerIds,
    uint16_t ownerIdsCount,
    int32_t limit,
    const char* cursor,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNLeaderboardRecordList*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    std::vector<std::string> cppOwnerIds;
    std::string cppCursor(cursor ? cursor : "");

    Nakama::assign(cppOwnerIds, ownerIds, ownerIdsCount);

    cppClient->listLeaderboardRecords(
        cppSession,
        leaderboardId,
        cppOwnerIds,
        limit,
        cppCursor,
        [client, reqData, successCallback](const Nakama::NLeaderboardRecordListPtr& recordList)
        {
            if (successCallback)
            {
                sNLeaderboardRecordList cRecordList;
                Nakama::assign(cRecordList, *recordList);
                successCallback(client, reqData, &cRecordList);
                Nakama::sNLeaderboardRecordList_free(cRecordList);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_listLeaderboardRecordsAroundOwner(
    NClient client,
    NSession session,
    const char* leaderboardId,
    const char* ownerId,
    int32_t limit,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNLeaderboardRecordList*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    Nakama::opt::optional<int32_t> cppLimit;

    if (limit) cppLimit = limit;

    cppClient->listLeaderboardRecordsAroundOwner(
        cppSession,
        leaderboardId,
        ownerId,
        cppLimit,
        [client, reqData, successCallback](const Nakama::NLeaderboardRecordListPtr& recordList)
        {
            if (successCallback)
            {
                sNLeaderboardRecordList cRecordList;
                Nakama::assign(cRecordList, *recordList);
                successCallback(client, reqData, &cRecordList);
                Nakama::sNLeaderboardRecordList_free(cRecordList);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_writeLeaderboardRecord(
    NClient client,
    NSession session,
    const char* leaderboardId,
    int64_t score,
    const int64_t* subscore,
    const char* metadata,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNLeaderboardRecord*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    Nakama::opt::optional<int64_t> cppSubscore;
    Nakama::opt::optional<std::string> cppMetadata;

    if (subscore) cppSubscore = *subscore;
    if (metadata) cppMetadata = metadata;

    cppClient->writeLeaderboardRecord(
        cppSession,
        leaderboardId,
        score,
        cppSubscore,
        cppMetadata,
        [client, reqData, successCallback](const Nakama::NLeaderboardRecord& record)
        {
            if (successCallback)
            {
                sNLeaderboardRecord cRecord;
                Nakama::assign(cRecord, record);
                successCallback(client, reqData, &cRecord);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_writeTournamentRecord(
    NClient client,
    NSession session,
    const char* tournamentId,
    int64_t score,
    const int64_t* subscore,
    const char* metadata,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNLeaderboardRecord*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    Nakama::opt::optional<int64_t> cppSubscore;
    Nakama::opt::optional<std::string> cppMetadata;

    if (subscore) cppSubscore = *subscore;
    if (metadata) cppMetadata = metadata;

    cppClient->writeTournamentRecord(
        cppSession,
        tournamentId,
        score,
        cppSubscore,
        cppMetadata,
        [client, reqData, successCallback](const Nakama::NLeaderboardRecord& record)
        {
            if (successCallback)
            {
                sNLeaderboardRecord cRecord;
                Nakama::assign(cRecord, record);
                successCallback(client, reqData, &cRecord);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_deleteLeaderboardRecord(NClient client, NSession session, const char* leaderboardId, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_listMatches(
    NClient client,
    NSession session,
    int32_t min_size,
    int32_t max_size,
    int32_t limit,
    const char* label,
    bool authoritative,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNMatchList*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);

    cppClient->listMatches(
        cppSession,
        min_size ? Nakama::opt::optional<int32_t>(min_size) : Nakama::opt::nullopt,
        max_size ? Nakama::opt::optional<int32_t>(max_size) : Nakama::opt::nullopt,
        limit ? Nakama::opt::optional<int32_t>(limit) : Nakama::opt::nullopt,
        label ? Nakama::opt::optional<std::string>(label) : Nakama::opt::nullopt,
        authoritative,
        [client, reqData, successCallback](const Nakama::NMatchListPtr& matchList)
        {
            if (successCallback)
            {
                sNMatchList cMatchList;
                Nakama::assign(cMatchList, *matchList);
                successCallback(client, reqData, &cMatchList);
                Nakama::sNMatchList_free(cMatchList);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_listNotifications(NClient client, NSession session, int32_t limit, const char* cacheableCursor, NClientReqData reqData, void(*successCallback)(const sNNotificationList*), NClientErrorCallback errorCallback)
{
    
}

void NClient_deleteNotifications(NClient client, NSession session, const char** notificationIds, uint16_t notificationIdsCount, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_listChannelMessages(NClient client, NSession session, const char* channelId, int32_t limit, const char* cursor, bool forward, NClientReqData reqData, void(*successCallback)(const sNChannelMessageList*), NClientErrorCallback errorCallback)
{
    
}

void NClient_listTournaments(NClient client, NSession session, const uint32_t* categoryStart, const uint32_t* categoryEnd, const uint32_t* startTime, const uint32_t* endTime, int32_t limit, const char* cursor, NClientReqData reqData, void(*successCallback)(const sNTournamentList*), NClientErrorCallback errorCallback)
{
    
}

void NClient_listTournamentRecords(NClient client, NSession session, const char* tournamentId, int32_t limit, const char* cursor, const char** ownerIds, uint16_t ownerIdsCount, NClientReqData reqData, void(*successCallback)(const sNTournamentRecordList*), NClientErrorCallback errorCallback)
{
    
}

void NClient_listTournamentRecordsAroundOwner(
    NClient client,
    NSession session,
    const char* tournamentId,
    const char* ownerId,
    int32_t limit,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNTournamentRecordList*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);

    cppClient->listTournamentRecordsAroundOwner(
        cppSession,
        tournamentId,
        ownerId,
        limit,
        [client, reqData, successCallback](const Nakama::NTournamentRecordListPtr& recordList)
        {
            if (successCallback)
            {
                sNTournamentRecordList cRecordList;
                Nakama::assign(cRecordList, *recordList);
                successCallback(client, reqData, &cRecordList);
                Nakama::sNTournamentRecordList_free(cRecordList);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_joinTournament(NClient client, NSession session, const char* tournamentId, NClientReqData reqData, void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    
}

void NClient_listStorageObjects(
    NClient client,
    NSession session,
    const char* collection,
    int32_t limit,
    const char* cursor,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNStorageObjectList*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);

    cppClient->listStorageObjects(
        cppSession,
        collection,
        limit,
        cursor ? cursor : "",
        [client, reqData, successCallback](const Nakama::NStorageObjectListPtr& objList)
        {
            if (successCallback)
            {
                sNStorageObjectList cObjList;
                Nakama::assign(cObjList, *objList);
                successCallback(client, reqData, &cObjList);
                Nakama::sNStorageObjectList_free(&cObjList);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_listUsersStorageObjects(
    NClient client,
    NSession session,
    const char* collection,
    const char* userId,
    int32_t limit,
    const char* cursor,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNStorageObjectList*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);

    cppClient->listUsersStorageObjects(
        cppSession,
        collection,
        userId,
        limit,
        cursor ? cursor : "",
        [client, reqData, successCallback](const Nakama::NStorageObjectListPtr& objList)
        {
            if (successCallback)
            {
                sNStorageObjectList cObjList;
                Nakama::assign(cObjList, *objList);
                successCallback(client, reqData, &cObjList);
                Nakama::sNStorageObjectList_free(&cObjList);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_writeStorageObjects(
    NClient client,
    NSession session,
    const sNStorageObjectWrite* objects, uint16_t objectsCount,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNStorageObjectAck* acks, uint16_t count), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    std::vector<Nakama::NStorageObjectWrite> cppObjects;

    assign(cppObjects, objects, objectsCount);

    cppClient->writeStorageObjects(
        cppSession,
        cppObjects,
        [client, reqData, successCallback](const Nakama::NStorageObjectAcks& acks)
        {
            if (successCallback)
            {
                sNStorageObjectAck* cAcks;
                uint16_t count;
                Nakama::assign(cAcks, count, acks);
                successCallback(client, reqData, cAcks, count);
                Nakama::sNStorageObjectAcks_free(cAcks);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_readStorageObjects(
    NClient client,
    NSession session,
    const sNReadStorageObjectId* objectIds,
    uint16_t objectIdsCount,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNStorageObject* objects, uint16_t count), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    std::vector<Nakama::NReadStorageObjectId> cppObjectIds;

    assign(cppObjectIds, objectIds, objectIdsCount);

    cppClient->readStorageObjects(
        cppSession,
        cppObjectIds,
        [client, reqData, successCallback](const Nakama::NStorageObjects& objects)
        {
            if (successCallback)
            {
                sNStorageObject* cObjects;
                uint16_t count;
                Nakama::assign(cObjects, count, objects);
                successCallback(client, reqData, cObjects, count);
                Nakama::sNStorageObjects_free(cObjects);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_deleteStorageObjects(
    NClient client,
    NSession session,
    const sNDeleteStorageObjectId* objectIds, uint16_t objectIdsCount,
    NClientReqData reqData,
    void (*successCallback)(NClient, NClientReqData), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);
    std::vector<Nakama::NDeleteStorageObjectId> cppObjectIds;

    assign(cppObjectIds, objectIds, objectIdsCount);

    cppClient->deleteStorageObjects(
        cppSession,
        cppObjectIds,
        Nakama::createOkEmptyCallback(client, reqData, successCallback),
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

void NClient_rpc(
    NClient client,
    NSession session,
    const char* id,
    const char* payload,
    NClientReqData reqData,
    void(*successCallback)(NClient, NClientReqData, const sNRpc*), NClientErrorCallback errorCallback)
{
    Nakama::NClientInterface* cppClient = getCppClient(client);
    auto cppSession = Nakama::getSession(session);

    cppClient->rpc(
        cppSession,
        id,
        payload ? Nakama::opt::optional<std::string>(payload) : Nakama::opt::nullopt,
        [client, reqData, successCallback](const Nakama::NRpc& rpc)
        {
            if (successCallback)
            {
                sNRpc cRpc;
                Nakama::assign(cRpc, rpc);
                successCallback(client, reqData, &cRpc);
            }
        },
        Nakama::createErrorCallback(client, reqData, errorCallback));
}

} // extern "C"
