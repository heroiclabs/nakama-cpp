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

#include <condition_variable>
#include <mutex>
#include <crossguid/guid.hpp>
#include "test_main.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_listFriends()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    const size_t numFriends = 20;
    std::vector<string> friendIds(numFriends);
    NSessionPtr session = nullptr;

    std::mutex mutex;

    auto successCallback = [&mutex, &session](NSessionPtr sessionResponse) -> void
    {
        session = sessionResponse;
    };

    auto failureCallback = [&test](NError error) -> void
    {
        test.stopTest(error);
    };

    test.client.get()->authenticateCustom(xg::newGuid().str(), "", true, {}, successCallback, failureCallback);

    lock.
    int numAuths = 0;

    for (int i = 0; i < numFriends; i++)
    {
        auto successCallback = [&cv, &mutex, &numAuths](NSessionPtr session) -> void
        {
            std::lock_guard(mutex);
            numAuths++;
            cv.notify_one();
        };

        auto failureCallback = [&test](NError error) -> void
        {
            test.stopTest(error);
        };

        friendIds[i] = xg::newGuid().str();
        test.client->authenticateCustom(friendIds[i], "", true, {}, successCallback, failureCallback);
    }

    cv.wait(lock);

    auto successCallback = [&cv, &mutex, &numAuths](NSessionPtr session) -> void
    {
        std::lock_guard(mutex);
        numAuths++;
        if (numAuths >= numFriends) {
            cv.notify_one();
        }
    };

    auto failureCallback = [&test](NError error) -> void
    {
        test.stopTest(error);
    };

    test.client->addFriends(test.client.[i], friendIds, {}, successCallback, failureCallback);

    cv.wait(lock, []{ return true; })
    test.runTest();
}

void test_friends()
{
    test_listFriends();
}

} // namespace Test
} // namespace Nakama
