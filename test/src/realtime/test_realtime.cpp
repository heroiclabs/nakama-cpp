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

#include <thread>
#include "NTest.h"
#include "globals.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_rt_match();
void test_notifications();
void test_authoritative_match();
void test_tournament();
void test_rpc();
void test_rt_party();


void run_realtime_tests()
{
//    test_rt_joinChat();
//    test_rt_joinGroupChat();
      test_rt_match();
//    test_notifications();
//    test_authoritative_match();
//    test_tournament();
//    test_rpc();
//    test_rt_party();
}

void test_realtime()
{
    // These tests are not protocol specific
//    test_rt_quickdestroy();
//    test_rt_rapiddisconnect();
//    //// change to 10 iterations to trigger https://github.com/microsoft/libHttpClient/issues/698 bug
//    for (int i = 0; i < 1; i++) {
//        test_rt_reconnect();
//    }
//    test_rt_heartbeat();

    run_realtime_tests();

    NTest::protocol = NRtClientProtocol::Json;
    NLOG_INFO("using Json protocol");

//    run_realtime_tests();
}

} // namespace Test
} // namespace Nakama
