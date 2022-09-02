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

#include "nakama-cpp/data/NAccount.h"
#include "nakama-cpp/data/NGroup.h"
#include "nakama-cpp/data/NGroupList.h"
#include "nakama-cpp/data/NGroupUserList.h"
#include "nakama-cpp/data/NUsers.h"
#include "nakama-cpp/data/NUserGroupList.h"
#include "nakama-cpp/data/NFriendList.h"
#include "nakama-cpp/data/NLeaderboardRecordList.h"
#include "nakama-cpp/data/NMatchList.h"
#include "nakama-cpp/data/NNotificationList.h"
#include "nakama-cpp/data/NChannelMessageList.h"
#include "nakama-cpp/data/NTournamentList.h"
#include "nakama-cpp/data/NTournamentRecordList.h"
#include "nakama-cpp/data/NStorageObjectList.h"
#include "nakama-cpp/data/NStorageObjectAck.h"
#include "nakama-cpp/data/NRpc.h"
#include "api/api.pb.h"

#include "nakama-cpp/realtime/rtdata/NChannelMessageAck.h"
#include "nakama-cpp/realtime/rtdata/NChannel.h"
#include "nakama-cpp/realtime/rtdata/NChannelPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NMatchmakerTicket.h"
#include "nakama-cpp/realtime/rtdata/NMatchPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NMatchmakerMatched.h"
#include "nakama-cpp/realtime/rtdata/NMatchData.h"
#include "nakama-cpp/realtime/rtdata/NStatus.h"
#include "nakama-cpp/realtime/rtdata/NStatusPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NRtError.h"
#include "nakama-cpp/realtime/rtdata/NStreamData.h"
#include "nakama-cpp/realtime/rtdata/NStreamPresenceEvent.h"
#include "nakama-cpp/realtime/rtdata/NParty.h"
#include "nakama-cpp/realtime/rtdata/NPartyClose.h"
#include "nakama-cpp/realtime/rtdata/NPartyData.h"
#include "nakama-cpp/realtime/rtdata/NPartyJoinRequest.h"

#include "nakama-cpp/realtime/rtdata/NPartyLeader.h"
#include "nakama-cpp/realtime/rtdata/NPartyMatchmakerTicket.h"
#include "nakama-cpp/realtime/rtdata/NPartyPresenceEvent.h"

#include "rtapi/realtime.pb.h"

namespace Nakama {

    void assign(NTimestamp& time, const ::google::protobuf::Timestamp& data);
    void assign(bool& b, const ::google::protobuf::BoolValue& data);
    void assign(int32_t& value, const ::google::protobuf::Int32Value& data);
    void assign(NUserGroupState& state, const ::google::protobuf::Int32Value& data);
    void assign(std::string& str, const ::google::protobuf::StringValue& data);
    void assign(NBytes& bytes, const std::string& str);
    void assign(NAccount& account, const nakama::api::Account& data);
    void assign(NUser& user, const nakama::api::User& data);
    void assign(NAccountDevice& device, const nakama::api::AccountDevice& data);
    void assign(NGroup& group, const nakama::api::Group& data);
    void assign(NGroupList& groups, const nakama::api::GroupList& data);
    void assign(NGroupUserList& users, const nakama::api::GroupUserList& data);
    void assign(NGroupUser& user, const nakama::api::GroupUserList_GroupUser& data);
    void assign(NUserGroup& group, const nakama::api::UserGroupList_UserGroup& data);
    void assign(NUserGroupList& users, const nakama::api::UserGroupList& data);
    void assign(NUsers& users, const nakama::api::Users& data);
    void assign(NFriend& afriend, const nakama::api::Friend& data);
    void assign(NFriendList& friends, const nakama::api::FriendList& data);
    void assign(NLeaderboardRecordList& list, const nakama::api::LeaderboardRecordList& data);
    void assign(NLeaderboardRecord& record, const nakama::api::LeaderboardRecord& data);
    void assign(NMatchList& list, const nakama::api::MatchList& data);
    void assign(NMatch& match, const nakama::api::Match& data);
    void assign(NNotificationList& list, const nakama::api::NotificationList& data);
    void assign(NNotification& notif, const nakama::api::Notification& data);
    void assign(NChannelMessageList& list, const nakama::api::ChannelMessageList& data);
    void assign(NChannelMessage& msg, const nakama::api::ChannelMessage& data);
    void assign(NTournamentList& list, const nakama::api::TournamentList& data);
    void assign(NTournament& tournament, const nakama::api::Tournament& data);
    void assign(NTournamentRecordList& list, const nakama::api::TournamentRecordList& data);
    void assign(NStorageObjectList& list, const nakama::api::StorageObjectList& data);
    void assign(NStorageObject& obj, const nakama::api::StorageObject& data);
    void assign(NStorageObjectAck& ack, const nakama::api::StorageObjectAck& data);
    void assign(NStoragePermissionRead& perm, const ::google::protobuf::int32& data);
    void assign(NStoragePermissionWrite& perm, const ::google::protobuf::int32& data);
    void assign(NRpc& rpc, const nakama::api::Rpc& data);

