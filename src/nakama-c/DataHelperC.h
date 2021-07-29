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
#include "nakama-c/NStringMap.h"
#include "nakama-c/NStringDoubleMap.h"
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
#include "nakama-c/data/NNotificationList.h"
#include "nakama-c/data/NChannelMessageList.h"
#include "nakama-c/realtime/rtdata/NRtError.h"
#include "nakama-c/realtime/rtdata/NChannel.h"
#include "nakama-c/realtime/rtdata/NStatus.h"
#include "nakama-c/realtime/rtdata/NMatchmakerTicket.h"
#include "nakama-c/realtime/rtdata/NChannelMessageAck.h"
#include "nakama-c/realtime/rtdata/NChannelPresenceEvent.h"
#include "nakama-c/realtime/rtdata/NMatchmakerMatched.h"
#include "nakama-c/realtime/rtdata/NMatchData.h"
#include "nakama-c/realtime/rtdata/NMatchPresenceEvent.h"
#include "nakama-c/realtime/rtdata/NStatusPresenceEvent.h"
#include "nakama-c/realtime/rtdata/NStreamPresenceEvent.h"
#include "nakama-c/realtime/rtdata/NStreamData.h"
#include "nakama-c/realtime/rtdata/NParty.h"
#include "nakama-c/realtime/rtdata/NPartyClose.h"
#include "nakama-c/realtime/rtdata/NPartyData.h"
#include "nakama-c/realtime/rtdata/NPartyJoinRequest.h"
#include "nakama-c/realtime/rtdata/NPartyLeader.h"
#include "nakama-c/realtime/rtdata/NPartyMatchmakerTicket.h"
#include "nakama-c/realtime/rtdata/NPartyPresenceEvent.h"
#include "nakama-c/realtime/NRtClientDisconnectInfo.h"

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
#include "nakama-cpp/data/NNotificationList.h"
#include "nakama-cpp/data/NChannelMessageList.h"
#include "nakama-cpp/realtime/rtdata/NRtError.h"
#include "nakama-cpp/realtime/rtdata/NChannel.h"
#include "nakama-cpp/realtime/rtdata/NStatus.h"
#include "nakama-cpp/realtime/rtdata/NMatchmakerTicket.h"
#include "nakama-cpp/realtime/rtdata/NChannelMessageAck.h"
#include "nakama-cpp/realtime/rtdata/NChannelPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NMatchmakerMatched.h"
#include "nakama-cpp/realtime/rtdata/NMatchData.h"
#include "nakama-cpp/realtime/rtdata/NMatchPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NStatusPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NStreamPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NStreamData.h"
#include "nakama-cpp/realtime/rtdata/NParty.h"
#include "nakama-cpp/realtime/rtdata/NPartyClose.h"
#include "nakama-cpp/realtime/rtdata/NPartyData.h"
#include "nakama-cpp/realtime/rtdata/NPartyJoinRequest.h"
#include "nakama-cpp/realtime/rtdata/NPartyLeader.h"
#include "nakama-cpp/realtime/rtdata/NPartyMatchmakerTicket.h"
#include "nakama-cpp/realtime/rtdata/NPartyPresenceEvent.h"

#include "nakama-cpp/realtime/NRtClientDisconnectInfo.h"

NAKAMA_NAMESPACE_BEGIN

void assign(std::vector<std::string>& strings, const char** cStrings, uint16_t count);
void assign(std::vector<Nakama::NDeleteStorageObjectId>& objectIds, const sNDeleteStorageObjectId* cObjectIds, uint16_t count);
void assign(std::vector<Nakama::NReadStorageObjectId>& objectIds, const sNReadStorageObjectId* cObjectIds, uint16_t count);
void assign(Nakama::NDeleteStorageObjectId& objectId, const sNDeleteStorageObjectId& cObjectId);
void assign(Nakama::NStorageObjectWrite& cppObject, const sNStorageObjectWrite* object);
void assign(std::vector<Nakama::NStorageObjectWrite>& cppObjects, const sNStorageObjectWrite* objects, uint16_t objectsCount);
void assign(std::vector<Nakama::NUserPresence>& presences, const sNUserPresence* cPresences, uint16_t count);
void assign(Nakama::NUserPresence& presence, const sNUserPresence& cPresence);
void assign(Nakama::NBytes& data, const sNBytes* cData);
void assign(Nakama::NStringMap& map, ::NStringMap cMap);
void assign(Nakama::NStringDoubleMap& map, ::NStringDoubleMap cMap);

