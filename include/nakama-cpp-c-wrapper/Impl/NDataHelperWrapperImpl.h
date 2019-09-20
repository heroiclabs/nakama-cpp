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

#pragma once

#include "nakama-cpp/NTypes.h"
#include "nakama-c/NStringMap.h"
#include "nakama-c/data/NAccount.h"
#include "nakama-c/data/NGroup.h"
#include "nakama-c/data/NGroupList.h"
#include "nakama-c/data/NGroupUserList.h"
#include "nakama-c/data/NUsers.h"
#include "nakama-c/data/NUserGroupList.h"
#include "nakama-c/data/NFriendList.h"
#include "nakama-c/data/NLeaderboardRecordList.h"
#include "nakama-c/data/NMatchList.h"
#include "nakama-c/data/NNotificationList.h"
#include "nakama-c/data/NChannelMessageList.h"
#include "nakama-c/data/NTournamentList.h"
#include "nakama-c/data/NTournamentRecordList.h"
#include "nakama-c/data/NStorageObjectList.h"
#include "nakama-c/data/NStorageObjectAck.h"
#include "nakama-c/data/NStorageObjectWrite.h"
#include "nakama-c/data/NStorageObjectId.h"
#include "nakama-c/data/NRpc.h"

NAKAMA_NAMESPACE_BEGIN

NStringMap toCppNStringMap(::NStringMap map)
{
    NStringMap cppMap;

    if (map)
    {
        auto size = ::NStringMap_getSize(map);

        if (size > 0)
        {
            const char** keys = new const char* [size];
            ::NStringMap_getKeys(map, keys);

            for (uint16_t i = 0; i < size; ++i)
            {
                cppMap.emplace(keys[i], ::NStringMap_getValue(map, keys[i]));
            }

            delete[] keys;
        }
    }

    return cppMap;
}

::NStringMap toCNStringMap(NStringMap map)
{
    ::NStringMap cMap = nullptr;

    if (map.size() > 0)
    {
        cMap = ::NStringMap_create();

        for (auto it : map)
        {
            ::NStringMap_setValue(cMap, it.first.c_str(), it.second.c_str());
        }
    }

    return cMap;
}

void assign(NError& error, const tNError* cError)
{
    error.code = (ErrorCode)cError->code;
    error.message = cError->message;
}

void assign(NStringMap& map, const ::NStringMap cMap)
{
    map = toCppNStringMap(cMap);
}

void assign(NAccountDevice& device, const sNAccountDevice* cDevice)
{
    device.id = cDevice->id;
}

void assign(std::vector<NAccountDevice>& devices, const sNAccountDevice* cDevices, uint16_t devicesCount)
{
    devices.resize(devicesCount);

    for (uint16_t i=0; i < devicesCount; ++i)
    {
        assign(devices[i], &cDevices[i]);
    }
}

void assign(NUser& user, const sNUser* cUser)
{
    user.id           = cUser->id;
    user.username     = cUser->username;
    user.displayName  = cUser->displayName;
    user.avatarUrl    = cUser->avatarUrl;
    user.lang         = cUser->lang;
    user.location     = cUser->location;
    user.timeZone     = cUser->timeZone;
    user.metadata     = cUser->metadata;
    user.facebookId   = cUser->facebookId;
    user.googleId     = cUser->googleId;
    user.gameCenterId = cUser->gameCenterId;
    user.steamId      = cUser->steamId;
    user.online       = cUser->online;
    user.edgeCount    = cUser->edgeCount;
    user.createdAt    = cUser->createdAt;
    user.updatedAt    = cUser->updatedAt;
}

void assign(NAccount& account, const sNAccount* cAccount)
{
    assign(account.user, &cAccount->user);
    account.wallet = cAccount->wallet;
    account.email = cAccount->email;
    assign(account.devices, cAccount->devices, cAccount->devicesCount);
    account.custom_id = cAccount->custom_id;
    account.verifyTime = cAccount->verifyTime;
}

void assign(NUsers& users, const sNUsers* cUsers)
{
    users.users.resize(cUsers->usersCount);

    for (uint16_t i=0; i < cUsers->usersCount; ++i)
    {
        assign(users.users[i], &cUsers->users[i]);
    }
}

void assign(NFriend& aFriend, const sNFriend* cFriend)
{
    aFriend.state = (NFriend::State)cFriend->state;
    assign(aFriend.user, &cFriend->user);
}

void assign(NFriendList& friendList, const sNFriendList* cFriendList)
{
    friendList.cursor = cFriendList->cursor;
    friendList.friends.resize(cFriendList->friendsCount);

    for (uint16_t i = 0; i < cFriendList->friendsCount; ++i)
    {
        assign(friendList.friends[i], &cFriendList->friends[i]);
    }
}

