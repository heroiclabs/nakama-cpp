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

void assign(NBytes & bytes, const std::string & str)
{
    size_t i = 0;

    bytes.resize(str.size());

    for (auto c : str)
    {
        bytes[i++] = c;
    }
}

void assign(NAccount& account, const nakama::api::Account& data)
{
    assign(account.user.id, data.user().id());
    assign(account.customId, data.custom_id());
    assign(account.email, data.email());
    assign(account.verifyTime, data.verify_time());
    assign(account.disableTime, data.disable_time());
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
    assign(user.appleId, data.apple_id());
    assign(user.steamId, data.steam_id());
    assign(user.online, data.online());
    assign(user.edgeCount, data.edge_count());
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
    assign(group.creatorId, data.creator_id());
    assign(group.name, data.name());
    assign(group.description, data.description());
    assign(group.lang, data.lang_tag());
    assign(group.metadata, data.metadata());
    assign(group.avatarUrl, data.avatar_url());
    assign(group.open, data.open());
    assign(group.edgeCount, data.edge_count());
    assign(group.maxCount, data.max_count());
    assign(group.createTime, data.create_time());
    assign(group.updateTime, data.update_time());
}

void assign(NGroupList & groups, const nakama::api::GroupList & data)
{
    assign(groups.groups, data.groups());
    assign(groups.cursor, data.cursor());
}

void assign(NGroupUserList & users, const nakama::api::GroupUserList & data)
{
    assign(users.groupUsers, data.group_users());
    assign(users.cursor, data.cursor());
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
    assign(users.userGroups, data.user_groups());
    assign(users.cursor, data.cursor());
}

void assign(NUsers & users, const nakama::api::Users & data)
{
    assign(users.users, data.users());
}

void assign(NFriend & afriend, const nakama::api::Friend & data)
{
    assign(afriend.user, data.user());
    afriend.state = static_cast<NFriend::State>(data.state().value());
    assign(afriend.updateTime, data.update_time());
}

void assign(NFriendList & friends, const nakama::api::FriendList & data)
{
    assign(friends.friends, data.friends());
    assign(friends.cursor, data.cursor());
}

void assign(NLeaderboardRecordList & list, const nakama::api::LeaderboardRecordList & data)
{
    assign(list.nextCursor, data.next_cursor());
    assign(list.prevCursor, data.prev_cursor());
    assign(list.ownerRecords, data.owner_records());
    assign(list.records, data.records());
}

void assign(NLeaderboardRecord & record, const nakama::api::LeaderboardRecord & data)
{
    assign(record.leaderboardId, data.leaderboard_id());
    assign(record.ownerId, data.owner_id());
    assign(record.username, data.username());
    assign(record.score, data.score());
    assign(record.subscore, data.subscore());
    assign(record.numScore, data.num_score());
    assign(record.maxNumScore, data.max_num_score());
    assign(record.metadata, data.metadata());
    assign(record.createTime, data.create_time());
    assign(record.updateTime, data.update_time());
    assign(record.expiryTime, data.expiry_time());
    assign(record.rank, data.rank());
}

void assign(NMatchList & list, const nakama::api::MatchList & data)
{
    assign(list.matches, data.matches());
}

void assign(NMatch & match, const nakama::api::Match & data)
{
    assign(match.matchId, data.match_id());
    assign(match.size, data.size());
    assign(match.authoritative, data.authoritative());
    assign(match.label, data.label());
}

void assign(NMatch& match, const ::nakama::realtime::Match& data)
{
    assign(match.matchId, data.match_id());
    assign(match.size, data.size());
    assign(match.authoritative, data.authoritative());
    assign(match.label, data.label());
    assign(match.self, data.self());
    assign(match.presences, data.presences());
}

void assign(NMatchData & match_data, const::nakama::realtime::MatchData & data)
{
    assign(match_data.matchId, data.match_id());
    assign(match_data.opCode, data.op_code());
    assign(match_data.presence, data.presence());
    assign(match_data.data, data.data());
}

