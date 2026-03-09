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

void test_linkCustom() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateDeviceAsync(TestGuid::newGuid(), std::nullopt, true).get();
    test.addSession(session);
    test.client->linkCustomAsync(session, TestGuid::newGuid()).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_linkCustom_tooShort() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateDeviceAsync(TestGuid::newGuid(), std::nullopt, true).get();
    test.addSession(session);
    test.client->linkCustomAsync(session, "abc").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_linkEmail() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateDeviceAsync(TestGuid::newGuid(), std::nullopt, true).get();
    test.addSession(session);
    string email = TestGuid::newGuid() + "@test.com";
    test.client->linkEmailAsync(session, email, "password123!").get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_linkDevice() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->linkDeviceAsync(session, TestGuid::newGuid()).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_unlinkCustom() {
  NTest test(__func__, true);
  test.runTest();

  try {
    string deviceId = TestGuid::newGuid();
    auto session = test.client->authenticateDeviceAsync(deviceId, std::nullopt, true).get();
    test.addSession(session);
    string customId = TestGuid::newGuid();
    test.client->linkCustomAsync(session, customId).get();
    test.client->unlinkCustomAsync(session, customId).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_unlinkEmail() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateDeviceAsync(TestGuid::newGuid(), std::nullopt, true).get();
    test.addSession(session);
    string email = TestGuid::newGuid() + "@test.com";
    string password = "password123!";
    test.client->linkEmailAsync(session, email, password).get();
    test.client->unlinkEmailAsync(session, email, password).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_unlinkDevice() {
  NTest test(__func__, true);
  test.runTest();

  try {
    // Auth with custom so we have another credential, then link+unlink device
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    string deviceId = TestGuid::newGuid();
    test.client->linkDeviceAsync(session, deviceId).get();
    test.client->unlinkDeviceAsync(session, deviceId).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_linkCustom_alreadyLinked() {
  NTest test1(__func__, true);
  test1.runTest();

  NTest test2("test_linkCustom_alreadyLinked_user2", true);
  test2.runTest();

  try {
    auto session1 = test1.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test1.addSession(session1);
    auto session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test2.addSession(session2);

    string customId = TestGuid::newGuid();
    test1.client->linkCustomAsync(session1, customId).get();

    try {
      test2.client->linkCustomAsync(session2, customId).get();
      // Should not succeed — the custom ID is already linked to user1
      test2.stopTest(false);
      test1.stopTest(false);
    } catch (const NException& e) {
      test2.stopTest(e.error.code == ErrorCode::AlreadyExists);
      test1.stopTest(e.error.code == ErrorCode::AlreadyExists);
    }
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test2.stopTest(false);
    test1.stopTest(false);
  }
}

void test_linkDevice_alreadyLinked() {
  NTest test1(__func__, true);
  test1.runTest();

  NTest test2("test_linkDevice_alreadyLinked_user2", true);
  test2.runTest();

  try {
    auto session1 = test1.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test1.addSession(session1);
    auto session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test2.addSession(session2);

    string deviceId = TestGuid::newGuid();
    test1.client->linkDeviceAsync(session1, deviceId).get();

    try {
      test2.client->linkDeviceAsync(session2, deviceId).get();
      // Should not succeed — the device ID is already linked to user1
      test2.stopTest(false);
      test1.stopTest(false);
    } catch (const NException& e) {
      test2.stopTest(e.error.code == ErrorCode::AlreadyExists);
      test1.stopTest(e.error.code == ErrorCode::AlreadyExists);
    }
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test2.stopTest(false);
    test1.stopTest(false);
  }
}

void test_linkEmail_alreadyLinked() {
  NTest test1(__func__, true);
  test1.runTest();

  NTest test2("test_linkEmail_alreadyLinked_user2", true);
  test2.runTest();

  try {
    auto session1 = test1.client->authenticateDeviceAsync(TestGuid::newGuid(), std::nullopt, true).get();
    test1.addSession(session1);
    auto session2 = test2.client->authenticateDeviceAsync(TestGuid::newGuid(), std::nullopt, true).get();
    test2.addSession(session2);

    string email = TestGuid::newGuid() + "@test.com";
    string password = "password123456";
    test1.client->linkEmailAsync(session1, email, password).get();

    try {
      test2.client->linkEmailAsync(session2, email, password).get();
      // Should not succeed — the email is already linked to user1
      test2.stopTest(false);
      test1.stopTest(false);
    } catch (const NException& e) {
      test2.stopTest(e.error.code == ErrorCode::AlreadyExists);
      test1.stopTest(e.error.code == ErrorCode::AlreadyExists);
    }
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test2.stopTest(false);
    test1.stopTest(false);
  }
}

void test_linkEmail_invalidFormat() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateDeviceAsync(TestGuid::newGuid(), std::nullopt, true).get();
    test.addSession(session);
    test.client->linkEmailAsync(session, "notanemail", "password123456").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_linkEmail_shortPassword() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateDeviceAsync(TestGuid::newGuid(), std::nullopt, true).get();
    test.addSession(session);
    test.client->linkEmailAsync(session, TestGuid::newGuid() + "@test.com", "abc").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_unlinkCustom_relink() {
  NTest test(__func__, true);
  test.runTest();

  try {
    // Auth with device, link custom, unlink custom, re-link custom — verify full cycle
    auto session = test.client->authenticateDeviceAsync(TestGuid::newGuid(), std::nullopt, true).get();
    test.addSession(session);
    string customId = TestGuid::newGuid();
    test.client->linkCustomAsync(session, customId).get();
    test.client->unlinkCustomAsync(session, customId).get();
    string customId2 = TestGuid::newGuid();
    test.client->linkCustomAsync(session, customId2).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_unlinkDevice_relink() {
  NTest test(__func__, true);
  test.runTest();

  try {
    // Auth with custom, link device, unlink device, re-link device — verify full cycle
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    string deviceId = TestGuid::newGuid();
    test.client->linkDeviceAsync(session, deviceId).get();
    test.client->unlinkDeviceAsync(session, deviceId).get();
    string deviceId2 = TestGuid::newGuid();
    test.client->linkDeviceAsync(session, deviceId2).get();
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_importFacebook_emptyToken() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateDeviceAsync(TestGuid::newGuid(), std::nullopt, true).get();
    test.addSession(session);
    test.client->importFacebookFriendsAsync(session, "").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code != ErrorCode::Unknown);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_linking() {
  test_linkCustom();
  test_linkCustom_tooShort();
  test_linkEmail();
  test_linkDevice();
  test_unlinkCustom();
  test_unlinkEmail();
  test_unlinkDevice();
  test_linkCustom_alreadyLinked();
  test_linkDevice_alreadyLinked();
  test_linkEmail_alreadyLinked();
  test_linkEmail_invalidFormat();
  test_linkEmail_shortPassword();
  test_unlinkCustom_relink();
  test_unlinkDevice_relink();
  test_importFacebook_emptyToken();
}

} // namespace Test
} // namespace Nakama
