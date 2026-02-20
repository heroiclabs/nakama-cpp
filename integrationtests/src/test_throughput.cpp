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

#include <chrono>
#include <future>
#include <string>
#include <vector>

namespace Nakama {
namespace Test {

using namespace std;

// Sequential: measures per-message round-trip latency (baseline)
void test_throughput_rpcSequential() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.rtClient->connectAsync(session, false, NTest::RtProtocol).get();

    string payload(1024, 'x');
    payload = "{\"d\":\"" + payload + "\"}";

    const int messageCount = 100;

    auto start = chrono::steady_clock::now();
    for (int i = 0; i < messageCount; i++) {
      test.rtClient->rpcAsync("clientrpc.rpc", payload).get();
    }
    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    double seconds = ms / 1000.0;
    double msgsPerSec = messageCount / seconds;

    NLOG_INFO("Sequential RPC (" + to_string(messageCount) + " x " + to_string(payload.size()) + "B):");
    NLOG_INFO("  " + to_string(ms) + " ms total, " + to_string(ms / messageCount) + " ms/msg, " +
              to_string(static_cast<int>(msgsPerSec)) + " msg/s");

    test.stopTest(true);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

// Pipelined: fire all RPCs, then wait for all responses.
// This saturates the send path and exercises the I/O thread's continuous drain.
void test_throughput_rpcPipelined() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.rtClient->connectAsync(session, false, NTest::RtProtocol).get();

    string payload(1024, 'x');
    payload = "{\"d\":\"" + payload + "\"}";

    const int messageCount = 500;
    const size_t totalBytes = payload.size() * messageCount * 2; // send + receive

    // Fire all RPCs without waiting
    auto start = chrono::steady_clock::now();
    vector<future<NRpc>> futures;
    futures.reserve(messageCount);
    for (int i = 0; i < messageCount; i++) {
      futures.push_back(test.rtClient->rpcAsync("clientrpc.rpc", payload));
    }

    // Wait for all responses
    for (auto& f : futures) {
      f.get();
    }
    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    double seconds = ms / 1000.0;
    double throughputMBs = (totalBytes / (1024.0 * 1024.0)) / seconds;
    double msgsPerSec = messageCount / seconds;

    NLOG_INFO("Pipelined RPC (" + to_string(messageCount) + " x " + to_string(payload.size()) + "B):");
    NLOG_INFO("  " + to_string(totalBytes / 1024) + " KB round-trip in " + to_string(ms) + " ms");
    NLOG_INFO("  " + to_string(throughputMBs) + " MB/s, " + to_string(static_cast<int>(msgsPerSec)) + " msg/s");

    test.stopTest(true);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

// Pipelined with large payloads: measures bulk data throughput
void test_throughput_rpcPipelinedLarge() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.rtClient->connectAsync(session, false, NTest::RtProtocol).get();

    // ~32KB payload (staying under typical WS frame limits)
    string payload(32 * 1024, 'y');
    payload = "{\"d\":\"" + payload + "\"}";

    const int messageCount = 100;
    const size_t totalBytes = payload.size() * messageCount * 2; // send + receive

    auto start = chrono::steady_clock::now();
    vector<future<NRpc>> futures;
    futures.reserve(messageCount);
    for (int i = 0; i < messageCount; i++) {
      futures.push_back(test.rtClient->rpcAsync("clientrpc.rpc", payload));
    }
    for (auto& f : futures) {
      f.get();
    }
    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    double seconds = ms / 1000.0;
    double throughputMBs = (totalBytes / (1024.0 * 1024.0)) / seconds;
    double msgsPerSec = messageCount / seconds;

    NLOG_INFO("Pipelined RPC LARGE (" + to_string(messageCount) + " x " + to_string(payload.size() / 1024) + "KB):");
    NLOG_INFO("  " + to_string(totalBytes / (1024 * 1024)) + " MB round-trip in " + to_string(ms) + " ms");
    NLOG_INFO("  " + to_string(throughputMBs) + " MB/s, " + to_string(static_cast<int>(msgsPerSec)) + " msg/s");