void assign(::NStringMap& cMap, const Nakama::NStringMap& map);
void assign(::NStringDoubleMap& cMap, const Nakama::NStringDoubleMap& map);
void assign(sNBytes& cData, const Nakama::NBytes& data);
void assign(sNError& cError, const Nakama::NError& error);
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
void assign(sNNotification* cN, const Nakama::NNotification& n);
void assign(sNNotificationList& cList, const Nakama::NNotificationList& list);
void assign(sNChannelMessage& cMsg, const Nakama::NChannelMessage& msg);
void assign(sNChannelMessageList& cList, const Nakama::NChannelMessageList& list);
void assign(sNRtError& cError, const NRtError& error);
void assign(sNChannel& cChannel, const NChannel& channel);
void assign(sNRtClientDisconnectInfo& cInfo, const Nakama::NRtClientDisconnectInfo& info);
void assign(sNStatus& cStatus, const Nakama::NStatus& status);
void assign(sNUserPresence& cPresence, const Nakama::NUserPresence& presence);
void assign(sNUserPresence*& cPresences, uint16_t& cPresencesCount, const std::vector<Nakama::NUserPresence>& presences);
void assign(sNMatchmakerTicket& cTicket, const Nakama::NMatchmakerTicket& ticket);
void assign(sNChannelMessageAck& cAck, const Nakama::NChannelMessageAck& ack);
void assign(sNChannelPresenceEvent& cPresence, const Nakama::NChannelPresenceEvent& presence);
void assign(sNMatchmakerUser& cUser, const Nakama::NMatchmakerUser& user);
void assign(sNMatchmakerUser*& cUsers, uint16_t& count, const std::vector<Nakama::NMatchmakerUser>& users);
void assign(sNMatchmakerMatched& cMatched, const Nakama::NMatchmakerMatched& matched);
void assign(sNMatchData& cData, const Nakama::NMatchData& data);
void assign(sNMatchPresenceEvent& cEvent, const Nakama::NMatchPresenceEvent& event);
void assign(sNStatusPresenceEvent& cEvent, const Nakama::NStatusPresenceEvent& event);
void assign(sNStream& cStream, const Nakama::NStream& stream);
void assign(sNStreamPresenceEvent& cEvent, const Nakama::NStreamPresenceEvent& event);
void assign(sNStreamData& cData, const Nakama::NStreamData& data);
void assign(sNParty& cParty, const Nakama::NParty& party);
void assign(sNPartyPresenceEvent& cEvent, const Nakama::NPartyPresenceEvent& event);
void assign(sNPartyData& cData, const Nakama::NPartyData& partyData);
void assign(sNPartyLeader& cLeader, const Nakama::NPartyLeader& partyLeader);
void assign(sNPartyMatchmakerTicket& cTicket, const Nakama::NPartyMatchmakerTicket& ticket);
void assign(sNPartyJoinRequest& cRequest, const Nakama::NPartyJoinRequest& request);

void sNRtError_free(sNRtError& cError);
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
void sNNotificationList_free(sNNotificationList& cList);
void sNChannelMessageList_free(sNChannelMessageList& cList);
void sNChannel_free(sNChannel& cChannel);
void sNStatus_free(sNStatus& cStatus);
void sNChannelPresenceEvent_free(sNChannelPresenceEvent& cPresence);
void sNMatchmakerUser_free(sNMatchmakerUser& user);
void sNMatchmakerMatched_free(sNMatchmakerMatched& cMatched);
void sNMatchPresenceEvent_free(sNMatchPresenceEvent& cEvent);
void sNStatusPresenceEvent_free(sNStatusPresenceEvent& cEvent);
void sNStreamPresenceEvent_free(sNStreamPresenceEvent& cEvent);
void sNPartyJoinRequest_free(sNPartyJoinRequest& cRequest);
void sNPartyMatchmakerTicket_free(sNPartyMatchmakerTicket& cTicket);
void sNPartyLeader_free(sNPartyLeader& cLeader);
void sNPartyPresenceEvent_free(sNPartyPresenceEvent& cEvent);
void sNPartyData_free(sNPartyData& cPartyData);
void sNParty_free(sNParty& cParty);

NAKAMA_NAMESPACE_END
