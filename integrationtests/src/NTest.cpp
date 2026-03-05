
/*
 * Copyright 2023 The Nakama Authors
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
#include "globals.h"
#include "nakama-cpp/Nakama.h"
#include <atomic>
#include <future>
#include <iostream>
#include <mutex>

namespace Nakama {
namespace Test {

NRtClientProtocol NTest::RtProtocol = NRtClientProtocol::Json;
std::function<NClientPtr(Nakama::NClientParameters)> NTest::ClientFactory = nullptr;
std::function<NRtClientPtr(Nakama::NClientPtr)> NTest::RtClientFactory = nullptr;
NClientParameters NTest::NClientParameters = {};
std::string NTest::ServerHttpKey = "defaulthttpkey";

NTest::NTest(std::string name, bool threadedTick)
    : _name(name), _threadedTick(threadedTick), _rtTickPaused(false),
      client(NTest::ClientFactory(NTest::NClientParameters)), rtClient(NTest::RtClientFactory(client)) {
  client->setErrorCallback([this](const NError& error) { stopTest(error); });
  rtClient->setListener(&listener);
  _isDone.store(false);
}

NTest::NTest(std::string name, Nakama::NClientParameters parameters)
    : _name(name), _threadedTick(false), client(NTest::ClientFactory(parameters)),
      rtClient(NTest::RtClientFactory(client)) {
  client->setErrorCallback([this](const NError& error) { stopTest(error); });
  rtClient->setListener(&listener);
}

NTest::NTest(const char* name, bool threadedTick) : NTest(std::string(name), threadedTick) {}

NTest::~NTest() {
  if (_threadedTick) {
    _tickThread.join();
  }
}

void NTest::runTest() {

  if (_threadedTick) {
    _tickThread = std::thread(&NTest::runTestInternal, this);
  } else {
    runTestInternal();
  }
}

void NTest::addSession(NSessionPtr session) {
  _sessionsToCleanup.push_back(std::move(session));
}

void NTest::cleanupSessions() {
  // Fire all delete requests concurrently
  std::vector<std::future<void>> futures;
  for (auto& session : _sessionsToCleanup) {
    try {
      futures.push_back(client->deleteAccountAsync(session));
    } catch (...) {
    }
  }

  // Pump the HTTP client on a detached thread so we can abandon it
  // if tick() hangs (libHttpClient XTaskQueueDispatch can block when
  // the queue is torn down by another thread's HCCleanup).
  auto sharedFutures = std::make_shared<std::vector<std::future<void>>>(std::move(futures));
  auto clientCopy = client; // extend lifetime via shared_ptr copy
  auto cleanupDone = std::make_shared<std::atomic<bool>>(false);

  std::thread cleanupThread([clientCopy, sharedFutures, cleanupDone]() {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    bool allDone = false;
    while (!allDone && std::chrono::steady_clock::now() < deadline) {
      clientCopy->tick();
      allDone = true;
      for (auto& f : *sharedFutures) {
        if (f.valid() && f.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
          allDone = false;
        }
      }
      if (!allDone) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
    for (auto& f : *sharedFutures) {
      try {
        if (f.valid()) f.get();
      } catch (...) {
      }
    }
    cleanupDone->store(true);
  });

  // Wait up to 10s for the cleanup thread, then abandon it
  auto waitStart = std::chrono::steady_clock::now();
  while (!cleanupDone->load() &&
         std::chrono::steady_clock::now() - waitStart < std::chrono::seconds(10)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  if (cleanupDone->load()) {
    cleanupThread.join();
  } else {
    NLOG_WARN("Session cleanup timed out (tick blocked), abandoning cleanup thread");
    cleanupThread.detach();
  }

  _sessionsToCleanup.clear();
}

void NTest::runTestInternal() {
  if (g_runTestsCount > 0) {
    std::cout << std::endl << std::endl;
  }

  ++g_runTestsCount;

  printTestName("Running");

  while (!isDone()) {
    if (!checkTimeout(5)) {
      NLOG_INFO("Test timeout");
      stopTest(isSucceeded());
    }

    tick();

    std::chrono::milliseconds sleep_period(5);
    std::this_thread::sleep_for(sleep_period);
  }

  cleanupSessions();

  NLOG_INFO("done running tst internal");
}

void NTest::stopTest(bool succeeded) {
  _testSucceeded.store(succeeded);
  _isDone.store(true);

  if (succeeded) {
    printTestName("Succeeded");
  } else {
    ++g_failedTestsCount;
    {
      std::lock_guard<std::mutex> lock(g_failedTestNamesMutex);
      g_failedTestNames.push_back(_name);
    }
    printTestName("Failed");
    std::cout << std::flush;
    // abort();
  }
}

void NTest::stopTest(const NError& error) {
  NLOG_ERROR("Stopping test with error: " + toString(error));
  stopTest(false);
}

void NTest::printTestName(const char* event) {
  NLOG_INFO("*************************************");
  NLOG_INFO(std::string(event) + " " + _name);
  NLOG_INFO("*************************************");
}

void NTest::tick() {
  client->tick();

  if (!_rtTickPaused) {
    rtClient->tick();
  }
}
} // namespace Test
} // namespace Nakama
