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

#ifdef BUILD_C_API

#include "nakama-c/DataHelperC.h"
#include "nakama-c/NStringMap.h"

NAKAMA_NAMESPACE_BEGIN

::NStringMap saveNStringMap(const NStringMap& map);
::NStringDoubleMap saveNStringDoubleMap(const NStringDoubleMap& map);

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

void assign(Nakama::NReadStorageObjectId& objectId, const sNReadStorageObjectId& cObjectId)
{
    objectId.collection = cObjectId.collection;
    objectId.key = cObjectId.key;
    objectId.userId = cObjectId.userId;
}

void assign(Nakama::NDeleteStorageObjectId& objectId, const sNDeleteStorageObjectId& cObjectId)
{
    objectId.collection = cObjectId.collection;
    objectId.key = cObjectId.key;
    objectId.version = cObjectId.version;
}

void assign(Nakama::NStorageObjectWrite& cppObject, const sNStorageObjectWrite* object)
{
    cppObject.collection = object->collection;
    cppObject.key = object->key;
    cppObject.value = object->value;
    cppObject.version = object->version;

    if (object->permissionRead) cppObject.permissionRead = (Nakama::NStoragePermissionRead) (*object->permissionRead);
    if (object->permissionWrite) cppObject.permissionWrite = (Nakama::NStoragePermissionWrite) (*object->permissionWrite);
}

void assign(std::vector<Nakama::NStorageObjectWrite>& cppObjects, const sNStorageObjectWrite* objects, uint16_t objectsCount)
{
    cppObjects.resize(objectsCount);

    for (uint16_t i=0; i < objectsCount; ++i)
    {
        assign(cppObjects[i], &objects[i]);
    }
}

void assign(std::vector<Nakama::NUserPresence>& presences, const sNUserPresence* cPresences, uint16_t count)
{
    presences.resize(count);

    for (uint16_t i=0; i < count; ++i)
    {
        assign(presences[i], cPresences[i]);
    }
}

void assign(Nakama::NBytes& data, const sNBytes* cData)
{
    if (cData)
    {
        data.resize(cData->size);
        for (uint32_t i=0; i < cData->size; ++i)
        {
            data[i] = cData->bytes[i];
        }
    }
}

void assign(sNBytes& cData, const Nakama::NBytes& data)
{
    cData.bytes = (uint8_t*)data.data();
    cData.size = (uint32_t)data.size();
}

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

NStringDoubleMap toCppNStringDoubleMap(::NStringDoubleMap map)
{
    NStringDoubleMap cppMap;

    if (map)
    {
        auto size = ::NStringDoubleMap_getSize(map);

        if (size > 0)
        {
            double value = 0;
            const char** keys = new const char* [size];
            ::NStringDoubleMap_getKeys(map, keys);

            for (uint16_t i = 0; i < size; ++i)
            {
                if (::NStringDoubleMap_getValue(map, keys[i], &value))
                    cppMap.emplace(keys[i], value);
            }

            delete[] keys;
        }
    }

    return cppMap;
}

void assign(Nakama::NStringMap& map, const ::NStringMap cMap)
{
    map = toCppNStringMap(cMap);
}

void assign(Nakama::NStringDoubleMap& map, ::NStringDoubleMap cMap)
{
    map = toCppNStringDoubleMap(cMap);
}

void assign(std::vector<Nakama::NDeleteStorageObjectId>& objectIds, const sNDeleteStorageObjectId* cObjectIds, uint16_t count)
{
    objectIds.resize(count);

    for (uint16_t i=0; i < count; ++i)
    {
        assign(objectIds[i], cObjectIds[i]);
    }
}

void assign(std::vector<Nakama::NReadStorageObjectId>& objectIds, const sNReadStorageObjectId* cObjectIds, uint16_t count)
{
    objectIds.resize(count);

    for (uint16_t i = 0; i < count; ++i)
    {
        assign(objectIds[i], cObjectIds[i]);
    }
}

void assign(::NStringMap& cMap, const Nakama::NStringMap& map)
{
    cMap = map.empty() ? nullptr : Nakama::saveNStringMap(map);
}

