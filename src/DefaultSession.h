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

#include "nakama-cpp/NSessionInterface.h"

namespace Nakama {

    class NAKAMA_API DefaultSession : public NSessionInterface
    {
    public:
        DefaultSession(const std::string& token, bool created);

        const std::string& getAuthToken() const override;
        bool isCreated() const override;
        const std::string& getUsername() const override;
        const std::string& getUserId() const override;
        NTimestamp getCreateTime() const override;
        NTimestamp getExpireTime() const override;
        bool isExpired() const override;
        bool isExpired(NTimestamp now) const override;

    private:
        std::string _token;
        std::string _user_id;
        std::string _username;
        bool _created = false;
        NTimestamp _create_time = 0;
        NTimestamp _expire_time = 0;
    };

}
