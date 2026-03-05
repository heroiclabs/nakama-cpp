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
#include "nakama-cpp/log/NLogger.h"

#include <nakama-cpp/NException.h>
#include <optional>

namespace Nakama {
namespace Test {

using namespace std;

void test_authenticateEmail1() {
  NTest test(__func__);

  auto successCallback = [&test](NSessionPtr session) { test.stopTest(!session->getAuthToken().empty()); };

  test.client->authenticateEmail("test@mail.com", "12345678", "", true, {}, successCallback);
  test.runTest();
}

void test_authenticateEmail2() {
  NTest test(__func__);

  auto successCallback = [&test](NSessionPtr session) {
    // ensure that username is encoded properly
    NLOG_INFO("session token: " + session->getAuthToken());
    NLOG_INFO("returning username: " + session->getUsername());
    test.stopTest(!session->getAuthToken().empty() && session->getUsername().compare("βσκαταη3") == 0);
  };

  test.client->authenticateEmail("test2@mail.com", "12345678", "βσκαταη3", true, {}, successCallback);

  test.runTest();
}

void test_authenticateDevice() {
  NTest test(__func__);

  auto successCallback = [&test](NSessionPtr session) {
    NLOG_INFO("session token: " + session->getAuthToken());
    test.stopTest(session->getAuthToken().empty() == false);
  };

  test.client->authenticateDevice("mytestdevice0000", std::nullopt, true, {}, successCallback);

  test.runTest();
}

void test_authenticateDevice2() {
  NTest test(__func__);

  auto successCallback = [&test](NSessionPtr session) {
    NLOG_INFO("session token: " + session->getAuthToken());
    test.stopTest(session->getAuthToken().empty() == false && session->getVariable("param1") == "test value");
  };

  NStringMap vars;

  vars.emplace("param1", "test value");

  test.client->authenticateDevice("mytestdevice0001", std::nullopt, std::nullopt, vars, successCallback);

  test.runTest();
}

void test_authenticateRefresh() {
  NTest test(__func__);

  auto successCallback = [&test](NSessionPtr session1) {
    auto successCallback2 = [&test, session1](NSessionPtr session2) {
      test.stopTest(session2->getAuthToken().empty() == false && session2->getAuthToken() == session1->getAuthToken());
    };

    auto errorCallback2 = [&test](const NError& error) { test.stopTest(error); };

    NStringMap vars;

    vars.emplace("param1", "test value changed");
    vars.emplace("param2", "test value new");

    test.client->authenticateRefresh(session1, vars, successCallback2, errorCallback2);
  };

  NStringMap vars;

  vars.emplace("param1", "test value");
  vars.emplace("paramC", "test constant");

  test.client->authenticateDevice("mytestdevice0001", std::nullopt, true, vars, successCallback);

  test.runTest();
}

void test_authenticateCustom() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.stopTest(!session->getAuthToken().empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateCustom_createFalse() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateCustomAsync(TestGuid::newGuid(), "", false).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::NotFound);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateDevice_tooShort() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateDeviceAsync("abc", std::nullopt, true).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateEmail_invalidFormat() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateEmailAsync("notanemail", "password123!", "", true).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateCustom_emptyId() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateCustomAsync("", "", true).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateCustom_tooLong() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateCustomAsync(string(200, 'x'), "", true).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateDevice_empty() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateDeviceAsync("", std::nullopt, true).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateDevice_tooLong() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateDeviceAsync(string(200, 'x'), std::nullopt, true).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateFacebook_emptyToken() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateFacebookAsync("", "", true).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateGoogle_emptyToken() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateGoogleAsync("", "", true).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateSteam_emptyToken() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateSteamAsync("", "", true).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateApple_emptyToken() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateAppleAsync("", "", true).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authenticateEmail_createFalse() {
  NTest test(__func__, true);
  test.runTest();

  try {
    test.client->authenticateEmailAsync("doesnotexist@test.com", "password123456", "", false).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::NotFound);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_authentication() {
  test_authenticateEmail1();

  // No matter what I do, it still fails when running from within Unreal on Windows, because
  // u8"...." utf-8 string constant is interpreted  incorrectly. When compiling outside
  // Unreal we pass /utf-8 on MSVC, but there seems to be no way to pass arbitrary compiler flags
  // when compiling Unreal module.
  // According to Unreal's Engine/Source/Programs/UnrealBuildTool/Platform/Windows/VCToolChain.cs
  // it passes '/source-charset:utf-8' and '/execution-charset:utf-8' which according to MSVC docs
  // is exactly what `/utf-8` flag expands to, but either it doesn't really pass it (and annoyingly
  // there is no way to see exact compiler flags used by UnrealBuildTool or at least I didn't find one)
  // or it doesn't have exactly same effect as `/utf-8`.
  // According to MSVC docs, another way to tell compiler that source is UTF-8 encoded is to add BOM,
  // which this file has now, but it changed nothing.
  test_authenticateEmail2();

  test_authenticateDevice();
  test_authenticateDevice2();
  // test_authenticateRefresh();

  test_authenticateCustom();
  test_authenticateCustom_createFalse();
  test_authenticateDevice_tooShort();
  test_authenticateEmail_invalidFormat();
  test_authenticateCustom_emptyId();
  test_authenticateCustom_tooLong();
  test_authenticateDevice_empty();
  test_authenticateDevice_tooLong();
  test_authenticateFacebook_emptyToken();
  test_authenticateGoogle_emptyToken();
  test_authenticateSteam_emptyToken();
  test_authenticateApple_emptyToken();
  test_authenticateEmail_createFalse();
}

} // namespace Test
} // namespace Nakama