void assign(::NStringDoubleMap& cMap, const Nakama::NStringDoubleMap& map)
{
    cMap = map.empty() ? nullptr : Nakama::saveNStringDoubleMap(map);
}

void assign(sNError& cError, const Nakama::NError& error)
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
    assign(cUser.appleId, user.appleId);
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
    assign(cAccount.customId, account.customId);
    cAccount.verifyTime = account.verifyTime;
    cAccount.disableTime = account.disableTime;
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
    cFriend.updateTime = aFriend.updateTime;
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

void assign(sNLeaderboardRecord*& cRecords, uint16_t& count, const std::vector<Nakama::NLeaderboardRecord>& records)
{
    count = (uint16_t)records.size();
    cRecords = nullptr;

    if (count > 0)
    {
        cRecords = new sNLeaderboardRecord[count];

        for (uint16_t i = 0; i < count; ++i)
        {
            assign(cRecords[i], records[i]);
        }
    }
}

void assign(sNLeaderboardRecordList& cRecordList, const Nakama::NLeaderboardRecordList& recordList)
{
    cRecordList.prevCursor = recordList.prevCursor.c_str();
    cRecordList.nextCursor = recordList.nextCursor.c_str();
    assign(cRecordList.records, cRecordList.recordsCount, recordList.records);
    assign(cRecordList.ownerRecords, cRecordList.ownerRecordsCount, recordList.ownerRecords);
}

void assign(Nakama::NUserPresence& presence, const sNUserPresence& cPresence)
{
    presence.userId = cPresence.userId;
    presence.sessionId = cPresence.sessionId;
    presence.username = cPresence.username;
    presence.persistence = cPresence.persistence;
    presence.status = cPresence.status;
}

void assign(sNUserPresence& cPresence, const Nakama::NUserPresence& presence)
{
    cPresence.userId = presence.userId.c_str();
    cPresence.sessionId = presence.sessionId.c_str();
    cPresence.username = presence.username.c_str();
    cPresence.persistence = presence.persistence;
    cPresence.status = presence.status.c_str();
}

void assign(sNMatchmakerTicket& cTicket, const Nakama::NMatchmakerTicket& ticket)
{
    cTicket.ticket = ticket.ticket.c_str();
}

void assign(sNChannelMessageAck& cAck, const Nakama::NChannelMessageAck& ack)
{
    cAck.channelId = ack.channelId.c_str();
    cAck.messageId = ack.messageId.c_str();
    cAck.username = ack.username.c_str();
    cAck.code = ack.code;
    cAck.createTime = ack.createTime;
    cAck.updateTime = ack.updateTime;
    cAck.persistent = ack.persistent;
    cAck.roomName = ack.roomName.c_str();
    cAck.groupId = ack.groupId.c_str();
    cAck.userIdOne = ack.userIdOne.c_str();
    cAck.userIdTwo = ack.userIdTwo.c_str();
}

void assign(sNChannelPresenceEvent& cPresence, const Nakama::NChannelPresenceEvent& presence)
{
    cPresence.channelId = presence.channelId.c_str();
    assign(cPresence.joins, cPresence.joinsCount, presence.joins);
    assign(cPresence.leaves, cPresence.leavesCount, presence.leaves);
    cPresence.roomName  = presence.roomName.c_str();
    cPresence.groupId   = presence.groupId.c_str();
    cPresence.userIdOne = presence.userIdOne.c_str();
    cPresence.userIdTwo = presence.userIdTwo.c_str();
}

void assign(sNMatchmakerUser& cUser, const Nakama::NMatchmakerUser& user)
{
    assign(cUser.presence, user.presence);
    assign(cUser.stringProperties, user.stringProperties);
    assign(cUser.numericProperties, user.numericProperties);
}

void assign(sNMatchmakerUser*& cUsers, uint16_t& count, const std::vector<Nakama::NMatchmakerUser>& users)
{
    cUsers = nullptr;
    count = (uint16_t)users.size();

    if (count > 0)
    {
        cUsers = new sNMatchmakerUser[count];

        for (uint16_t i=0; i < count; ++i)
        {
            assign(cUsers[i], users[i]);
        }
    }
}

