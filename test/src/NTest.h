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

#pragma once

#include <nakama-cpp/ClientFactory.h>
#include <nakama-cpp/NClientInterface.h>
#include <nakama-cpp/realtime/NRtDefaultClientListener.h>
#include <nakama-cpp/NError.h>

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

namespace Nakama {
namespace Test {

class NTest {
public:
  static NRtClientProtocol RtProtocol;
  static std::function<NClientPtr(Nakama::NClientParameters)> ClientFactory;
  static std::function<NRtClientPtr(Nakama::NClientPtr)> RtClientFactory;
  static Nakama::NClientParameters NClientParameters;
  static std::string ServerHttpKey;
  NTest(const char* name, bool threadedTick = false);
  NTest(std::string name, bool threadedTick = false);
  NTest(std::string name, Nakama::NClientParameters parameters);
  ~NTest();

  virtual void runTest();

  void stopTest(bool succeeded = false);
  void stopTest(const NError& error);

  void setRtTickPaused(bool paused = true) { _rtTickPaused = paused; }

  void tick();
  bool checkTimeout(int timePassedMs) {
    timeoutMs -= timePassedMs;
    return timeoutMs >= 0;
  }
  bool isDone() const { return _isDone.load(); }
  bool isSucceeded() const { return _testSucceeded.load(); }

  void setTestTimeoutMs(int ms) { timeoutMs = ms; }

  const NClientPtr client;
  const NRtClientPtr rtClient;
  NRtDefaultClientListener listener;

private:
  void printTestName(const char* event);
  std::atomic<bool> _isDone;
  std::atomic<bool> _testSucceeded;
  bool _threadedTick = false;
  std::string _name;
  std::thread _tickThread;
  int timeoutMs = 60 * 1000;
  bool _rtTickPaused;
  void runTestInternal();
};

} // namespace Test
} // namespace Nakama