    void assign(NChannelMessageAck& ack, const ::nakama::realtime::ChannelMessageAck& data);
    void assign(NChannel& channel, const ::nakama::realtime::Channel& data);
    void assign(NChannelPresenceEvent& event, const ::nakama::realtime::ChannelPresenceEvent& data);
    void assign(NUserPresence& presence, const ::nakama::realtime::UserPresence& data);
    void assign(NMatch& match, const ::nakama::realtime::Match& data);
    void assign(NMatchData& match_data, const ::nakama::realtime::MatchData& data);
    void assign(NMatchmakerTicket& ticket, const ::nakama::realtime::MatchmakerTicket& data);
    void assign(NMatchPresenceEvent& event, const ::nakama::realtime::MatchPresenceEvent& data);
    void assign(NMatchmakerMatched& matched, const ::nakama::realtime::MatchmakerMatched& data);
    void assign(NMatchmakerUser& user, const ::nakama::realtime::MatchmakerMatched_MatchmakerUser& data);
    void assign(NNotificationList& list, const ::nakama::realtime::Notifications& data);
    void assign(NStatus& status, const ::nakama::realtime::Status& data);
    void assign(NStatusPresenceEvent& event, const ::nakama::realtime::StatusPresenceEvent& data);
    void assign(NRtError& error, const ::nakama::realtime::Error& data);
    void assign(NStream& stream, const ::nakama::realtime::Stream& data);
    void assign(NStreamData& streamData, const ::nakama::realtime::StreamData& data);
    void assign(NStreamPresenceEvent& event, const ::nakama::realtime::StreamPresenceEvent& data);
    void assign(NPartyMatchmakerTicket& ticket, const ::nakama::realtime::PartyMatchmakerTicket& data);
    void assign(NParty& party, const ::nakama::realtime::Party& data);
    void assign(NPartyJoinRequest& partyJoinRequest, const ::nakama::realtime::PartyJoinRequest& data);
    void assign(NPartyClose & partyClose,  const::nakama::realtime::PartyClose & data);
    void assign(NPartyData & partyData,  const::nakama::realtime::PartyData & data);
    void assign(NPartyLeader & partyLeader,  const::nakama::realtime::PartyLeader & data);
    void assign(NPartyPresenceEvent & partyPresenceEvent,  const::nakama::realtime::PartyPresenceEvent & data);

    template <class T>
    void assign(T& b, const T& data)
    {
        b = data;
    }

    template <class T, class B>
    void assign(T& b, const ::google::protobuf::RepeatedPtrField<B>& data)
    {
        b.resize(data.size());

        size_t i = 0;
        for (auto& item : data)
        {
            assign(b[i++], item);
        }
    }

    template <class A, class B>
    void assign(std::map<A, B>& b, const ::google::protobuf::Map<A, B>& data)
    {
        b.clear();

        for (auto& it : data)
        {
            b.emplace(it.first, it.second);
        }
    }

}
