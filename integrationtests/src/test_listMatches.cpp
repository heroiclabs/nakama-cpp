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
#include "globals.h"

#include <optional>

namespace Nakama {
namespace Test {

using namespace std;

class NMatchListTest : public NTest {
  NSessionPtr session;

public:
  explicit NMatchListTest(const char* name) : NTest(name) {}

  void runTest() override {
    auto successCallback = [this](NSessionPtr sess) {
      this->session = sess;

      NLOG_INFO("session token: " + sess->getAuthToken());

      listMatches();
    };

    client->authenticateDevice("mytestdevice0000", std::nullopt, true, {}, successCallback);

    NTest::runTest();
  }

  void listMatches() {
    auto rpcSuccessCallback = [this](const NRpc& rpc) {
      auto successCallback = [this](NMatchListPtr matchList) {
        NLOG_INFO("Expecting match count to be 2. Actual count: " + std::to_string(matchList->matches.size()));
        NTEST_ASSERT(matchList->matches.size() == 2);
        stopTest(true);
      };

      int minPlayers = 0;
      int maxPlayers = 10;
      int limit = 10;
      bool authoritative = true;
      string label = "";
      string query = "+label.type:freeforall +label.difficulty:>1";
      client->listMatches(session, minPlayers, maxPlayers, limit, label, query, authoritative, successCallback);
    };

    client->rpc(session, "create_matches", "", rpcSuccessCallback);
  }
};

void test_listMatchesWithRpc() {
  NMatchListTest test(__func__);
  test.runTest();
}

void test_listMatches_basic() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    auto matchList = test.client->listMatchesAsync(session).get();
    // Just verify the call succeeds; empty result is fine
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listMatches_withMinSize() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    auto matchList = test.client->listMatchesAsync(session, 1).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listMatches_withMaxSize() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    auto matchList = test.client->listMatchesAsync(session, std::nullopt, 10).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listMatches_authoritativeOnly() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    auto matchList =
        test.client
            ->listMatchesAsync(session, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, true)
            .get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listMatches_withLabel() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    auto matchList =
        test.client
            ->listMatchesAsync(
                session, std::nullopt, std::nullopt, std::nullopt, std::string("TestAuthoritativeMatch"))
            .get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listMatches() {
  // TODO flaky: test_listMatchesWithRpc();
  test_listMatches_basic();
  test_listMatches_withMinSize();
  test_listMatches_withMaxSize();
  test_listMatches_authoritativeOnly();
  test_listMatches_withLabel();
}

} // namespace Test
} // namespace Nakama