void assign(sNMatchmakerMatched& cMatched, const Nakama::NMatchmakerMatched& matched)
{
    cMatched.ticket  = matched.ticket.c_str();
    cMatched.matchId = matched.matchId.c_str();
    cMatched.token   = matched.token.c_str();
    assign(cMatched.users, cMatched.usersCount, matched.users);
    assign(cMatched.self, matched.self);
}

void assign(sNMatchData& cData, const Nakama::NMatchData& data)
{
    cData.matchId = data.matchId.c_str();
    assign(cData.presence, data.presence);
    cData.opCode = data.opCode;
    assign(cData.data, data.data);
}

void assign(sNMatchPresenceEvent& cEvent, const Nakama::NMatchPresenceEvent& event)
{
    cEvent.matchId = event.matchId.c_str();
    assign(cEvent.joins, cEvent.joinsCount, event.joins);
    assign(cEvent.leaves, cEvent.leavesCount, event.leaves);
}

void assign(sNStatusPresenceEvent& cEvent, const Nakama::NStatusPresenceEvent& event)
{
    assign(cEvent.joins, cEvent.joinsCount, event.joins);
    assign(cEvent.leaves, cEvent.leavesCount, event.leaves);
}

void assign(sNStream& cStream, const Nakama::NStream& stream)
{
    cStream.mode = stream.mode;
    cStream.subject = stream.subject.c_str();
    cStream.subcontext = stream.subcontext.c_str();
    cStream.label = stream.label.c_str();
}

void assign(sNStreamPresenceEvent& cEvent, const Nakama::NStreamPresenceEvent& event)
{
    assign(cEvent.stream, event.stream);
    assign(cEvent.joins, cEvent.joinsCount, event.joins);
    assign(cEvent.leaves, cEvent.leavesCount, event.leaves);
}

void assign(sNStreamData& cData, const Nakama::NStreamData& data)
{
    assign(cData.stream, data.stream);
    assign(cData.sender, data.sender);
    cData.data = data.data.c_str();
}

void assign(sNUserPresence*& cPresences, uint16_t& cPresencesCount, const std::vector<Nakama::NUserPresence>& presences)
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

void assign(sNRpc& cRpc, const Nakama::NRpc& rpc)
{
    cRpc.id = rpc.id.c_str();
    cRpc.payload = rpc.payload.c_str();
    cRpc.httpKey = rpc.httpKey.c_str();
}

void assign(sNStorageObject* cObject, const NStorageObject& object)
{
    cObject->collection = object.collection.c_str();
    cObject->key = object.key.c_str();
    cObject->userId = object.userId.c_str();
    cObject->value = object.value.c_str();
    cObject->version = object.version.c_str();
    cObject->permissionRead = (eNStoragePermissionRead)object.permissionRead;
    cObject->permissionWrite = (eNStoragePermissionWrite)object.permissionWrite;
    cObject->createTime = object.createTime;
    cObject->updateTime = object.updateTime;
}

void assign(sNStorageObject*& cObjects, uint16_t& count, const Nakama::NStorageObjects& objects)
{
    cObjects = nullptr;
    count = (uint16_t)objects.size();

    if (count > 0)
    {
        cObjects = new sNStorageObject[count];

        for (uint16_t i = 0; i < count; ++i)
        {
            assign(&cObjects[i], objects[i]);
        }
    }
}

void assign(sNStorageObjectAck* cAck, const Nakama::NStorageObjectAck& ack)
{
    cAck->collection = ack.collection.c_str();
    cAck->key = ack.key.c_str();
    cAck->version = ack.version.c_str();
    cAck->userId = ack.userId.c_str();
}

void assign(sNStorageObjectAck*& cAcks, uint16_t& count, const Nakama::NStorageObjectAcks& acks)
{
    cAcks = nullptr;
    count = (uint16_t)acks.size();

    if (count > 0)
    {
        cAcks = new sNStorageObjectAck[count];

        for (uint16_t i = 0; i < count; ++i)
        {
            assign(&cAcks[i], acks[i]);
        }
    }
}

