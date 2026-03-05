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

namespace Nakama {
namespace Test {

using namespace std;

void test_rt_create_match() {
  const bool threadedTick = true;
  NTest test1(__func__, threadedTick);

  test1.runTest();

  NSessionPtr session = test1.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  bool createStatus = false;
  test1.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();

  NMatch match = test1.rtClient->createMatchAsync().get();

  test1.stopTest(true);

  NLOG_INFO("stopped create match");
}

void test_rt_matchmaker() {
  NLOG_INFO("started testing matchmaker");

  bool threadedTick = true;
  NTest test1(__func__, threadedTick);
  NTest test2(std::string(__func__) + std::string("2"), threadedTick);

  test1.setTestTimeoutMs(20000);
  test2.setTestTimeoutMs(20000);

  test1.runTest();
  test2.runTest();

  NLOG_INFO("authing");

  NSessionPtr session = test1.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  bool createStatus = false;
  test1.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();

  NLOG_INFO("done 1");

  NSessionPtr session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  test2.rtClient->connectAsync(session2, createStatus, NTest::RtProtocol).get();

  NLOG_INFO("done 1");
  NLOG_INFO("connected");

  auto matchedPromise = std::promise<NMatchmakerMatchedPtr>();
  auto matched2Promise = std::promise<NMatchmakerMatchedPtr>();

  test1.listener.setMatchmakerMatchedCallback(
      [&test1, &matchedPromise](NMatchmakerMatchedPtr matched) { matchedPromise.set_value(matched); });

  test2.listener.setMatchmakerMatchedCallback(
      [&test2, &matched2Promise](NMatchmakerMatchedPtr matched) { matched2Promise.set_value(matched); });

  const int minCount = 2;
  const int maxCount = 2;
  const std::string query = "*";
  const NStringMap stringProperties = {};
  const NStringDoubleMap numericProperties = {};
  const int countMultiple = 1;

  NLOG_INFO("adding matchmaker");

  NMatchmakerTicket ticket =
      test1.rtClient->addMatchmakerAsync(minCount, maxCount, query, stringProperties, numericProperties, countMultiple)
          .get();
  NMatchmakerTicket ticket2 =
      test2.rtClient->addMatchmakerAsync(minCount, maxCount, query, stringProperties, numericProperties, countMultiple)
          .get();

  NLOG_INFO("got tickets");

  NMatchmakerMatchedPtr matched = matchedPromise.get_future().get();
  NMatchmakerMatchedPtr matched2 = matched2Promise.get_future().get();

  NLOG_INFO("got matched");
  NLOG_INFO(matched->matchId);
  NLOG_INFO(matched2->matchId);

  test1.rtClient->joinMatchByTokenAsync(matched->token).get();
  test2.rtClient->joinMatchByTokenAsync(matched2->token).get();

  test1.stopTest(true);
  test2.stopTest(true);
}

void test_rt_match() {
  test_rt_create_match();
  test_rt_matchmaker();
}

} // namespace Test
} // namespace Nakama
