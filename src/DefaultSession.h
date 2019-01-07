/**
 * Copyright 2017 The Nakama Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
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

        const std::string& getAuthToken() override;
        bool isCreated() override;
        const std::string& getUsername() override;
        const std::string& getUserId() override;
        uint64_t getCreateTime() override;
        uint64_t getExpireTime() override;
        bool isExpired() override;
        bool isExpired(uint64_t now) override;

    private:
        std::string _token;
        std::string _user_id;
        std::string _username;
        bool _created = false;
        uint64_t _create_time = 0;
        uint64_t _expire_time = 0;
    };

}
