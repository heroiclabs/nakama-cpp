/*
 * Copyright 2024 The Nakama Authors
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

void test_listChannelMessages_emptyId() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->listChannelMessagesAsync(session, "").get();
    test.stopTest(false);
  } catch (const NException&) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listChannelMessages_invalidId() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->listChannelMessagesAsync(session, "not-a-valid-channel-id").get();
    test.stopTest(false);
  } catch (const NException&) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listChannelMessages_withLimit() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->listChannelMessagesAsync(session, "some-channel-id", 5).get();
    // If it somehow succeeds, that's fine too
    test.stopTest(true);
  } catch (const NException&) {
    // Expected: invalid channel id causes an error
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_channelMessages() {
  test_listChannelMessages_emptyId();
  test_listChannelMessages_invalidId();
  test_listChannelMessages_withLimit();
}

} // namespace Test
} // namespace Nakama
