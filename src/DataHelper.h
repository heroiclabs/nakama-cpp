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
#include "api/github.com/heroiclabs/nakama/api/api.pb.h"

namespace Nakama {

    void assign(uint64_t& time, const google::protobuf::Timestamp& data);
    void assign(NAccount& account, const nakama::api::Account& data);
    void assign(NUser& user, const nakama::api::User& data);
    void assign(NAccountDevice& device, const nakama::api::AccountDevice& data);

    template <class T>
    void assign(T& b, const T& data)
    {
        b = data;
    }

    template <class T, class B>
    void assign(T& b, const google::protobuf::RepeatedPtrField<B>& data)
    {
        b.resize(data.size());

        int i = 0;
        for (auto it : data)
        {
            assign(b[i++], it);
        }
    }

}
