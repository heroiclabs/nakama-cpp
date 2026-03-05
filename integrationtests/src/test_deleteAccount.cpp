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
#include "globals.h"

#include <optional>

namespace Nakama {
namespace Test {

using namespace std;

void test_deleteAccount_basic() {
  NTest test(__func__);

  auto successCallback = [&test](NSessionPtr session) {
    NLOG_INFO("session token: " + session->getAuthToken());

    auto deleteSuccess = [&test, session]() {
      NLOG_INFO("account deleted successfully");

      // Verify the session is no longer valid by trying to fetch the account.
      auto shouldNotSucceed = [&test](const NAccount&) {
        NLOG_INFO("getAccount succeeded after delete — unexpected");
        test.stopTest(false);
      };

      auto expectError = [&test](const NError& error) {
        NLOG_INFO("getAccount after delete returned error: " + toString(error));
        test.stopTest(error.code == ErrorCode::NotFound || error.code == ErrorCode::Unauthenticated);
      };

      test.client->getAccount(session, shouldNotSucceed, expectError);
    };

    test.client->deleteAccount(session, deleteSuccess);
  };

  test.client->authenticateDevice("delete_test_device_01", std::nullopt, true, {}, successCallback);

  test.runTest();
}

void test_deleteAccount() {
  test_deleteAccount_basic();
}

} // namespace Test
} // namespace Nakama
