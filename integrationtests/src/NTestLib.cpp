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

#include "NTestLib.h"
#include "globals.h"
#include <nakama-cpp/NException.h>
#include <mutex>
#include <stdint.h>
#include <thread>
#include <vector>

namespace Nakama {
namespace Test {

void test_internals();
void test_authentication();
void test_session();
void test_getAccount();
void test_users();
void test_timeouts();
void test_disconnect();
void test_errors();
void test_restoreSession();
void test_storage();
void test_groups();
void test_friends();
void test_linking();
void test_leaderboards();
void test_tournaments_rest();
void test_listMatches();
void test_channelMessages();
void test_deleteAccount();
void test_notifications_rest();
void test_rpc_rest();
void test_profiling();
void test_stress();
void test_realtime();
void test_throughput();
void test_cancellation();

static void runSuiteSafely(const char* suiteName, void (*suite)()) {
  try {
    suite();
  } catch (const NException& e) {
    ++g_failedTestsCount;
    NLOG_ERROR(std::string("Unhandled NException in ") + suiteName + ": " + e.what());
  } catch (const std::exception& e) {
    ++g_failedTestsCount;
    NLOG_ERROR(std::string("Unhandled exception in ") + suiteName + ": " + e.what());
  } catch (...) {
    ++g_failedTestsCount;
    NLOG_ERROR(std::string("Unhandled non-standard exception in ") + suiteName);
  }
}

std::ostream& printPercent(std::ostream& os, std::uint32_t totalCount, std::uint32_t count) {
  if (totalCount > 0) {
    os << count * 100 / totalCount << "%";
  } else {
    os << "0%";
  }

  return os;
}

int runAllTests(
    std::function<NClientPtr(Nakama::NClientParameters)> clientFactory,
    std::function<NRtClientPtr(Nakama::NClientPtr client)> rtClientFactory,
    NClientParameters parameters,
    std::string serverHttpKey) {
  NTest::ClientFactory = clientFactory;
  NTest::RtClientFactory = rtClientFactory;
  NTest::ServerHttpKey = serverHttpKey;
  NTest::NClientParameters = parameters;

  Nakama::NLogger::initWithConsoleSink(Nakama::NLogLevel::Debug);

  // Run internals first (pure unit tests, no server)
  test_internals();

  // Launch all test groups sequentially to avoid resource contention on emulator
  // std::vector<std::thread> threads;
  auto startSuite = [](const char* suiteName, void (*suite)()) {
    runSuiteSafely(suiteName, suite);
  };

  startSuite("test_authentication", test_authentication);
  startSuite("test_session", test_session);
  startSuite("test_getAccount", test_getAccount);
  startSuite("test_users", test_users);
  startSuite("test_timeouts", test_timeouts);
  startSuite("test_disconnect", test_disconnect);
  startSuite("test_errors", test_errors);
  startSuite("test_restoreSession", test_restoreSession);
  startSuite("test_storage", test_storage);
  startSuite("test_groups", test_groups);
  startSuite("test_friends", test_friends);
  startSuite("test_linking", test_linking);
  startSuite("test_leaderboards", test_leaderboards);
  startSuite("test_tournaments_rest", test_tournaments_rest);
  startSuite("test_listMatches", test_listMatches);
  startSuite("test_channelMessages", test_channelMessages);
  startSuite("test_deleteAccount", test_deleteAccount);
  startSuite("test_notifications_rest", test_notifications_rest);
  startSuite("test_rpc_rest", test_rpc_rest);
  startSuite("test_profiling", test_profiling);
  // startSuite("test_stress", test_stress);
  startSuite("test_realtime", test_realtime);
  // startSuite("test_throughput", test_throughput);
  startSuite("test_cancellation", test_cancellation);

  for (auto& t : threads) {
    t.join();
  }

  // total stats
  uint32_t total = g_runTestsCount.load();
  uint32_t failed = g_failedTestsCount.load();
  uint32_t passed = total - failed;

  NLOG_INFO("Total tests : " + std::to_string(total));
  NLOG_INFO("Tests passed: " + std::to_string(passed) + " (");
  printPercent(std::cout, total, passed);
  NLOG_INFO("Tests failed: " + std::to_string(failed) + " (");
  printPercent(std::cout, total, failed);

  if (failed > 0) {
    NLOG_INFO("");
    NLOG_INFO("=== FAILED TESTS ===");
    std::lock_guard<std::mutex> lock(g_failedTestNamesMutex);
    for (const auto& name : g_failedTestNames) {
      NLOG_INFO("  - " + name);
    }
    NLOG_INFO("====================");
  }

  return failed == 0 ? 0 : -1;
}
} // namespace Test
} // namespace Nakama