void assign(NMatchmakerTicket & ticket, const::nakama::realtime::MatchmakerTicket & data)
{
    assign(ticket.ticket, data.ticket());
}

void assign(NMatchPresenceEvent & event, const::nakama::realtime::MatchPresenceEvent & data)
{
    assign(event.matchId, data.match_id());
    assign(event.joins, data.joins());
    assign(event.leaves, data.leaves());
}

void assign(NMatchmakerMatched & matched, const::nakama::realtime::MatchmakerMatched & data)
{
    assign(matched.matchId, data.match_id());
    assign(matched.ticket, data.ticket());
    assign(matched.token, data.token());
    assign(matched.self, data.self());
    assign(matched.users, data.users());
}

void assign(NMatchmakerUser & user, const::nakama::realtime::MatchmakerMatched_MatchmakerUser & data)
{
    assign(user.presence, data.presence());
    assign(user.stringProperties, data.string_properties());
    assign(user.numericProperties, data.numeric_properties());
}

void assign(NNotificationList & list, const::nakama::realtime::Notifications & data)
{
    assign(list.notifications, data.notifications());
}

void assign(NStatus & status, const::nakama::realtime::Status & data)
{
    assign(status.presences, data.presences());
}

void assign(NStatusPresenceEvent & event, const::nakama::realtime::StatusPresenceEvent & data)
{
    assign(event.joins, data.joins());
    assign(event.leaves, data.leaves());
}

void assign(NRtError & error, const::nakama::realtime::Error & data)
{
    assign(error.message, data.message());
    error.code = static_cast<RtErrorCode>(data.code());

    for (auto& it : data.context())
    {
        error.context.emplace(it.first, it.second);
    }
}

void assign(NStream & stream, const::nakama::realtime::Stream & data)
{
    assign(stream.label, data.label());
    assign(stream.mode, data.mode());
    assign(stream.subcontext, data.subcontext());
    assign(stream.subject, data.subject());
}

void assign(NStreamData & streamData, const::nakama::realtime::StreamData & data)
{
    assign(streamData.sender, data.sender());
    assign(streamData.data, data.data());
    assign(streamData.stream, data.stream());
}

void assign(NStreamPresenceEvent & event, const::nakama::realtime::StreamPresenceEvent & data)
{
    assign(event.stream, data.stream());
    assign(event.joins, data.joins());
    assign(event.leaves, data.leaves());
}

void assign(NNotificationList & list, const nakama::api::NotificationList & data)
{
    assign(list.notifications, data.notifications());
    assign(list.cacheableCursor, data.cacheable_cursor());
}

void assign(NNotification & notif, const nakama::api::Notification & data)
{
    assign(notif.id, data.id());
    assign(notif.subject, data.subject());
    assign(notif.content, data.content());
    assign(notif.code, data.code());
    assign(notif.senderId, data.sender_id());
    assign(notif.createTime, data.create_time());
    assign(notif.persistent, data.persistent());
}

void assign(NChannelMessageList & list, const nakama::api::ChannelMessageList & data)
{
    assign(list.messages, data.messages());
    assign(list.prevCursor, data.prev_cursor());
    assign(list.nextCursor, data.next_cursor());
}

void assign(NChannelMessage & msg, const nakama::api::ChannelMessage & data)
{
    assign(msg.channelId, data.channel_id());
    assign(msg.code, data.code());
    assign(msg.content, data.content());
    assign(msg.createTime, data.create_time());
    assign(msg.messageId, data.message_id());
    assign(msg.persistent, data.persistent());
    assign(msg.senderId, data.sender_id());
    assign(msg.updateTime, data.update_time());
    assign(msg.username, data.username());
    assign(msg.roomName, data.room_name());
    assign(msg.groupId, data.group_id());
    assign(msg.userIdOne, data.user_id_one());
    assign(msg.userIdTwo, data.user_id_two());
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
    assign(tournament.sortOrder, data.sort_order());
    assign(tournament.size, data.size());
    assign(tournament.maxSize, data.max_size());
    assign(tournament.maxNumScore, data.max_num_score());
    assign(tournament.canEnter ,data.can_enter());
    assign(tournament.createTime, data.create_time());
    assign(tournament.startTime, data.start_time());
    assign(tournament.endTime, data.end_time());
    assign(tournament.endActive, data.end_active());
    assign(tournament.nextReset, data.next_reset());
    assign(tournament.duration, data.duration());
    assign(tournament.startActive, data.start_active());
    assign(tournament.metadata, data.metadata());
}

