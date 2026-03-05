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
#include <chrono>
#include <future>
#include <thread>
#include <vector>

namespace Nakama {
namespace Test {

using namespace std;

void test_stress_concurrentAuth() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    const int count = 20;
    vector<future<NSessionPtr>> futures;

    for (int i = 0; i < count; i++) {
      futures.push_back(test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true));
    }

    int succeeded = 0;
    for (auto& f : futures) {
      try {
        auto session = f.get();
        if (session) {
          test.addSession(session);
          succeeded++;
        }
      } catch (...) {
      }
    }

    NLOG_INFO("Concurrent auth: " + to_string(succeeded) + "/" + to_string(count) + " succeeded");
    test.stopTest(succeeded == count);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_stress_rapidSequentialRequests() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    const int count = 50;
    int succeeded = 0;

    for (int i = 0; i < count; i++) {
      try {
        test.client->getAccountAsync(session).get();
        succeeded++;
      } catch (...) {
      }
    }

    NLOG_INFO("Rapid sequential requests: " + to_string(succeeded) + "/" + to_string(count) + " succeeded");
    test.stopTest(succeeded == count);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_stress_multipleClients() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    const int count = 10;
    int succeeded = 0;

    // Use test.client for all operations (it's the only client that gets tick() pumped)
    for (int i = 0; i < count; i++) {
      auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
      test.addSession(session);
      NAccount account = test.client->getAccountAsync(session).get();

      if (!account.user.id.empty()) succeeded++;
    }

    NLOG_INFO("Multiple clients: " + to_string(succeeded) + "/" + to_string(count) + " succeeded");
    test.stopTest(succeeded == count);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_stress_storageFlood() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    const int count = 50;
    vector<NStorageObjectWrite> objects;

    for (int i = 0; i < count; i++) {
      NStorageObjectWrite obj;
      obj.collection = "stress_test";
      obj.key = "item_" + to_string(i);
      obj.value = "{ \"index\": " + to_string(i) + " }";
      obj.permissionRead = NStoragePermissionRead::OWNER_READ;
      obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
      objects.push_back(obj);
    }

    NStorageObjectAcks acks = test.client->writeStorageObjectsAsync(session, objects).get();

    NLOG_INFO("Storage flood: " + to_string(acks.size()) + "/" + to_string(count) + " acks received");
    test.stopTest(acks.size() == static_cast<size_t>(count));
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_stress() {
  test_stress_concurrentAuth();
  test_stress_rapidSequentialRequests();
  test_stress_multipleClients();
  test_stress_storageFlood();
}

} // namespace Test
} // namespace Nakama
