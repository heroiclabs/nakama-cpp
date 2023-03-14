/*
 * Copyright 2023 The Nakama Authors
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

#include <condition_variable>
#include <mutex>
#include "TestGuid.h"
#include "test_main.h"

namespace Nakama
{
    namespace Test {

        using namespace std;

        void test_listFriends()
        {
            NLOG_INFO("testing friends");
            NCppTest test(__func__, true);
            test.createWorkingClient();
            test.runTest();

            NLOG_INFO("about to call run test");

            NLOG_INFO("run test done being called");

            const size_t numFriends = 5;
            std::vector<string> friendIds(numFriends);


            Nakama::NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true, {}).get();

            for (int i = 0; i < numFriends; i++)
            {
                friendIds[i] = TestGuid::newGuid();
                // unfortunately, std::when_any is a C++23 feature.
                test.client->authenticateCustomAsync(friendIds[i], "", true, {}).get();
            }

            // test that using cursor gives a different friend.
            test.client->addFriendsAsync(session, friendIds, {}).get();
            const int limit = 1;
            Nakama::NFriendListPtr invitedList = test.client->listFriendsAsync(session, limit, Nakama::NFriend::State::INVITE_SENT).get();
            std::string returnedFriendId1 = invitedList->friends[0].user.id;
            invitedList = test.client->listFriendsAsync(session, limit, Nakama::NFriend::State::INVITE_SENT, invitedList->cursor).get();
            std::string returnedFriendId2 = invitedList->friends[0].user.id;

            test.stopTest(returnedFriendId1 != returnedFriendId2);
        }

        void test_friends()
        {
            test_listFriends();
        }

    }

} // namespace Nakama::Test
