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

#include <rapidjson/document.h>

namespace Nakama {
namespace Test {

using namespace std;

void test_authoritative_match() {
  bool threadedTick = true;
  NTest test(__func__, threadedTick);
  NTest test2("test_authoritative_match_join", threadedTick);

  test.runTest();
  test2.runTest();

  NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  bool createStatus = false;
  test.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();

  NSessionPtr session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  test2.rtClient->connectAsync(session2, createStatus, NTest::RtProtocol).get();

  const NRpc rpc =
      test.rtClient
          ->rpcAsync("clientrpc.create_authoritative_match", "{\"debug\": true, \"label\": \"TestAuthoritativeMatch\"}")
          .get();

  rapidjson::Document document;
  if (!document.Parse(rpc.payload).HasParseError()) {
    auto& jsonMatchId = document["match_id"];

    if (jsonMatchId.IsString()) {
      string matchId = jsonMatchId.GetString();
      test2.rtClient->joinMatchAsync(matchId, {}).get();
    } else {
      test.stopTest(false);
    }
  } else {
    test.stopTest(false);
  }

  test.stopTest(true);
  test2.stopTest(true);
}

} // namespace Test
} // namespace Nakama
