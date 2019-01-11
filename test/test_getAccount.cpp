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

#include "test_main.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_getAccount()
{
    cout << endl << __func__ << endl;

    DefaultClientParameters parameters;

    setWorkingClientParameters(parameters);

    ClientPtr client = createDefaultClient(parameters);
    bool continue_loop = true;
    NSessionPtr my_session;

    auto errorCallback = [&continue_loop](const NError& error)
    {
        std::cout << "error: " << error.GetErrorMessage() << std::endl;
        continue_loop = false;
    };

    auto successCallback = [client, &continue_loop, &my_session, errorCallback](NSessionPtr session)
    {
        my_session = session;
        std::cout << "session token: " << session->getAuthToken() << std::endl;

        auto successCallback = [&continue_loop](const NAccount& account)
        {
            std::cout << "account user id: " << account.user.id << std::endl;
            continue_loop = false;
        };

        client->getAccount(session, successCallback, errorCallback);
    };

    client->authenticateDevice("mytestdevice0000", "", true, successCallback, errorCallback);

    std::chrono::milliseconds sleep_period(15);

    while (continue_loop)
    {
        client->tick();

        std::this_thread::sleep_for(sleep_period);
    }
}

} // namespace Test
} // namespace Nakama
