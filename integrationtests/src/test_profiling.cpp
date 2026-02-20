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

#include <nakama-cpp/NException.h>
#include <chrono>
#include <thread>
#include <vector>

namespace Nakama {
namespace Test {

using namespace std;

void test_profiling_authLatency() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    const int iterations = 10;
    auto start = chrono::steady_clock::now();

    for (int i = 0; i < iterations; i++) {
      test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    }

    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    NLOG_INFO("Auth latency: " + to_string(ms / iterations) + "ms avg over " + to_string(iterations) + " calls");
    test.stopTest(true);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_profiling_storageLatency() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    const int iterations = 10;
    auto start = chrono::steady_clock::now();

    for (int i = 0; i < iterations; i++) {
      vector<NStorageObjectWrite> objects;
      NStorageObjectWrite obj;
      obj.collection = "profiling";
      obj.key = "item_" + to_string(i);
      obj.value = "{ \"index\": " + to_string(i) + " }";
      obj.permissionRead = NStoragePermissionRead::OWNER_READ;
      obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
      objects.push_back(obj);

      test.client->writeStorageObjectsAsync(session, objects).get();

      vector<NReadStorageObjectId> readIds;
      NReadStorageObjectId readId;
      readId.collection = "profiling";
      readId.key = "item_" + to_string(i);
      readId.userId = session->getUserId();
      readIds.push_back(readId);

      test.client->readStorageObjectsAsync(session, readIds).get();
    }

    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    NLOG_INFO("Storage write+read latency: " + to_string(ms / iterations) + "ms avg over " + to_string(iterations) +
              " calls");
    test.stopTest(true);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_profiling_accountGetLatency() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    const int iterations = 10;
    auto start = chrono::steady_clock::now();

    for (int i = 0; i < iterations; i++) {
      test.client->getAccountAsync(session).get();
    }

    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    NLOG_INFO("getAccount latency: " + to_string(ms / iterations) + "ms avg over " + to_string(iterations) + " calls");
    test.stopTest(true);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_profiling_clientCreateDestroy() {
  NTest test(__func__, true);
  test.runTest();

  try {
    for (int i = 0; i < 20; i++) {
      auto client = NTest::ClientFactory(NTest::NClientParameters);
      // Just create and let it go out of scope
    }
    test.stopTest(true);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_profiling() {
  test_profiling_authLatency();
  test_profiling_storageLatency();
  test_profiling_accountGetLatency();
  test_profiling_clientCreateDestroy();
}

} // namespace Test
} // namespace Nakama
