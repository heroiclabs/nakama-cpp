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
#include <nakama-cpp/NException.h>
#include <nakama-cpp/log/NLogger.h>

#include <atomic>
#include <optional>

namespace Nakama {
namespace Test {

using namespace std;

// ── Cancellation Tests ──────────────────────────────────────────────

void test_cancel_noPendingRequests() {
  NTest test(__func__);
  test.setTestTimeoutMs(10000);

  // disconnect() with nothing in-flight should be a safe no-op
  test.client->disconnect();
  test.client->disconnect();

  // Prove the client still works after double-disconnect
  auto successCallback = [&test](NSessionPtr session) {
    NLOG_INFO("session token: " + session->getAuthToken());
    test.stopTest(true);
  };

  test.client->authenticateDevice("cancel_noop_device", std::nullopt, true, {}, successCallback);

  test.runTest();
}

void test_cancel_inflightRequests() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(15000);

  // Override the default error callback that auto-fails on any error.
  // Cancellation tests expect CancelledByUser errors.
  test.client->setErrorCallback(nullptr);

  test.runTest();

  constexpr int kNumRequests = 5;
  atomic<int> cancelledCount{0};
  atomic<int> succeededCount{0};
  atomic<int> unexpectedCount{0};
  mutex mtx;

  vector<future<NSessionPtr>> futures;
  futures.reserve(kNumRequests);

  for (int i = 0; i < kNumRequests; i++) {
    string deviceId = "cancel_inflight_" + to_string(i) + "_" + TestGuid::newGuid();
    futures.push_back(test.client->authenticateCustomAsync(deviceId, "", true));
  }

  // Cancel immediately — some requests may have already completed (race)
  test.client->disconnect();

  for (auto& f : futures) {
    try {
      auto session = f.get();
      test.addSession(session);
      succeededCount++;
    } catch (const NException& e) {
      if (e.error.code == ErrorCode::CancelledByUser) {
        cancelledCount++;
      } else {
        NLOG_ERROR("Unexpected error code: " + string(toString(e.error.code)));
        unexpectedCount++;
      }
    } catch (const std::exception& e) {
      NLOG_ERROR("Unexpected exception: " + string(e.what()));
      unexpectedCount++;
    }
  }

  int total = cancelledCount.load() + succeededCount.load();
  NLOG_INFO("cancelled=" + to_string(cancelledCount.load()) + " succeeded=" + to_string(succeededCount.load()) +
            " unexpected=" + to_string(unexpectedCount.load()));

  test.stopTest(total == kNumRequests && unexpectedCount.load() == 0);
}

void test_cancel_recoveryAfterDisconnect() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(15000);

  // Override default error callback — first request may be cancelled
  test.client->setErrorCallback(nullptr);

  test.runTest();

