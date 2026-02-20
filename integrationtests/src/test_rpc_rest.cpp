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

void test_rpc_nonExistent() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->rpcAsync(session, "nonexistent_rpc_id", "{}").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::NotFound);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_rpc_withHttpKey() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto rpc = test.client->rpcAsync(NTest::ServerHttpKey, "clientrpc.rpc", "{\"test\":1}").get();
    test.stopTest(!rpc.payload.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_rpc_errorFromServer() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.client->rpcAsync(session, "clientrpc.rpc_error", "{}").get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(true);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_rpc_rest() {
  test_rpc_nonExistent();
  test_rpc_withHttpKey();
  test_rpc_errorFromServer();
}

} // namespace Test
} // namespace Nakama