void assign(NGroup& group, const sNGroup* cGroup)
{
    group.id = cGroup->id;
    group.creatorId = cGroup->creatorId;
    group.name = cGroup->name;
    group.description = cGroup->description;
    group.lang = cGroup->lang;
    group.metadata = cGroup->metadata;
    group.avatarUrl = cGroup->avatarUrl;
    group.open = cGroup->open;
    group.edgeCount = cGroup->edgeCount;
    group.maxCount = cGroup->maxCount;
    group.createTime = cGroup->createTime;
    group.updateTime = cGroup->updateTime;
}

void assign(NGroupUser& groupUser, const sNGroupUser* cGroupUser)
{
    assign(groupUser.user, &cGroupUser->user);
    groupUser.state = (NUserGroupState)cGroupUser->state;
}

void assign(NGroupUserList& groupUserList, const sNGroupUserList* cGroupUserList)
{
    groupUserList.cursor = cGroupUserList->cursor;
    groupUserList.groupUsers.resize(cGroupUserList->groupUsersCount);

    for (uint16_t i=0; i < cGroupUserList->groupUsersCount; ++i)
    {
        assign(groupUserList.groupUsers[i], &cGroupUserList->groupUsers[i]);
    }
}

void assign(NGroupList& groupList, const sNGroupList* cGroupList)
{
    groupList.cursor = cGroupList->cursor;
    groupList.groups.resize(cGroupList->groupsCount);

    for (uint16_t i = 0; i < cGroupList->groupsCount; ++i)
    {
        assign(groupList.groups[i], &cGroupList->groups[i]);
    }
}

void assign(NUserGroup& userGroup, const sNUserGroup* cUserGroup)
{
    assign(userGroup.group, &cUserGroup->group);
    userGroup.state = (NUserGroupState)cUserGroup->state;
}

void assign(NUserGroupList& userGroupList, const sNUserGroupList* cUserGroupList)
{
    userGroupList.cursor = cUserGroupList->cursor;
    userGroupList.userGroups.resize(cUserGroupList->userGroupsCount);

    for (uint16_t i = 0; i < cUserGroupList->userGroupsCount; ++i)
    {
        assign(userGroupList.userGroups[i], &cUserGroupList->userGroups[i]);
    }
}

void assign(NLeaderboardRecord& record, const sNLeaderboardRecord* cRecord)
{
    record.leaderboardId = cRecord->leaderboardId;
    record.ownerId = cRecord->ownerId;
    record.username = cRecord->username;
    record.score = cRecord->score;
    record.subscore = cRecord->subscore;
    record.numScore = cRecord->numScore;
    record.maxNumScore = cRecord->maxNumScore;
    record.metadata = cRecord->metadata;
    record.createTime = cRecord->createTime;
    record.updateTime = cRecord->updateTime;
    record.expiryTime = cRecord->expiryTime;
    record.rank = cRecord->rank;
}

void assign(NLeaderboardRecordList& list, const sNLeaderboardRecordList* cList)
{
    list.prevCursor = cList->prevCursor;
    list.nextCursor = cList->nextCursor;
    list.records.resize(cList->recordsCount);
    list.ownerRecords.resize(cList->ownerRecordsCount);

    for (uint16_t i = 0; i < cList->recordsCount; ++i)
    {
        assign(list.records[i], &cList->records[i]);
    }

    for (uint16_t i = 0; i < cList->ownerRecordsCount; ++i)
    {
        assign(list.ownerRecords[i], &cList->ownerRecords[i]);
    }
}

void assign(NUserPresence& presence, const sNUserPresence* cPresence)
{
    presence.userId = cPresence->userId;
    presence.sessionId = cPresence->sessionId;
    presence.username = cPresence->username;
    presence.persistence = cPresence->persistence;
    presence.status = cPresence->status;
}

void assign(std::vector<NUserPresence>& presences, const sNUserPresence* cPresences, uint16_t presencesCount)
{
    presences.resize(presencesCount);

    for (uint16_t i = 0; i < presencesCount; ++i)
    {
        assign(presences[i], &cPresences[i]);
    }
}

void assign(NMatch& match, const sNMatch* cMatch)
{
    match.matchId = cMatch->matchId;
    match.authoritative = cMatch->authoritative;
    match.label = cMatch->label;
    match.size = cMatch->size;
    assign(match.self, &cMatch->self);
    assign(match.presences, cMatch->presences, cMatch->presencesCount);
}

void assign(NMatchList& matchList, const sNMatchList* cMatchList)
{
    matchList.matches.resize(cMatchList->matchesCount);

    for (uint16_t i = 0; i < cMatchList->matchesCount; ++i)
    {
        assign(matchList.matches[i], &cMatchList->matches[i]);
    }
}

NAKAMA_NAMESPACE_END