  try {
    // Authenticate first to get a valid session
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    // Fire a request and immediately disconnect
    auto accountFuture = test.client->getAccountAsync(session);
    test.client->disconnect();

    // First result: don't care if cancelled or succeeded
    try {
      accountFuture.get();
    } catch (...) {
    }

    // Re-issue the same call on the same client — must succeed
    NAccount account = test.client->getAccountAsync(session).get();
    test.stopTest(!account.user.id.empty());
  } catch (const std::exception& e) {
    NLOG_ERROR("Recovery test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

// ── Retry Pattern Tests ─────────────────────────────────────────────

void test_retry_afterConnectionError() {
  // Phase 1: attempt against unreachable port — expect failure
  {
    NClientParameters badParams;
    badParams.port = 1111;

    NTest badTest("test_retry_afterConnectionError_bad", badParams);
    badTest.setTestTimeoutMs(20000);

    auto successCallback = [&badTest](NSessionPtr /*session*/) {
      // Should not succeed against a bad port
      badTest.stopTest(false);
    };

    auto errorCallback = [&badTest](const NError& error) {
      NLOG_INFO("Expected connection error: " + string(toString(error.code)));
      badTest.stopTest(error.code == ErrorCode::ConnectionError || error.code == ErrorCode::CancelledByUser);
    };

    badTest.client->authenticateDevice("retry_conn_device", std::nullopt, true, {}, successCallback, errorCallback);

    badTest.runTest();
  }

  // Phase 2: retry with correct parameters — must succeed
  {
    NTest goodTest("test_retry_afterConnectionError_good", true);
    goodTest.setTestTimeoutMs(15000);
    goodTest.runTest();

    try {
      auto session = goodTest.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
      goodTest.addSession(session);
      goodTest.stopTest(true);
    } catch (const std::exception& e) {
      NLOG_ERROR("Retry with good params failed: " + string(e.what()));
      goodTest.stopTest(false);
    }
  }
}

void test_retry_afterUnauthenticated() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(15000);

  // Override default error callback — we expect Unauthenticated from first call
  test.client->setErrorCallback(nullptr);

  test.runTest();

  try {
    // Use an invalid session
    auto badSession = restoreSession("bad.token.value", "bad.refresh.value");

    // Expect Unauthenticated error
    bool gotUnauthenticated = false;
    try {
      test.client->getAccountAsync(badSession).get();
    } catch (const NException& e) {
      gotUnauthenticated = (e.error.code == ErrorCode::Unauthenticated);
      if (!gotUnauthenticated) {
        NLOG_ERROR("Expected Unauthenticated, got: " + string(toString(e.error.code)));
      }
    }

    if (!gotUnauthenticated) {
      test.stopTest(false);
      return;
    }

    // Re-authenticate to get a valid session
    auto goodSession = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(goodSession);

    // Retry getAccount — should succeed now
    NAccount account = test.client->getAccountAsync(goodSession).get();
    test.stopTest(!account.user.id.empty());
  } catch (const std::exception& e) {
    NLOG_ERROR("Retry after unauth failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_retry_storageIdempotent() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(15000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    string collection = "retry_idem_" + TestGuid::newGuid();

    vector<NStorageObjectWrite> objects;
    NStorageObjectWrite obj;
    obj.collection = collection;
    obj.key = "idempotent_key";
    obj.value = "{ \"attempt\": 1 }";
    obj.permissionRead = NStoragePermissionRead::OWNER_READ;
    obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
    objects.push_back(obj);

    // First write
    test.client->writeStorageObjectsAsync(session, objects).get();

    // Simulated retry: write same key with updated value
    objects[0].value = "{ \"attempt\": 2 }";
    test.client->writeStorageObjectsAsync(session, objects).get();

    // Read back — should have exactly one object with latest value
    auto list =
        test.client->listUsersStorageObjectsAsync(session, collection, session->getUserId()).get();

    bool passed = list->objects.size() == 1 && list->objects[0].value.find("attempt") != string::npos &&
                  list->objects[0].value.find("2") != string::npos;
    test.stopTest(passed);
  } catch (const std::exception& e) {
    NLOG_ERROR("Storage idempotency test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_retry_afterSessionExpiry() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(15000);

  // Override default error callback — we expect Unauthenticated from expired session
  test.client->setErrorCallback(nullptr);

  test.runTest();

  try {
    // Known-expired JWT (exp: 1516910973 = Jan 2018)
    string expiredToken = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
                          "eyJleHAiOjE1MTY5MTA5NzMsInVpZCI6ImY0MTU4ZjJiLTgwZjMtNDkyNi05NDZiLWE4Y2NmYzE2NTQ"
                          "5MCIsInVzbiI6InZUR2RHSHl4dmwifQ."
                          "gzLaMQPaj5wEKoskOSALIeJLOYXEVFoPx3KY0Jm1EVU";
    auto expiredSession = restoreSession(expiredToken, "");

    // Client-side check: session should report as expired
    if (!expiredSession->isExpired()) {
      NLOG_ERROR("Expected session to be expired");
      test.stopTest(false);
      return;
    }

    // Server rejects expired token with Unauthenticated
    bool gotUnauthenticated = false;
    try {
      test.client->getAccountAsync(expiredSession).get();
    } catch (const NException& e) {
      gotUnauthenticated = (e.error.code == ErrorCode::Unauthenticated);
    }

    if (!gotUnauthenticated) {
      NLOG_ERROR("Expected Unauthenticated from expired session");
      test.stopTest(false);
      return;
    }

    // Real-world recovery: re-authenticate and retry
    auto freshSession = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(freshSession);

    if (freshSession->isExpired()) {
      NLOG_ERROR("Fresh session should not be expired");
      test.stopTest(false);
      return;
    }

    NAccount account = test.client->getAccountAsync(freshSession).get();
    test.stopTest(!account.user.id.empty());
  } catch (const std::exception& e) {
    NLOG_ERROR("Session expiry retry failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_retry_authAndGetAccount() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(15000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    // First getAccount
    NAccount account1 = test.client->getAccountAsync(session).get();

    // Second getAccount with same session (sequential retry pattern)
    NAccount account2 = test.client->getAccountAsync(session).get();

    test.stopTest(account1.user.id == account2.user.id && !account1.user.id.empty());
  } catch (const std::exception& e) {
    NLOG_ERROR("Auth+getAccount retry failed: " + string(e.what()));
    test.stopTest(false);
  }
}

// ── Suite ────────────────────────────────────────────────────────────

void test_cancellation() {
  test_cancel_noPendingRequests();
  test_cancel_inflightRequests();
  test_cancel_recoveryAfterDisconnect();
  test_retry_afterConnectionError();
  test_retry_afterUnauthenticated();
  test_retry_afterSessionExpiry();
  test_retry_storageIdempotent();
  test_retry_authAndGetAccount();
}

} // namespace Test
} // namespace Nakama
