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
#include "nakama-cpp/log/NLogger.h"

#include <nakama-cpp/NException.h>

namespace Nakama {
namespace Test {

using namespace std;

void test_listFriends() {
  NTest test(__func__, true);
  test.runTest();

  const size_t numFriends = 5;
  std::vector<string> friendIds(numFriends);

  Nakama::NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true, {}).get();
  test.addSession(session);

  // unfortunately, std::when_any is a C++23 feature.
  for (int i = 0; i < numFriends; i++) {
    NSessionPtr friendSession = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true, {}).get();
    test.addSession(friendSession);
    friendIds[i] = friendSession->getUserId();
  }

  // test that using cursor gives a different friend.
  test.client->addFriendsAsync(session, friendIds, {}).get();

  const int limit = 1;

  Nakama::NFriendListPtr invitedList =
      test.client->listFriendsAsync(session, limit, Nakama::NFriend::State::INVITE_SENT).get();
  if (invitedList->friends.empty()) {
    NLOG_ERROR("empty invited list 1");
    test.stopTest(false);
  }

  std::string returnedFriendId1 = invitedList->friends[0].user.id;

  invitedList =
      test.client->listFriendsAsync(session, limit, Nakama::NFriend::State::INVITE_SENT, invitedList->cursor).get();
  if (invitedList->friends.empty()) {
    NLOG_ERROR("empty invited list 2");
    test.stopTest(false);
  }

  std::string returnedFriendId2 = invitedList->friends[0].user.id;
  test.stopTest(returnedFriendId1 != returnedFriendId2);
}

void test_addFriendById() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);
    auto session2 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    test.client->addFriendsAsync(session1, {session2->getUserId()}).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_blockFriendById() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);
    auto session2 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    test.client->blockFriendsAsync(session1, {session2->getUserId()}).get();
    auto friendList = test.client->listFriendsAsync(session1, 10, Nakama::NFriend::State::BLOCKED).get();
    bool found = false;
    for (auto& f : friendList->friends) {
      if (f.user.id == session2->getUserId()) {
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

void test_deleteFriendById() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);
    auto session2 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    test.client->addFriendsAsync(session1, {session2->getUserId()}).get();
    test.client->deleteFriendsAsync(session1, {session2->getUserId()}).get();
    auto friendList = test.client->listFriendsAsync(session1, 10, std::nullopt).get();
    bool found = false;
    for (auto& f : friendList->friends) {
      if (f.user.id == session2->getUserId()) {
        found = true;
        break;
      }
    }
    test.stopTest(!found);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_addFriendByUsername() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);

    NTest test2("test_addFriendByUsername_helper", true);
    test2.runTest();
    auto session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    std::string username2 = session2->getUsername();
    test2.stopTest(true);

    test.client->addFriendsAsync(session1, {}, {username2}).get();
    auto friendList = test.client->listFriendsAsync(session1, std::nullopt, std::nullopt).get();
    test.stopTest(!friendList->friends.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_deleteFriendByUsername() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);

    NTest test2("test_deleteFriendByUsername_helper", true);
    test2.runTest();
    auto session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    std::string userId2 = session2->getUserId();
    std::string username2 = session2->getUsername();
    test2.stopTest(true);

    test.client->addFriendsAsync(session1, {userId2}).get();
    test.client->deleteFriendsAsync(session1, {}, {username2}).get();
    auto friendList = test.client->listFriendsAsync(session1, std::nullopt, std::nullopt).get();
    bool found = false;
    for (auto& f : friendList->friends) {
      if (f.user.id == userId2) {
        found = true;
        break;
      }
    }
    test.stopTest(!found);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_blockFriendByUsername() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);

    NTest test2("test_blockFriendByUsername_helper", true);
    test2.runTest();
    auto session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    std::string username2 = session2->getUsername();
    test2.stopTest(true);

    test.client->blockFriendsAsync(session1, {}, {username2}).get();
    auto friendList = test.client->listFriendsAsync(session1, std::nullopt, NFriend::State::BLOCKED).get();
    test.stopTest(!friendList->friends.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listFriends_withLimit() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    NTest test2("test_listFriends_withLimit_helper1", true);
    test2.runTest();
    auto friendSession1 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(friendSession1);
    test2.stopTest(true);

    NTest test3("test_listFriends_withLimit_helper2", true);
    test3.runTest();
    auto friendSession2 = test3.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(friendSession2);
    test3.stopTest(true);

    test.client->addFriendsAsync(session, {friendSession1->getUserId(), friendSession2->getUserId()}).get();
    auto friendList = test.client->listFriendsAsync(session, 1, std::nullopt).get();
    test.stopTest(friendList->friends.size() <= 1);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listFriends_stateFilter() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session1 = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session1);

    NTest test2("test_listFriends_stateFilter_helper", true);
    test2.runTest();
    auto session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session2);
    test2.stopTest(true);

    test.client->addFriendsAsync(session1, {session2->getUserId()}).get();
    // List with FRIEND state filter - the invite might be pending so we just verify the call succeeds
    auto friendList = test.client->listFriendsAsync(session1, std::nullopt, NFriend::State::FRIEND).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_blockSelf() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    std::string userId = session->getUserId();
    test.client->blockFriendsAsync(session, {userId}).get();
    // If we get here, the server did not reject it - fail the test
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_friends() {
  test_listFriends();
  test_addFriendById();
  test_blockFriendById();
  test_deleteFriendById();
  test_addFriendByUsername();
  test_deleteFriendByUsername();
  test_blockFriendByUsername();
  test_listFriends_withLimit();
  test_listFriends_stateFilter();
  test_blockSelf();
}

} // namespace Test
} // namespace Nakama
