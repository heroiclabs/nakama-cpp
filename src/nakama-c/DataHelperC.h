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

#include "nakama-cpp/NError.h"
#include "nakama-cpp/data/NAccount.h"
#include "nakama-cpp/data/NUsers.h"
#include "nakama-cpp/data/NFriendList.h"
#include "nakama-cpp/data/NGroup.h"
#include "nakama-cpp/data/NGroupUserList.h"
#include "nakama-cpp/data/NUserGroupList.h"
#include "nakama-cpp/data/NGroupList.h"

NAKAMA_NAMESPACE_BEGIN

void assign(std::vector<std::string>& strings, const char** cStrings, uint16_t count);

void assign(tNError& cError, const Nakama::NError& error);
void assign(sNAccount& cAccount, const Nakama::NAccount& account);
void assign(sNUsers& cUsers, const Nakama::NUsers& users);
void assign(sNFriendList& cFriends, const Nakama::NFriendList& friends);
void assign(sNGroup& cGroup, const Nakama::NGroup& group);
void assign(sNGroupUserList& cGroupUserList, const Nakama::NGroupUserList& groupUserList);
void assign(sNUserGroup& cGroup, const Nakama::NUserGroup& group);
void assign(sNUserGroupList& cGroupList, const Nakama::NUserGroupList& groupList);
void assign(sNGroupList& cGroupList, const Nakama::NGroupList& groupList);

void sNAccountDevice_free(sNAccountDevice& cDevice);
void sNAccount_free(sNAccount& cAccount);
void sNUsers_free(sNUsers& cUsers);
void sNFriendList_free(sNFriendList& cFriends);
void sNGroupUserList_free(sNGroupUserList& cGroupUserList);
void sNUserGroupList_free(sNUserGroupList& cUserGroupList);
void sNGroupList_free(sNGroupList& cGroupList);

NAKAMA_NAMESPACE_END