    test.stopTest(true);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

// Pipelined with small payloads: measures message-rate ceiling
void test_throughput_rpcPipelinedSmall() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(120000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.rtClient->connectAsync(session, false, NTest::RtProtocol).get();

    // ~100B payload
    string payload(100, 'z');
    payload = "{\"d\":\"" + payload + "\"}";

    const int messageCount = 500;
    const size_t totalBytes = payload.size() * messageCount * 2; // send + receive

    auto start = chrono::steady_clock::now();
    vector<future<NRpc>> futures;
    futures.reserve(messageCount);
    for (int i = 0; i < messageCount; i++) {
      futures.push_back(test.rtClient->rpcAsync("clientrpc.rpc", payload));
    }
    for (auto& f : futures) {
      f.get();
    }
    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    double seconds = ms / 1000.0;
    double throughputMBs = (totalBytes / (1024.0 * 1024.0)) / seconds;
    double msgsPerSec = messageCount / seconds;

    NLOG_INFO("Pipelined RPC SMALL (" + to_string(messageCount) + " x " + to_string(payload.size()) + "B):");
    NLOG_INFO("  " + to_string(totalBytes / 1024) + " KB round-trip in " + to_string(ms) + " ms");
    NLOG_INFO("  " + to_string(throughputMBs) + " MB/s, " + to_string(static_cast<int>(msgsPerSec)) + " msg/s");

    test.stopTest(true);
  } catch (const exception& e) {
    NLOG_INFO("test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

// Pipelined 100MB total: stress-tests sustained throughput with large aggregate volume
void test_throughput_rpc100MB() {
  NTest test(__func__, true);
  test.setTestTimeoutMs(300000);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    test.rtClient->connectAsync(session, false, NTest::RtProtocol).get();

    // ~64KB payload, ~1600 messages ≈ 100MB sent (+ 100MB received = 200MB round-trip)
    const size_t payloadSize = 64 * 1024;
    string payload(payloadSize, 'w');
    payload = "{\"d\":\"" + payload + "\"}";

    const size_t targetBytes = 100ULL * 1024 * 1024;
    const int messageCount = static_cast<int>(targetBytes / payload.size());
    const size_t totalBytesSent = payload.size() * messageCount;
    const size_t totalBytesRoundTrip = totalBytesSent * 2;

    NLOG_INFO("100MB test: sending " + to_string(messageCount) + " x " + to_string(payload.size() / 1024) +
              "KB = " + to_string(totalBytesSent / (1024 * 1024)) + " MB");

    auto start = chrono::steady_clock::now();
    vector<future<NRpc>> futures;
    futures.reserve(messageCount);
    for (int i = 0; i < messageCount; i++) {
      futures.push_back(test.rtClient->rpcAsync("clientrpc.rpc", payload));
    }
    for (auto& f : futures) {
      f.get();
    }
    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    double seconds = ms / 1000.0;
    double sendMBs = (totalBytesSent / (1024.0 * 1024.0)) / seconds;
    double roundTripMBs = (totalBytesRoundTrip / (1024.0 * 1024.0)) / seconds;
    double msgsPerSec = messageCount / seconds;

    NLOG_INFO("100MB Pipelined RPC (" + to_string(messageCount) + " x " + to_string(payload.size() / 1024) + "KB):");
    NLOG_INFO("  " + to_string(totalBytesSent / (1024 * 1024)) + " MB sent in " + to_string(ms) + " ms");
    NLOG_INFO("  Send: " + to_string(sendMBs) + " MB/s");
    NLOG_INFO("  Round-trip: " + to_string(roundTripMBs) + " MB/s");
    NLOG_INFO("  " + to_string(static_cast<int>(msgsPerSec)) + " msg/s");

    test.stopTest(true);
  } catch (const exception& e) {
    NLOG_INFO("100MB throughput test failed: " + string(e.what()));
    test.stopTest(false);
  }
}

void test_throughput() {
  test_throughput_rpcSequential();
  test_throughput_rpcPipelinedSmall();
  test_throughput_rpcPipelined();
  test_throughput_rpcPipelinedLarge();
  test_throughput_rpc100MB();
}

} // namespace Test
} // namespace Nakama
