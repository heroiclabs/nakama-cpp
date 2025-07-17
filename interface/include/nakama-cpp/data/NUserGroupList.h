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

#include <nakama-cpp/NTypes.h>
#include <nakama-cpp/data/NUserGroup.h>
#include <vector>
#include <memory>

NAKAMA_NAMESPACE_BEGIN

    /// A list of groups belonging to a user, along with the user's role in each group.
    struct NUserGroupList
    {
        std::vector<NUserGroup> userGroups;     ///< Group-role pairs for a user.
        std::string cursor;                     ///< Cursor for the next page of results, if any.
    };

    using NUserGroupListPtr = std::shared_ptr<NUserGroupList>;

NAKAMA_NAMESPACE_END