void assign(sNStorageObjectList& cObjList, const Nakama::NStorageObjectList& objList)
{
    cObjList.objects = nullptr;
    cObjList.objectsCount = (uint16_t)objList.objects.size();
    cObjList.cursor = objList.cursor.c_str();

    if (cObjList.objectsCount > 0)
    {
        cObjList.objects = new sNStorageObject[cObjList.objectsCount];

        for (uint16_t i = 0; i < cObjList.objectsCount; ++i)
        {
            assign(&cObjList.objects[i], objList.objects[i]);
        }
    }
}

void assign(sNTournamentRecordList& cRecordList, const Nakama::NTournamentRecordList& recordList)
{
    cRecordList.nextCursor = recordList.nextCursor.c_str();
    cRecordList.prevCursor = recordList.prevCursor.c_str();
    assign(cRecordList.records, cRecordList.recordsCount, recordList.records);
    assign(cRecordList.ownerRecords, cRecordList.ownerRecordsCount, recordList.ownerRecords);
}

void assign(sNTournament* cT, const NTournament& t)
{
    cT->id = t.id.c_str();
    cT->title = t.title.c_str();
    cT->description = t.description.c_str();
    cT->category = t.category;
    cT->sortOrder = t.sortOrder;
    cT->size = t.size;
    cT->maxSize = t.maxSize;
    cT->maxNumScore = t.maxNumScore;
    cT->canEnter = t.canEnter;
    cT->createTime = t.createTime;
    cT->startTime = t.startTime;
    cT->endTime = t.endTime;
    cT->endActive = t.endActive;
    cT->nextReset = t.nextReset;
    cT->duration = t.duration;
    cT->startActive = t.startActive;
    cT->metadata = t.metadata.c_str();
}

void assign(sNTournament*& cList, uint16_t& count, const std::vector<Nakama::NTournament>& list)
{
    cList = nullptr;
    count = (uint16_t)list.size();

    if (count > 0)
    {
        cList = new sNTournament[count];

        for (uint16_t i=0; i < count; ++i)
        {
            assign(&cList[i], list[i]);
        }
    }
}

void assign(sNTournamentList& cList, const Nakama::NTournamentList& list)
{
    cList.cursor = list.cursor.c_str();
    assign(cList.tournaments, cList.tournamentsCount, list.tournaments);
}

void assign(sNNotification* cN, const Nakama::NNotification& n)
{
    cN->id = n.id.c_str();
    cN->subject = n.subject.c_str();
    cN->content = n.content.c_str();
    cN->code = n.code;
    cN->senderId = n.senderId.c_str();
    cN->createTime = n.createTime;
    cN->persistent = n.persistent;
}

void assign(sNNotificationList& cList, const Nakama::NNotificationList& list)
{
    cList.cacheableCursor = list.cacheableCursor.c_str();
    cList.notificationsCount = (uint16_t)list.notifications.size();
    cList.notifications = nullptr;

    if (cList.notificationsCount > 0)
    {
        cList.notifications = new sNNotification[cList.notificationsCount];

        for (uint16_t i=0; i < cList.notificationsCount; ++i)
        {
            assign(&cList.notifications[i], list.notifications[i]);
        }
    }
}

void assign(sNChannelMessage& cMsg, const Nakama::NChannelMessage& msg)
{
    cMsg.channelId = msg.channelId.c_str();
    cMsg.messageId = msg.messageId.c_str();
    cMsg.code = msg.code;
    cMsg.senderId = msg.senderId.c_str();
    cMsg.username = msg.username.c_str();
    cMsg.content = msg.content.c_str();
    cMsg.createTime = msg.createTime;
    cMsg.updateTime = msg.updateTime;
    cMsg.persistent = msg.persistent;
    cMsg.roomName = msg.roomName.c_str();
    cMsg.groupId = msg.groupId.c_str();
    cMsg.userIdOne = msg.userIdOne.c_str();
    cMsg.userIdTwo = msg.userIdTwo.c_str();
}

