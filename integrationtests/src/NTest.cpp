
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

  // Only pump the HTTP client — the RT client is done and late WS messages
  // could try to fulfill destroyed promises.
  bool allDone = false;
  while (!allDone) {
    client->tick();
    allDone = true;
    for (auto& f : futures) {
      if (f.valid() && f.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        allDone = false;
      }
    }
    if (!allDone) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  // Consume results — best-effort, don't fail the test on cleanup errors
  for (auto& f : futures) {
    try {
      if (f.valid()) f.get();
    } catch (...) {
    }
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
