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

class NGroupsTest : public NTest
{
    const std::string group_name = "We're-Nakama-Lovers";

    NSessionPtr session;

public:
    NGroupsTest(const char* name) : NTest(name) {}

    void runTest() override
    {
        createWorkingClient();

        auto errorCallback = [this](const NError& error)
        {
            stopTest();
        };

        auto successCallback = [this, errorCallback](NSessionPtr session)
        {
            this->session = session;

            std::cout << "session token: " << session->getAuthToken() << std::endl;

            listGroups();
        };

        client->authenticateDevice("mytestdevice0000", opt::nullopt, true, successCallback, errorCallback);

        NTest::runTest();
    }

    void listGroups()
    {
        auto errorCallback = [this](const NError& error)
        {
            stopTest();
        };

        auto successCallback = [this](NGroupListPtr list)
        {
            cout << "Groups count " << list->groups.size() << endl;

            for (auto& group : list->groups)
            {
                cout << "Group name " << group.name << endl;
                cout << "Group ID " << group.id << endl;
            }

            if (list->groups.size() > 0)
            {
                updateGroup(list->groups[0].id);
            }
            else
            {
                createGroup();
            }
        };

        client->listGroups(session, group_name, 0, "", successCallback, errorCallback);
    }

    void createGroup()
    {
        auto errorCallback = [this](const NError& error)
        {
            stopTest();
        };

        auto successCallback = [this](const NGroup& group)
        {
            updateGroup(group.id);
        };

        std::string description = "Nakama is cool!";

        client->createGroup(session,
            group_name,
            description,
            "",  // avatar URL
            "en_US",
            true, // open
            successCallback,
            errorCallback);
    }

    void updateGroup(const string& groupId)
    {
        auto errorCallback = [this](const NError& error)
        {
            stopTest();
        };

        auto successCallback = [this]()
        {
            std::cout << "group updated" << std::endl;
            stopTest(true);
        };

        client->updateGroup(session,
            groupId,
            opt::nullopt,
            "Nakama is awesome!",
            opt::nullopt,
            opt::nullopt,
            opt::nullopt,
            successCallback,
            errorCallback
        );
    }
};

void test_listGroups()
{
    NGroupsTest test(__func__);

    test.runTest();
}

void test_groups()
{
    test_listGroups();
}

} // namespace Test
} // namespace Nakama