void assign(sNChannelMessageList& cList, const Nakama::NChannelMessageList& list)
{
    cList.messages = nullptr;
    cList.messagesCount = (uint16_t)list.messages.size();
    if (cList.messagesCount > 0)
    {
        cList.messages = new sNChannelMessage[cList.messagesCount];

        for (uint16_t i=0; i < cList.messagesCount; ++i)
        {
            assign(cList.messages[i], list.messages[i]);
        }
    }

    cList.nextCursor = list.nextCursor.c_str();
    cList.prevCursor = list.prevCursor.c_str();
}

void assign(sNRtError& cError, const NRtError& error)
{
    cError.code = (eRtErrorCode)error.code;
    cError.message = error.message.c_str();
    assign(cError.context, error.context);
}

void assign(sNChannel& cChannel, const NChannel& channel)
{
    cChannel.id = channel.id.c_str();
    assign(cChannel.presences, cChannel.presencesCount, channel.presences);
    assign(cChannel.self, channel.self);
    cChannel.roomName = channel.roomName.c_str();
    cChannel.groupId = channel.groupId.c_str();
    cChannel.userIdOne = channel.userIdOne.c_str();
    cChannel.userIdTwo = channel.userIdTwo.c_str();
}

void assign(sNRtClientDisconnectInfo& cInfo, const Nakama::NRtClientDisconnectInfo& info)
{
    cInfo.code = info.code;
    cInfo.reason = info.reason.c_str();
    cInfo.remote = info.remote;
}

void assign(sNStatus& cStatus, const Nakama::NStatus& status)
{
    assign(cStatus.presences, cStatus.presencesCount, status.presences);
}

void assign(sNParty& cParty, const Nakama::NParty& party)
{
    cParty.id = party.id.c_str();
    cParty.open = party.open;
    cParty.maxSize = party.maxSize;
    assign(cParty.self, party.self);
    assign(cParty.leader, party.leader);
    assign(cParty.presences, cParty.presencesCount, party.presences);
}

void assign(sNPartyPresenceEvent& cEvent, const Nakama::NPartyPresenceEvent& event)
{
    cEvent.partyId = event.partyId.c_str();
    assign(cEvent.joins, cEvent.joinsCount, event.joins);
    assign(cEvent.leaves, cEvent.leavesCount, event.leaves);
}

void assign(sNPartyData& cData, const Nakama::NPartyData& partyData)
{
    cData.partyId = partyData.partyId.c_str();
    assign(cData.presence, partyData.presence);
    cData.opCode = partyData.opCode;
    assign(cData.data, partyData.data);
}

void assign(sNPartyLeader& cLeader, const Nakama::NPartyLeader& partyLeader)
{
    cLeader.partyId = partyLeader.partyId.c_str();
    assign(cLeader.presence, partyLeader.presence);
}

void assign(sNPartyMatchmakerTicket& cTicket, const Nakama::NPartyMatchmakerTicket& ticket)
{
    cTicket.partyId = ticket.partyId.c_str();
    cTicket.ticket = ticket.ticket.c_str();
}

void assign(sNPartyJoinRequest& cRequest, const Nakama::NPartyJoinRequest& request)
{
    cRequest.partyId = request.partyId.c_str();
    assign(cRequest.presences, cRequest.presencesCount, request.presences);
}

void sNRtError_free(sNRtError& cError)
{
    ::NStringMap_destroy(cError.context);
    cError.context = nullptr;
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
        cAccount.devices = nullptr;
    }
}

void sNUsers_free(sNUsers& cUsers)
{
    delete[] cUsers.users;
    cUsers.users = nullptr;
}

void sNFriendList_free(sNFriendList& cFriends)
{
    delete[] cFriends.friends;
    cFriends.friends = nullptr;
}

void sNGroupUserList_free(sNGroupUserList& cGroupUserList)
{
    delete[] cGroupUserList.groupUsers;
    cGroupUserList.groupUsers = nullptr;
}

void sNUserGroupList_free(sNUserGroupList& cUserGroupList)
{
    delete[] cUserGroupList.userGroups;
    cUserGroupList.userGroups = nullptr;
}

