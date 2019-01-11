/*
* Copyright 2018 The Nakama Authors
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
#include "nakama-cpp/data/NFriends.h"
#include "nakama-cpp/data/NLeaderboardRecordList.h"
#include "nakama-cpp/data/NMatchList.h"
#include "nakama-cpp/data/NNotificationList.h"
#include "nakama-cpp/data/NChannelMessageList.h"
#include "api/github.com/heroiclabs/nakama/api/api.pb.h"

namespace Nakama {

    void assign(NTimestamp& time, const ::google::protobuf::Timestamp& data);
    void assign(bool& b, const ::google::protobuf::BoolValue& data);
    void assign(int32_t& value, const ::google::protobuf::Int32Value& data);
    void assign(NUserGroupState& state, const ::google::protobuf::Int32Value& data);
    void assign(std::string& str, const ::google::protobuf::StringValue& data);
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
    void assign(NFriends& friends, const nakama::api::Friends& data);
    void assign(NLeaderboardRecordList& list, const nakama::api::LeaderboardRecordList& data);
    void assign(NLeaderboardRecord& record, const nakama::api::LeaderboardRecord& data);
    void assign(NMatchList& list, const nakama::api::MatchList& data);
    void assign(NMatch& match, const nakama::api::Match& data);
    void assign(NNotificationList& list, const nakama::api::NotificationList& data);
    void assign(NNotification& notif, const nakama::api::Notification& data);
    void assign(NChannelMessageList& list, const nakama::api::ChannelMessageList& data);
    void assign(NChannelMessage& msg, const nakama::api::ChannelMessage& data);

    template <class T>
    void assign(T& b, const T& data)
    {
        b = data;
    }

    template <class T, class B>
    void assign(T& b, const ::google::protobuf::RepeatedPtrField<B>& data)
    {
        b.resize(data.size());

        int i = 0;
        for (auto it : data)
        {
            assign(b[i++], it);
        }
    }

}
