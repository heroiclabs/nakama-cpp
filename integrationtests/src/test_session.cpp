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

void test_sessionRefresh() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    auto refreshed = test.client->authenticateRefreshAsync(session).get();
    test.stopTest(!refreshed->getAuthToken().empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_sessionLogout() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->sessionLogoutAsync(session).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_sessionLogout_thenGetAccount() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->sessionLogoutAsync(session).get();
    test.client->getAccountAsync(session).get();
    // Should not reach here — getAccount after logout should fail
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::Unauthenticated);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_sessionRefresh_invalidToken() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->sessionLogoutAsync(session).get();
    test.client->authenticateRefreshAsync(session).get();
    // Should not reach here — refresh after logout should fail
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::Unauthenticated);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_session() {
  test_sessionRefresh();
  test_sessionLogout();
  test_sessionLogout_thenGetAccount();
  test_sessionRefresh_invalidToken();
}

} // namespace Test
} // namespace Nakama