void sNGroupList_free(sNGroupList& cGroupList)
{
    delete[] cGroupList.groups;
    cGroupList.groups = nullptr;
}

void sNLeaderboardRecordList_free(sNLeaderboardRecordList& cRecordList)
{
    delete[] cRecordList.records;
    delete[] cRecordList.ownerRecords;
    cRecordList.records = nullptr;
    cRecordList.ownerRecords = nullptr;
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
        cMatchList.matches = nullptr;
    }
}

void sNStorageObjects_free(sNStorageObject* cObjects)
{
    delete[] cObjects;
}

void sNStorageObjectAcks_free(sNStorageObjectAck* cAcks)
{
    delete[] cAcks;
}

void sNStorageObjectList_free(sNStorageObjectList* cObjList)
{
    delete[] cObjList->objects;
    cObjList->objects = nullptr;
}

void sNTournamentRecordList_free(sNTournamentRecordList& cRecordList)
{
    delete[] cRecordList.records;
    delete[] cRecordList.ownerRecords;
    cRecordList.records = nullptr;
    cRecordList.ownerRecords = nullptr;
}

void sNTournamentList_free(sNTournamentList& cList)
{
    delete[] cList.tournaments;
    cList.tournaments = nullptr;
}

void sNNotificationList_free(sNNotificationList& cList)
{
    delete[] cList.notifications;
    cList.notifications = nullptr;
}

void sNChannelMessageList_free(sNChannelMessageList& cList)
{
    delete[] cList.messages;
    cList.messages = nullptr;
}

void sNChannel_free(sNChannel& cChannel)
{
    delete[] cChannel.presences;
    cChannel.presences = nullptr;
}

void sNStatus_free(sNStatus& cStatus)
{
    delete[] cStatus.presences;
    cStatus.presences = nullptr;
}

void sNChannelPresenceEvent_free(sNChannelPresenceEvent& cPresence)
{
    delete[] cPresence.joins;
    delete[] cPresence.leaves;
    cPresence.joins  = nullptr;
    cPresence.leaves = nullptr;
}

void sNMatchmakerUser_free(sNMatchmakerUser& user)
{
    NStringMap_destroy(user.stringProperties);
    NStringDoubleMap_destroy(user.numericProperties);
}

void sNMatchmakerMatched_free(sNMatchmakerMatched& cMatched)
{
    for (uint16_t i = 0; i < cMatched.usersCount; ++i)
    {
        sNMatchmakerUser_free(cMatched.users[i]);
    }

    sNMatchmakerUser_free(cMatched.self);

    delete[] cMatched.users;
    cMatched.users = nullptr;
}

void sNMatchPresenceEvent_free(sNMatchPresenceEvent& cEvent)
{
    delete[] cEvent.joins;
    delete[] cEvent.leaves;
    cEvent.joins = nullptr;
    cEvent.leaves = nullptr;
}

void sNStatusPresenceEvent_free(sNStatusPresenceEvent& cEvent)
{
    delete[] cEvent.joins;
    delete[] cEvent.leaves;
    cEvent.joins = nullptr;
    cEvent.leaves = nullptr;
}

void sNStreamPresenceEvent_free(sNStreamPresenceEvent& cEvent)
{
    delete[] cEvent.joins;
    delete[] cEvent.leaves;
    cEvent.joins = nullptr;
    cEvent.leaves = nullptr;
}

void sNPartyJoinRequest_free(sNPartyJoinRequest& cRequest)
{
}

void sNPartyMatchmakerTicket_free(sNPartyMatchmakerTicket& cTicket)
{

}

void sNPartyLeader_free(sNPartyLeader& cLeader)
{

}

void sNPartyPresenceEvent_free(sNPartyPresenceEvent& cEvent)
{
    delete[] cEvent.joins;
    delete[] cEvent.leaves;
    cEvent.joins = nullptr;
    cEvent.leaves = nullptr;
}

void sNPartyData_free(sNPartyData& cPartyData)
{

}

void sNParty_free(sNParty& cParty)
{

}

NAKAMA_NAMESPACE_END

#endif // BUILD_C_API
