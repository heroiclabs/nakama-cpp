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
#include "TestGuid.h"
#include "nakama-cpp/log/NLogger.h"

#include <optional>

namespace Nakama {
namespace Test {

using namespace std;

void test_getAccountAndUpdate() {
  NTest test(__func__);

  auto successCallback = [&test](NSessionPtr session) {
    NLOG_INFO("session token: " + session->getAuthToken());

    auto successCallback = [&test, session](const NAccount& account) {
      NLOG_INFO("account user id: " + account.user.id);

      auto successCallback = [&test]() {
        NLOG_INFO("account updated");
        test.stopTest(true);
      };

      test.client->updateAccount(
          session, std::nullopt, "Nakama-test", std::nullopt, std::nullopt, std::nullopt, std::nullopt,
          successCallback);
    };

    test.client->getAccount(session, successCallback);
  };

  test.client->authenticateDevice("mytestdevice0000", std::nullopt, true, {}, successCallback);

  test.runTest();
}

void test_updateAccountAllFields() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client
        ->updateAccountAsync(session, std::nullopt, "TestDisplay", "http://example.com/avatar.png", "en", "US", "UTC")
        .get();
    NAccount account = test.client->getAccountAsync(session).get();
    bool ok = account.user.displayName == "TestDisplay" && account.user.avatarUrl == "http://example.com/avatar.png" &&
              account.user.lang == "en" && account.user.location == "US" && account.user.timeZone == "UTC";
    test.stopTest(ok);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_getAccountDetails() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    NAccount account = test.client->getAccountAsync(session).get();
    test.stopTest(!account.user.id.empty() && account.user.createdAt > 0);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_getAccount() {
  test_getAccountAndUpdate();
  test_updateAccountAllFields();
  test_getAccountDetails();
}

} // namespace Test
} // namespace Nakama
