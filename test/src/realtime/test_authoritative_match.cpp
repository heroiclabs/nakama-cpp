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

#include "realtime/RtClientTestBase.h"
#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_rt_match_join(NRtClientTest& test, const std::string& match_id)
{
    auto successCallback = [&test](const NMatch& match)
    {
        NLOG_INFO("joined match: " + match.matchId);

        std::string data = "Anybody there?";

        test.rtClient->sendMatchData(
            match.matchId,
            1, // op code
            data,
            {}
        );
        test.stopTest(true);
    };

    test.rtClient->joinMatch(
        match_id,
        {},
        successCallback);
}

void test_authoritative_match()
{
    NRtClientTest test(__func__);
    NRtClientTest test2("test_authoritative_match_join");

    test.onRtConnect = [&]()
    {
        auto successCallback = [&](const NRpc& rpc)
        {
            NLOG_INFO("rpc response: " + rpc.payload);

            bool succeeded = false;
            rapidjson::Document document;
            if (!document.Parse(rpc.payload).HasParseError())
            {
                auto& jsonMatchId = document["match_id"];

                if (jsonMatchId.IsString())
                {
                    string matchId = jsonMatchId.GetString();

                    test2.onRtConnect = [&test2, matchId]()
                    {
                        test_rt_match_join(test2, matchId);
                    };

                    test2.runTest();
                    succeeded = true;
                }
            }

            test.stopTest(succeeded);
        };

        test.rtClient->rpc(
            "clientrpc.create_authoritative_match",
            "{\"debug\": true, \"label\": \"TestAuthoritativeMatch\"}",
            successCallback);
    };

    test.runTest();
}

} // namespace Test
} // namespace Nakama