void assign(NTournamentRecordList & list, const nakama::api::TournamentRecordList & data)
{
    assign(list.records, data.records());
    assign(list.ownerRecords, data.owner_records());
    assign(list.nextCursor, data.next_cursor());
    assign(list.prevCursor, data.prev_cursor());
}

void assign(NStorageObjectList & list, const nakama::api::StorageObjectList & data)
{
    assign(list.objects, data.objects());
    assign(list.cursor, data.cursor());
}

void assign(NStorageObject & obj, const nakama::api::StorageObject & data)
{
    assign(obj.collection, data.collection());
    assign(obj.createTime, data.create_time());
    assign(obj.key, data.key());
    assign(obj.permissionRead, data.permission_read());
    assign(obj.permissionWrite, data.permission_write());
    assign(obj.updateTime, data.update_time());
    assign(obj.userId, data.user_id());
    assign(obj.value, data.value());
    assign(obj.version, data.version());
}

void assign(NStorageObjectAck & ack, const nakama::api::StorageObjectAck & data)
{
    assign(ack.collection, data.collection());
    assign(ack.key, data.key());
    assign(ack.userId, data.user_id());
    assign(ack.version, data.version());
}

void assign(NStoragePermissionRead & perm, const ::google::protobuf::int32 & data)
{
    perm = static_cast<NStoragePermissionRead>(data);
}

void assign(NStoragePermissionWrite & perm, const ::google::protobuf::int32 & data)
{
    perm = static_cast<NStoragePermissionWrite>(data);
}

void assign(NRpc & rpc, const nakama::api::Rpc & data)
{
    assign(rpc.id, data.id());
    assign(rpc.payload, data.payload());
    assign(rpc.httpKey, data.http_key());
}

void assign(NChannelMessageAck & ack, const::nakama::realtime::ChannelMessageAck & data)
{
    assign(ack.channelId, data.channel_id());
    assign(ack.code, data.code());
    assign(ack.createTime, data.create_time());
    assign(ack.messageId, data.message_id());
    assign(ack.persistent, data.persistent());
    assign(ack.updateTime, data.update_time());
    assign(ack.username, data.username());
    assign(ack.roomName, data.room_name());
    assign(ack.groupId, data.group_id());
    assign(ack.userIdOne, data.user_id_one());
    assign(ack.userIdTwo, data.user_id_two());
}

void assign(NChannel & channel, const::nakama::realtime::Channel & data)
{
    assign(channel.id, data.id());
    assign(channel.self, data.self());
    assign(channel.presences, data.presences());
    assign(channel.roomName, data.room_name());
    assign(channel.groupId, data.group_id());
    assign(channel.userIdOne, data.user_id_one());
    assign(channel.userIdTwo, data.user_id_two());
}

void assign(NChannelPresenceEvent & event, const::nakama::realtime::ChannelPresenceEvent & data)
{
    assign(event.channelId, data.channel_id());
    assign(event.joins, data.joins());
    assign(event.leaves, data.leaves());
    assign(event.roomName, data.room_name());
    assign(event.groupId, data.group_id());
    assign(event.userIdOne, data.user_id_one());
    assign(event.userIdTwo, data.user_id_two());
}

void assign(NUserPresence & presence, const::nakama::realtime::UserPresence & data)
{
    assign(presence.persistence, data.persistence());
    assign(presence.sessionId, data.session_id());
    assign(presence.status, data.status());
    assign(presence.username, data.username());
    assign(presence.userId, data.user_id());
}

}
