/*
 * Copyright 2026 The Nakama Authors
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
#include "TestGuid.h"
#include "nakama-cpp/NSessionInterface.h"
#include "nakama-cpp/log/NLogger.h"

#include <nakama-cpp/NException.h>
#include <optional>

namespace Nakama {
namespace Test {

using namespace std;

class NGroupsTest : public NTest {
  const std::string group_name = "We're-Nakama-Lovers";

  NSessionPtr session;

public:
  NGroupsTest(std::string name) : NTest(name) {}

  void runTest() override {
    auto successCallback = [this](NSessionPtr sess) {
      this->session = sess;

      NLOG_INFO("session token: " + session->getAuthToken());

      listGroups();
    };

    client->authenticateDevice("mytestdevice0000", std::nullopt, true, {}, successCallback);

    NTest::runTest();
  }

  void listGroups() {
    auto successCallback = [this](NGroupListPtr list) {
      for (auto& group : list->groups) {
        cout << "Group name " + group.name << endl;
        cout << "Group ID " + group.id << endl;
      }

      if (list->groups.size() > 0) {
        updateGroup(list->groups[0].id);
      } else {
        createGroup();
      }
    };

    client->listGroups(session, group_name, 0, "", successCallback);
  }

  void createGroup() {
    auto successCallback = [this](const NGroup& group) { updateGroup(group.id); };

    std::string description = "Nakama is cool!";

    client->createGroup(
        session, group_name, description,
        "", // avatar URL
        "en_US",
        true, // open
        std::nullopt, successCallback);
  }

  void updateGroup(const string& groupId) {
    auto successCallback = [this]() {
      NLOG_INFO("group updated");
      stopTest(true);
    };

    client->updateGroup(
        session, groupId, std::nullopt, "Nakama is awesome!", std::nullopt, std::nullopt, std::nullopt,
        successCallback);
  }
};

// this test must run after NGroupTest
class NGroupUsersTest : public NTest {
  const std::string group_name = "We're-Nakama-Lovers";

  NSessionPtr session;

public:
  explicit NGroupUsersTest(std::string name) : NTest(name) {}

  void runTest() override {
    auto successCallback = [this](NSessionPtr sess) {
      this->session = sess;

      NLOG_INFO("session token: " + sess->getAuthToken());

      listGroups();
    };

    client->authenticateDevice("mytestdevice0000", std::nullopt, true, {}, successCallback);

    NTest::runTest();
  }

  void listGroups() {
    auto successCallback = [this](NGroupListPtr list) {
      if (!list) {
        NLOG_ERROR("listGroups returned null list");
        stopTest(false);
        return;
      }

      for (auto& group : list->groups) {
        cout << "Group name " + group.name << endl;
        cout << "Group ID " + group.id << endl;
      }

      if (list->groups.empty()) {
        NLOG_ERROR("listGroups returned no groups for listGroupUsers test");
        stopTest(false);
        return;
      }

      const auto& groupId = list->groups.front().id;
      if (groupId.empty()) {
        NLOG_ERROR("listGroups returned group with empty id");
        stopTest(false);
        return;
      }

      auto successCallback = [this](NGroupUserListPtr) { stopTest(true); };

      auto failureCallback = [this](const NError&) { stopTest(false); };

      client->listGroupUsers(this->session, groupId, 30, std::nullopt, "", successCallback, failureCallback);
    };

    client->listGroups(session, group_name, 0, "", successCallback);
  }
};

void test_listGroups() {
  NGroupsTest test(__func__);
  test.runTest();
}

void test_listGroupUsers() {
  NGroupUsersTest test(__func__);
  test.runTest();
}

void test_createGroup() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    NGroup group =
        test.client->createGroupAsync(session, "TestGroup-" + TestGuid::newGuid(), "A test group", "", "en", true)
            .get();
    test.stopTest(!group.id.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_createGroup_emptyName() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->createGroupAsync(session, "").get();
    test.stopTest(false);
  } catch (const NException&) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_deleteGroup() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    NGroup group =
        test.client->createGroupAsync(session, "DeleteMe-" + TestGuid::newGuid(), "", "", "", true).get();
    test.client->deleteGroupAsync(session, group.id).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_joinAndLeaveGroup() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);
    auto session2 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    NGroup group =
        test.client->createGroupAsync(session1, "JoinLeave-" + TestGuid::newGuid(), "", "", "", true).get();
    test.client->joinGroupAsync(session2, group.id).get();
    test.client->leaveGroupAsync(session2, group.id).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_addAndKickGroupUser() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);
    auto session2 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    NGroup group =
        test.client->createGroupAsync(session1, "KickTest-" + TestGuid::newGuid(), "", "", "", true).get();
    test.client->addGroupUsersAsync(session1, group.id, {session2->getUserId()}).get();
    test.client->kickGroupUsersAsync(session1, group.id, {session2->getUserId()}).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_createClosedGroup() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    NGroup group =
        test.client->createGroupAsync(session, "ClosedGroup_" + TestGuid::newGuid(), "", "", "", false).get();
    test.stopTest(!group.id.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_createGroup_withDescription() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    NGroup group =
        test.client->createGroupAsync(session, "DescGroup_" + TestGuid::newGuid(), "TestDesc", "", "en").get();
    test.stopTest(group.description == "TestDesc");
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_updateGroup() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    NGroup group =
        test.client->createGroupAsync(session, "PreUpdate_" + TestGuid::newGuid(), "", "", "", true).get();
    std::string updatedName = "UpdatedName_" + TestGuid::newGuid();
    test.client->updateGroupAsync(session, group.id, updatedName).get();
    auto groupList = test.client->listGroupsAsync(session, updatedName, 10).get();
    bool found = false;
    for (auto& g : groupList->groups) {
      if (g.id == group.id) {
        found = true;
        break;
      }
    }
    test.stopTest(found);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_promoteGroupUser() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);

    NTest test2("test_promoteGroupUser_helper", true);
    test2.runTest();
    auto session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    std::string userId2 = session2->getUserId();
    test2.stopTest(true);

    NGroup group =
        test.client->createGroupAsync(session1, "PromoteTest_" + TestGuid::newGuid(), "", "", "", true).get();
    test.client->joinGroupAsync(session2, group.id).get();
    test.client->promoteGroupUsersAsync(session1, group.id, {userId2}).get();

    auto groupUsers = test.client->listGroupUsersAsync(session1, group.id, std::nullopt, std::nullopt).get();
    bool promoted = false;
    for (auto& gu : groupUsers->groupUsers) {
      if (gu.user.id == userId2 && gu.state != NUserGroupState::MEMBER) {
        promoted = true;
        break;
      }
    }
    test.stopTest(promoted);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_demoteGroupUser() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);

    NTest test2("test_demoteGroupUser_helper", true);
    test2.runTest();
    auto session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    std::string userId2 = session2->getUserId();
    test2.stopTest(true);

    NGroup group =
        test.client->createGroupAsync(session1, "DemoteTest_" + TestGuid::newGuid(), "", "", "", true).get();
    test.client->joinGroupAsync(session2, group.id).get();
    test.client->promoteGroupUsersAsync(session1, group.id, {userId2}).get();
    test.client->demoteGroupUsersAsync(session1, group.id, {userId2}).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listGroups_withNameFilter() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    std::string uniqueName = "UniqueGroup_" + TestGuid::newGuid();
    test.client->createGroupAsync(session, uniqueName, "", "", "", true).get();
    auto groupList = test.client->listGroupsAsync(session, uniqueName, 10).get();
    test.stopTest(!groupList->groups.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listGroups_withLimit() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    auto groupList = test.client->listGroupsAsync(session, "", 1).get();
    test.stopTest(groupList->groups.size() <= 1);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listUserGroups() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->createGroupAsync(session, "UserGroupTest_" + TestGuid::newGuid(), "", "", "", true).get();
    auto userGroups = test.client->listUserGroupsAsync(session, std::nullopt, std::nullopt).get();
    test.stopTest(!userGroups->userGroups.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_joinGroup_nonExistent() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->joinGroupAsync(session, "00000000-0000-0000-0000-000000000000").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::NotFound);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_leaveGroup_nonMember() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);

    NTest test2("test_leaveGroup_nonMember_helper", true);
    test2.runTest();
    auto session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    test2.stopTest(true);

    NGroup group =
        test.client->createGroupAsync(session1, "ClosedLeave_" + TestGuid::newGuid(), "", "", "", false).get();
    // User2 is not a member of this closed group. Attempt to leave.
    test.client->leaveGroupAsync(session2, group.id).get();
    // If the server accepts the call silently, that is fine
    test.stopTest(true);
  } catch (const NException&) {
    // An error is also acceptable behavior
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_groups() {
  test_listGroups();
  test_listGroupUsers();
  test_createGroup();
  test_createGroup_emptyName();
  test_deleteGroup();
  test_joinAndLeaveGroup();
  test_addAndKickGroupUser();
  test_createClosedGroup();
  test_createGroup_withDescription();
  test_updateGroup();
  test_promoteGroupUser();
  test_demoteGroupUser();
  test_listGroups_withNameFilter();
  test_listGroups_withLimit();
  test_listUserGroups();
  test_joinGroup_nonExistent();
  test_leaveGroup_nonMember();
}

} // namespace Test
} // namespace Nakama
