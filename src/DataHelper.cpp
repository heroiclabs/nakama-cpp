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

#include "DataHelper.h"

using namespace std;

namespace Nakama {

void assign(NTimestamp& time, const google::protobuf::Timestamp& data)
{
    time = data.seconds() * 1000;
}

void assign(bool & b, const::google::protobuf::BoolValue & data)
{
    b = data.value();
}

void assign(int32_t & value, const::google::protobuf::Int32Value & data)
{
    value = data.value();
}

void assign(NUserGroupState & state, const::google::protobuf::Int32Value & data)
{
    state = static_cast<NUserGroupState>(data.value());
}

void assign(std::string & str, const::google::protobuf::StringValue & data)
{
    if (data.IsInitialized())
        str = data.value();
    else
        str.clear();
}

void assign(NAccount& account, const nakama::api::Account& data)
{
    assign(account.user.id, data.user().id());
    assign(account.custom_id, data.custom_id());
    assign(account.email, data.email());
    assign(account.verify_time, data.verify_time());
    assign(account.wallet, data.wallet());
    assign(account.user, data.user());
    assign(account.devices, data.devices());
}

void assign(NUser& user, const nakama::api::User& data)
{
    assign(user.id, data.id());
    assign(user.username, data.username());
    assign(user.displayName, data.display_name());
    assign(user.avatarUrl, data.avatar_url());
    assign(user.lang, data.lang_tag());
    assign(user.location, data.location());
    assign(user.timeZone, data.timezone());
    assign(user.metadata, data.metadata());
    assign(user.facebookId, data.facebook_id());
    assign(user.googleId, data.google_id());
    assign(user.gameCenterId, data.gamecenter_id());
    assign(user.steamId, data.steam_id());
    assign(user.online, data.online());
    assign(user.edge_count, data.edge_count());
    assign(user.createdAt, data.create_time());
    assign(user.updatedAt, data.update_time());
}

void assign(NAccountDevice & device, const nakama::api::AccountDevice & data)
{
    assign(device.id, data.id());
}

void assign(NGroup& group, const nakama::api::Group& data)
{
    assign(group.id, data.id());
    assign(group.creator_id, data.creator_id());
    assign(group.name, data.name());
    assign(group.description, data.description());
    assign(group.lang, data.lang_tag());
    assign(group.metadata, data.metadata());
    assign(group.avatar_url, data.avatar_url());
    assign(group.open, data.open());
    assign(group.edge_count, data.edge_count());
    assign(group.max_count, data.max_count());
    assign(group.create_time, data.create_time());
    assign(group.update_time, data.update_time());
}

void assign(NGroupList & groups, const nakama::api::GroupList & data)
{
    assign(groups.groups, data.groups());
    assign(groups.cursor, data.cursor());
}

void assign(NGroupUserList & users, const nakama::api::GroupUserList & data)
{
    assign(users.group_users, data.group_users());
}

void assign(NGroupUser & user, const nakama::api::GroupUserList_GroupUser & data)
{
    assign(user.user, data.user());
    assign(user.state, data.state());
}

void assign(NUserGroup & group, const nakama::api::UserGroupList_UserGroup & data)
{
    assign(group.group, data.group());
    assign(group.state, data.state());
}

void assign(NUserGroupList & users, const nakama::api::UserGroupList & data)
{
    assign(users.user_groups, data.user_groups());
}

void assign(NUsers & users, const nakama::api::Users & data)
{
    assign(users.users, data.users());
}

void assign(NFriend & afriend, const nakama::api::Friend & data)
{
    assign(afriend.user, data.user());
    afriend.state = static_cast<NFriend::State>(data.state().value());
}

void assign(NFriends & friends, const nakama::api::Friends & data)
{
    assign(friends.friends, data.friends());
}

void assign(NLeaderboardRecordList & list, const nakama::api::LeaderboardRecordList & data)
{
    assign(list.next_cursor, data.next_cursor());
    assign(list.prev_cursor, data.prev_cursor());
    assign(list.owner_records, data.owner_records());
    assign(list.records, data.records());
}

void assign(NLeaderboardRecord & record, const nakama::api::LeaderboardRecord & data)
{
    assign(record.leaderboard_id, data.leaderboard_id());
    assign(record.owner_id, data.owner_id());
    assign(record.username, data.username());
    assign(record.score, data.score());
    assign(record.subscore, data.subscore());
    assign(record.num_score, data.num_score());
    assign(record.max_num_score, data.max_num_score());
    assign(record.metadata, data.metadata());
    assign(record.create_time, data.create_time());
    assign(record.update_time, data.update_time());
    assign(record.expiry_time, data.expiry_time());
    assign(record.rank, data.rank());
}

void assign(NMatchList & list, const nakama::api::MatchList & data)
{
    assign(list.matches, data.matches());
}

void assign(NMatch & match, const nakama::api::Match & data)
{
    assign(match.match_id, data.match_id());
    assign(match.size, data.size());
    assign(match.authoritative, data.authoritative());
    assign(match.label, data.label());
}

void assign(NNotificationList & list, const nakama::api::NotificationList & data)
{
    assign(list.notifications, data.notifications());
    assign(list.cacheable_cursor, data.cacheable_cursor());
}

void assign(NNotification & notif, const nakama::api::Notification & data)
{
    assign(notif.id, data.id());
    assign(notif.subject, data.subject());
    assign(notif.content, data.content());
    assign(notif.code, data.code());
    assign(notif.sender_id, data.sender_id());
    assign(notif.create_time, data.create_time());
    assign(notif.persistent, data.persistent());
}

void assign(NChannelMessageList & list, const nakama::api::ChannelMessageList & data)
{
    assign(list.messages, data.messages());
    assign(list.prev_cursor, data.prev_cursor());
    assign(list.next_cursor, data.next_cursor());
}

void assign(NChannelMessage & msg, const nakama::api::ChannelMessage & data)
{
    assign(msg.channel_id, data.channel_id());
    assign(msg.code, data.code());
    assign(msg.content, data.content());
    assign(msg.create_time, data.create_time());
    assign(msg.message_id, data.message_id());
    assign(msg.persistent, data.persistent());
    assign(msg.sender_id, data.sender_id());
    assign(msg.update_time, data.update_time());
    assign(msg.username, data.username());
}

void assign(NTournamentList & list, const nakama::api::TournamentList & data)
{
    assign(list.tournaments, data.tournaments());
    assign(list.cursor, data.cursor());
}

void assign(NTournament & tournament, const nakama::api::Tournament & data)
{
    assign(tournament.id, data.id());
    assign(tournament.title, data.title());
    assign(tournament.description, data.description());
    assign(tournament.category, data.category());
    assign(tournament.sort_order, data.sort_order());
    assign(tournament.size, data.size());
    assign(tournament.max_size, data.max_size());
    assign(tournament.max_num_score, data.max_num_score());
    assign(tournament.can_enter ,data.can_enter());
    assign(tournament.create_time, data.create_time());
    assign(tournament.start_time, data.start_time());
    assign(tournament.end_time, data.end_time());
    assign(tournament.end_active, data.end_active());
    assign(tournament.next_reset, data.next_reset());
    assign(tournament.duration, data.duration());
    assign(tournament.metadata, data.metadata());
}

void assign(NTournamentRecordList & list, const nakama::api::TournamentRecordList & data)
{
    assign(list.records, data.records());
    assign(list.owner_records, data.owner_records());
    assign(list.next_cursor, data.next_cursor());
    assign(list.prev_cursor, data.prev_cursor());
}

}
