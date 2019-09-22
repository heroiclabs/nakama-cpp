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

#include "nakama-c/NError.h"
#include "nakama-c/data/NAccount.h"
#include "nakama-c/data/NUsers.h"
#include "nakama-c/data/NFriendList.h"
#include "nakama-c/data/NGroup.h"
#include "nakama-c/data/NGroupUserList.h"
#include "nakama-c/data/NUserGroupList.h"
#include "nakama-c/data/NGroupList.h"
#include "nakama-c/data/NLeaderboardRecordList.h"
#include "nakama-c/data/NMatchList.h"
#include "nakama-c/data/NRpc.h"
#include "nakama-c/data/NStorageObjectId.h"
#include "nakama-c/data/NStorageObject.h"
#include "nakama-c/data/NStorageObjectWrite.h"
#include "nakama-c/data/NStorageObjectAck.h"
#include "nakama-c/data/NStorageObjectList.h"
#include "nakama-c/data/NTournamentRecordList.h"
#include "nakama-c/data/NTournamentList.h"

#include "nakama-cpp/NError.h"
#include "nakama-cpp/data/NAccount.h"
#include "nakama-cpp/data/NUsers.h"
#include "nakama-cpp/data/NFriendList.h"
#include "nakama-cpp/data/NGroup.h"
#include "nakama-cpp/data/NGroupUserList.h"
#include "nakama-cpp/data/NUserGroupList.h"
#include "nakama-cpp/data/NGroupList.h"
#include "nakama-cpp/data/NLeaderboardRecordList.h"
#include "nakama-cpp/data/NMatchList.h"
#include "nakama-cpp/data/NRpc.h"
#include "nakama-cpp/data/NStorageObjectId.h"
#include "nakama-cpp/data/NStorageObject.h"
#include "nakama-cpp/data/NStorageObjectWrite.h"
#include "nakama-cpp/data/NStorageObjectAck.h"
#include "nakama-cpp/data/NStorageObjectList.h"
#include "nakama-cpp/data/NTournamentRecordList.h"
#include "nakama-cpp/data/NTournamentList.h"

NAKAMA_NAMESPACE_BEGIN

void assign(std::vector<std::string>& strings, const char** cStrings, uint16_t count);
void assign(std::vector<Nakama::NDeleteStorageObjectId>& objectIds, const sNDeleteStorageObjectId* cObjectIds, uint16_t count);
void assign(std::vector<Nakama::NReadStorageObjectId>& objectIds, const sNReadStorageObjectId* cObjectIds, uint16_t count);
void assign(Nakama::NDeleteStorageObjectId& objectId, const sNDeleteStorageObjectId& cObjectId);
void assign(Nakama::NStorageObjectWrite& cppObject, const sNStorageObjectWrite* object);
void assign(std::vector<Nakama::NStorageObjectWrite>& cppObjects, const sNStorageObjectWrite* objects, uint16_t objectsCount);

void assign(tNError& cError, const Nakama::NError& error);
void assign(sNAccount& cAccount, const Nakama::NAccount& account);
void assign(sNUsers& cUsers, const Nakama::NUsers& users);
void assign(sNFriendList& cFriends, const Nakama::NFriendList& friends);
void assign(sNGroup& cGroup, const Nakama::NGroup& group);
void assign(sNGroupUserList& cGroupUserList, const Nakama::NGroupUserList& groupUserList);
void assign(sNUserGroup& cGroup, const Nakama::NUserGroup& group);
void assign(sNUserGroupList& cGroupList, const Nakama::NUserGroupList& groupList);
void assign(sNGroupList& cGroupList, const Nakama::NGroupList& groupList);
void assign(sNLeaderboardRecord& cRecord, const Nakama::NLeaderboardRecord& recordList);
void assign(sNLeaderboardRecordList& cRecordList, const Nakama::NLeaderboardRecordList& recordList);
void assign(sNMatch& cMatch, const Nakama::NMatch& match);
void assign(sNMatchList& cMatchList, const Nakama::NMatchList& matchList);
void assign(sNRpc& cRpc, const Nakama::NRpc& rpc);
void assign(sNReadStorageObjectId& cObjectId, const Nakama::NReadStorageObjectId& objectId);
void assign(sNStorageObject* cObject, const NStorageObject& object);
void assign(sNStorageObject*& cObjects, uint16_t& count, const Nakama::NStorageObjects& objects);
void assign(sNStorageObjectAck* cAck, const Nakama::NStorageObjectAck& ack);
void assign(sNStorageObjectAck*& cAcks, uint16_t& count, const Nakama::NStorageObjectAcks& acks);
void assign(sNStorageObject* cObject, const Nakama::NStorageObject& object);
void assign(sNStorageObjectList& cObjList, const Nakama::NStorageObjectList& objList);
void assign(sNTournamentRecordList& cRecordList, const Nakama::NTournamentRecordList& recordList);
void assign(sNTournamentList& cList, const Nakama::NTournamentList& list);

void sNAccountDevice_free(sNAccountDevice& cDevice);
void sNAccount_free(sNAccount& cAccount);
void sNUsers_free(sNUsers& cUsers);
void sNFriendList_free(sNFriendList& cFriends);
void sNGroupUserList_free(sNGroupUserList& cGroupUserList);
void sNUserGroupList_free(sNUserGroupList& cUserGroupList);
void sNGroupList_free(sNGroupList& cGroupList);
void sNLeaderboardRecordList_free(sNLeaderboardRecordList& cRecordList);
void sNMatch_free(sNMatch& cMatch);
void sNMatchList_free(sNMatchList& cMatchList);
void sNStorageObjects_free(sNStorageObject* cObjects);
void sNStorageObjectAcks_free(sNStorageObjectAck* cAcks);
void sNStorageObjectList_free(sNStorageObjectList* cObjList);
void sNTournamentRecordList_free(sNTournamentRecordList& cRecordList);
void sNTournamentList_free(sNTournamentList& cList);

NAKAMA_NAMESPACE_END
