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

void test_getUsersById() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    string userId = session->getUserId();
    NUsers users = test.client->getUsersAsync(session, {userId}).get();
    test.stopTest(users.users.size() == 1 && users.users[0].id == userId);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_getUsersByUsername() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    NAccount account = test.client->getAccountAsync(session).get();
    string username = account.user.username;
    NUsers users = test.client->getUsersAsync(session, {}, {username}).get();
    test.stopTest(users.users.size() == 1 && users.users[0].username == username);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_getUsersEmpty() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    NUsers users = test.client->getUsersAsync(session, {"ffffffff-ffff-ffff-ffff-ffffffffffff"}).get();
    test.stopTest(users.users.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_getUsersEmpty_emptyRequest() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    NUsers users = test.client->getUsersAsync(session, {}, {}, {}).get();
    test.stopTest(users.users.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_getUsersByFacebookId_empty() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    NUsers users = test.client->getUsersAsync(session, {}, {}, {"nonexistent_fb_id"}).get();
    test.stopTest(users.users.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_users() {
  test_getUsersById();
  test_getUsersByUsername();
  test_getUsersEmpty();
  test_getUsersEmpty_emptyRequest();
  test_getUsersByFacebookId_empty();
}

} // namespace Test
} // namespace Nakama
