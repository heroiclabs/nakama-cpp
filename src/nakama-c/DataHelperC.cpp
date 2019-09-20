#include "DataHelperC.h"
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

#include "nakama-c/DataHelperC.h"
#include "nakama-c/NStringMap.h"

NAKAMA_NAMESPACE_BEGIN

::NStringMap saveNStringMap(const NStringMap& map);

void assign(const char*& s, const std::string& str)
{
    s = str.c_str();
}

void assign(std::vector<std::string>& strings, const char** cStrings, uint16_t count)
{
    strings.resize(count);

    for (uint16_t i = 0; i < count; ++i)
    {
        strings[i] = cStrings[i];
    }
}

void assign(tNError& cError, const Nakama::NError& error)
{
    cError.code = (tNErrorCode)error.code;
    assign(cError.message, error.message);
}

void assign(sNUser& cUser, const Nakama::NUser& user)
{
    assign(cUser.id, user.id);
    assign(cUser.username, user.username);
    assign(cUser.displayName, user.displayName);
    assign(cUser.avatarUrl, user.avatarUrl);
    assign(cUser.lang, user.lang);
    assign(cUser.location, user.location);
    assign(cUser.timeZone, user.timeZone);
    assign(cUser.metadata, user.metadata);
    assign(cUser.facebookId, user.facebookId);
    assign(cUser.googleId, user.googleId);
    assign(cUser.gameCenterId , user.gameCenterId);
    assign(cUser.steamId, user.steamId);
    cUser.online = user.online;
    cUser.edgeCount = user.edgeCount;
    cUser.createdAt = user.createdAt;
    cUser.updatedAt = user.updatedAt;
}

void assign(sNAccountDevice& cDevice, const Nakama::NAccountDevice& device)
{
    assign(cDevice.id, device.id);
}

void assign(sNAccountDevice*& cDevices, uint16_t& devicesCount, const std::vector<Nakama::NAccountDevice>& devices)
{
    devicesCount = (uint16_t)devices.size();
    cDevices = nullptr;

    if (devicesCount > 0)
    {
        cDevices = new sNAccountDevice[devicesCount];

        for (uint16_t i=0; i < devicesCount; ++i)
        {
            assign(cDevices[i], devices[i]);
        }
    }
}

void assign(sNAccount& cAccount, const Nakama::NAccount& account)
{
    assign(cAccount.user, account.user);
    assign(cAccount.wallet, account.wallet);
    assign(cAccount.email, account.email);
    assign(cAccount.devices, cAccount.devicesCount, account.devices);
    assign(cAccount.custom_id, account.custom_id);
    cAccount.verifyTime = account.verifyTime;
}

void assign(sNUsers& cUsers, const Nakama::NUsers& users)
{
    cUsers.usersCount = (uint16_t)users.users.size();
    cUsers.users = nullptr;

    if (cUsers.usersCount > 0)
    {
        cUsers.users = new sNUser[cUsers.usersCount];

        for (uint16_t i = 0; i < cUsers.usersCount; ++i)
        {
            assign(cUsers.users[i], users.users[i]);
        }
    }
}

void assign(sNFriend& cFriend, const Nakama::NFriend& aFriend)
{
    cFriend.state = (eFriendState)aFriend.state;
    assign(cFriend.user, aFriend.user);
}

void assign(sNFriendList& cFriends, const Nakama::NFriendList& friends)
{
    cFriends.cursor = friends.cursor.c_str();
    cFriends.friendsCount = (uint16_t)friends.friends.size();
    cFriends.friends = nullptr;

    if (cFriends.friendsCount > 0)
    {
        cFriends.friends = new sNFriend[cFriends.friendsCount];

        for (uint16_t i = 0; i < cFriends.friendsCount; ++i)
        {
            assign(cFriends.friends[i], friends.friends[i]);
        }
    }
}

