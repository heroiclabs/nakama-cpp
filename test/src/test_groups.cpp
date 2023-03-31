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

#include "NTest.h"
#include "nakama-cpp/NSessionInterface.h"
#include "nakama-cpp/log/NLogger.h"

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
        auto successCallback = [this](NSessionPtr sess)
        {
            this->session = sess;

            NLOG_INFO("session token: " + session->getAuthToken());

            listGroups();
        };

        client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

        NTest::runTest();
    }

    void listGroups()
    {
        auto successCallback = [this](NGroupListPtr list)
        {
            cout << "Groups count " + list->groups.size() << endl;

            for (auto& group : list->groups)
            {
                cout << "Group name " + group.name << endl;
                cout << "Group ID " + group.id << endl;
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

        client->listGroups(session, group_name, 0, "", successCallback);
    }

    void createGroup()
    {
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
            opt::nullopt,
            successCallback);
    }

    void updateGroup(const string& groupId)
    {
        auto successCallback = [this]()
        {
            NLOG_INFO("group updated");
            stopTest(true);
        };

        client->updateGroup(session,
            groupId,
            opt::nullopt,
            "Nakama is awesome!",
            opt::nullopt,
            opt::nullopt,
            opt::nullopt,
            successCallback
        );
    }
};

// this test must run after NGroupTest
class NGroupUsersTest : public NTest
{
    const std::string group_name = "We're-Nakama-Lovers";

    NSessionPtr session;

public:
    explicit NGroupUsersTest(const char* name) : NTest(name) {}

    void runTest() override
    {
        auto successCallback = [this](NSessionPtr sess)
        {
            this->session = sess;

            NLOG_INFO("session token: " + sess->getAuthToken());

            listGroups();
        };

        client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

        NTest::runTest();
    }

    void listGroups()
    {
        auto successCallback = [this](NGroupListPtr list)
        {
            cout << "Groups count " + list->groups.size() << endl;

            for (auto& group : list->groups)
            {
                cout << "Group name " + group.name << endl;
                cout << "Group ID " + group.id << endl;
            }

            auto successCallback = [this](NGroupUserListPtr)
            {
                stopTest(true);
            };

            auto failureCallback = [this](const NError&)
            {
                stopTest(false);
            };

            client->listGroupUsers(this->session, list->groups[0].id, 30, opt::nullopt, "", successCallback, failureCallback);

        };

        client->listGroups(session, group_name, 0, "", successCallback);
    }
};

void test_listGroups()
{
    NGroupsTest test(__func__);
    test.runTest();
}

void test_listGroupUsers()
{
    NGroupUsersTest test(__func__);
    test.runTest();
}

void test_groups()
{
    test_listGroups();
    test_listGroupUsers();
}

} // namespace Test
} // namespace Nakama
