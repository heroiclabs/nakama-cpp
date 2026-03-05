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

void test_writeLeaderboardRecord_nonExistent() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->writeLeaderboardRecordAsync(session, "nonexistent_leaderboard", 100).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::NotFound);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listLeaderboardRecords_nonExistent() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->listLeaderboardRecordsAsync(session, "nonexistent_leaderboard").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::NotFound);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listLeaderboardRecordsAroundOwner_nonExistent() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->listLeaderboardRecordsAroundOwnerAsync(session, "nonexistent_leaderboard", session->getUserId()).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::NotFound);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_deleteLeaderboardRecord_nonExistent() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->deleteLeaderboardRecordAsync(session, "nonexistent_leaderboard").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::NotFound);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_writeLeaderboardRecord_emptyId() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->writeLeaderboardRecordAsync(session, "", 100).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listLeaderboardRecords_emptyId() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->listLeaderboardRecordsAsync(session, "").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listLeaderboardRecordsAroundOwner_emptyId() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->listLeaderboardRecordsAroundOwnerAsync(session, "", session->getUserId()).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listLeaderboardRecordsAroundOwner_emptyOwnerId() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->listLeaderboardRecordsAroundOwnerAsync(session, "nonexistent", "").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_deleteLeaderboardRecord_emptyId() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->deleteLeaderboardRecordAsync(session, "").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_leaderboards() {
  test_writeLeaderboardRecord_nonExistent();
  test_listLeaderboardRecords_nonExistent();
  test_listLeaderboardRecordsAroundOwner_nonExistent();
  test_deleteLeaderboardRecord_nonExistent();
  test_writeLeaderboardRecord_emptyId();
  test_listLeaderboardRecords_emptyId();
  test_listLeaderboardRecordsAroundOwner_emptyId();
  test_listLeaderboardRecordsAroundOwner_emptyOwnerId();
  test_deleteLeaderboardRecord_emptyId();
}

} // namespace Test
} // namespace Nakama