void assign(sNGroup& cGroup, const Nakama::NGroup& group)
{
    cGroup.id = group.id.c_str();
    cGroup.creatorId = group.creatorId.c_str();
    cGroup.name = group.name.c_str();
    cGroup.description = group.description.c_str();
    cGroup.lang = group.lang.c_str();
    cGroup.metadata = group.metadata.c_str();
    cGroup.avatarUrl = group.avatarUrl.c_str();
    cGroup.open = group.open;
    cGroup.edgeCount = group.edgeCount;
    cGroup.maxCount = group.maxCount;
    cGroup.createTime = group.createTime;
    cGroup.updateTime = group.updateTime;
}

void assign(sNGroupUser& cGroupUser, const Nakama::NGroupUser& groupUser)
{
    assign(cGroupUser.user, groupUser.user);
    cGroupUser.state = (eNUserGroupState)groupUser.state;
}

void assign(sNGroupUserList& cGroupUserList, const Nakama::NGroupUserList& groupUserList)
{
    cGroupUserList.cursor = groupUserList.cursor.c_str();
    cGroupUserList.groupUsersCount = (uint16_t)groupUserList.groupUsers.size();
    cGroupUserList.groupUsers = nullptr;

    if (cGroupUserList.groupUsersCount > 0)
    {
        cGroupUserList.groupUsers = new sNGroupUser[cGroupUserList.groupUsersCount];

        for (uint16_t i = 0; i < cGroupUserList.groupUsersCount; ++i)
        {
            assign(cGroupUserList.groupUsers[i], groupUserList.groupUsers[i]);
        }
    }
}

void assign(sNUserGroup& cGroup, const Nakama::NUserGroup& group)
{
    assign(cGroup.group, group.group);
    cGroup.state = (eNUserGroupState)group.state;
}

void assign(sNUserGroupList& cGroupList, const Nakama::NUserGroupList& groupList)
{
    cGroupList.cursor = groupList.cursor.c_str();
    cGroupList.userGroupsCount = (uint16_t)groupList.userGroups.size();
    cGroupList.userGroups = nullptr;

    if (cGroupList.userGroupsCount > 0)
    {
        cGroupList.userGroups = new sNUserGroup[cGroupList.userGroupsCount];

        for (uint16_t i = 0; i < cGroupList.userGroupsCount; ++i)
        {
            assign(cGroupList.userGroups[i], groupList.userGroups[i]);
        }
    }
}

void assign(sNGroupList& cGroupList, const Nakama::NGroupList& groupList)
{
    cGroupList.cursor = groupList.cursor.c_str();
    cGroupList.groupsCount = (uint16_t)groupList.groups.size();
    cGroupList.groups = nullptr;

    if (cGroupList.groupsCount > 0)
    {
        cGroupList.groups = new sNGroup[cGroupList.groupsCount];

        for (uint16_t i = 0; i < cGroupList.groupsCount; ++i)
        {
            assign(cGroupList.groups[i], groupList.groups[i]);
        }
    }
}

void assign(sNLeaderboardRecord& cRecord, const Nakama::NLeaderboardRecord& recordList)
{
    cRecord.leaderboardId = recordList.leaderboardId.c_str();
    cRecord.ownerId = recordList.ownerId.c_str();
    cRecord.username = recordList.username.c_str();
    cRecord.score = recordList.score;
    cRecord.subscore = recordList.subscore;
    cRecord.numScore = recordList.numScore;
    cRecord.maxNumScore = recordList.maxNumScore;
    cRecord.metadata = recordList.metadata.c_str();
    cRecord.createTime = recordList.createTime;
    cRecord.updateTime = recordList.updateTime;
    cRecord.expiryTime = recordList.expiryTime;
    cRecord.rank = recordList.rank;
}

void assign(sNLeaderboardRecordList& cRecordList, const Nakama::NLeaderboardRecordList& recordList)
{
    cRecordList.prevCursor = recordList.prevCursor.c_str();
    cRecordList.nextCursor = recordList.nextCursor.c_str();
    cRecordList.ownerRecords = nullptr;
    cRecordList.records = nullptr;
    cRecordList.recordsCount = (uint16_t)recordList.records.size();
    cRecordList.ownerRecordsCount = (uint16_t)recordList.ownerRecords.size();
    
    if (cRecordList.recordsCount > 0)
    {
        cRecordList.records = new sNLeaderboardRecord[cRecordList.recordsCount];

        for (uint16_t i = 0; i < cRecordList.recordsCount; ++i)
        {
            assign(cRecordList.records[i], recordList.records[i]);
        }
    }

    if (cRecordList.ownerRecordsCount > 0)
    {
        cRecordList.ownerRecords = new sNLeaderboardRecord[cRecordList.ownerRecordsCount];

        for (uint16_t i = 0; i < cRecordList.ownerRecordsCount; ++i)
        {
            assign(cRecordList.ownerRecords[i], recordList.ownerRecords[i]);
        }
    }
}

void assign(sNUserPresence& cPresence, const NUserPresence& presence)
{
    cPresence.userId = presence.userId.c_str();
    cPresence.sessionId = presence.sessionId.c_str();
    cPresence.username = presence.username.c_str();
    cPresence.persistence = presence.persistence;
    cPresence.status = presence.status.c_str();
}

void assign(sNUserPresence*& cPresences, uint16_t& cPresencesCount, const std::vector<NUserPresence>& presences)
{
    cPresences = nullptr;
    cPresencesCount = (uint16_t)presences.size();

    if (cPresencesCount > 0)
    {
        cPresences = new sNUserPresence[cPresencesCount];

        for (uint16_t i=0; i < cPresencesCount; ++i)
        {
            assign(cPresences[i], presences[i]);
        }
    }
}

void assign(sNMatch& cMatch, const Nakama::NMatch& match)
{
    cMatch.matchId = match.matchId.c_str();
    cMatch.authoritative = match.authoritative;
    cMatch.label = match.label.c_str();
    cMatch.size = match.size;
    assign(cMatch.presences, cMatch.presencesCount, match.presences);
    assign(cMatch.self, match.self);
}

void assign(sNMatchList& cMatchList, const Nakama::NMatchList& matchList)
{
    cMatchList.matches = nullptr;
    cMatchList.matchesCount = (uint16_t)matchList.matches.size();

    if (cMatchList.matchesCount > 0)
    {
        cMatchList.matches = new sNMatch[cMatchList.matchesCount];

        for (uint16_t i = 0; i < cMatchList.matchesCount; ++i)
        {
            assign(cMatchList.matches[i], matchList.matches[i]);
        }
    }
}

void sNAccountDevice_free(sNAccountDevice& cDevice)
{
}

void sNAccount_free(sNAccount& cAccount)
{
    if (cAccount.devicesCount > 0)
    {
        for (uint16_t i=0; i < cAccount.devicesCount; ++i)
        {
            sNAccountDevice_free(cAccount.devices[i]);
        }

        delete[] cAccount.devices;
    }
}

void sNUsers_free(sNUsers& cUsers)
{
    delete[] cUsers.users;
}

void sNFriendList_free(sNFriendList& cFriends)
{
    delete[] cFriends.friends;
}

void sNGroupUserList_free(sNGroupUserList& cGroupUserList)
{
    delete[] cGroupUserList.groupUsers;
}

void sNUserGroupList_free(sNUserGroupList& cUserGroupList)
{
    delete[] cUserGroupList.userGroups;
}

void sNGroupList_free(sNGroupList& cGroupList)
{
    delete[] cGroupList.groups;
}

void sNLeaderboardRecordList_free(sNLeaderboardRecordList& cRecordList)
{
    delete[] cRecordList.records;
    delete[] cRecordList.ownerRecords;
}

void sNMatch_free(sNMatch& cMatch)
{
    delete[] cMatch.presences;
}

void sNMatchList_free(sNMatchList& cMatchList)
{
    if (cMatchList.matchesCount > 0)
    {
        for (uint16_t i = 0; i < cMatchList.matchesCount; ++i)
        {
            sNMatch_free(cMatchList.matches[i]);
        }

        delete[] cMatchList.matches;
    }
}

NAKAMA_NAMESPACE_END
