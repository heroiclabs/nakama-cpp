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

#include "nakama-cpp/log/NLogger.h"
#include "nakama-cpp/NUtils.h"
#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "NTest.h"
#include "TestGuid.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_tournament()
{
    bool threadedTick = true;

    NTest test(__func__, threadedTick);
    NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();

    test.runTest();
    bool createStatus = false;
    test.rtClient->connectAsync(session, createStatus).get();


    NTimestamp start_time = getUnixTimestampMs() / 1000; // starts now in seconds
    uint32_t duration = 5;                               // in seconds
    string operator_ = "best";                           // one of : "best", "set", "incr"
    string reset_schedule = "";                          // none
    NTimestamp end_time = start_time + 5;                // end after 5 sec
    uint32_t max_size = 10000;                           // first 10,000 players who join
    uint32_t max_num_score = 3;                          // each player can have 3 attempts to score
    bool join_required = true;                           // must join to compete

    rapidjson::Document document;
    document.SetObject();

    document.AddMember("authoritative", true, document.GetAllocator());
    document.AddMember("sort_order", "desc", document.GetAllocator());
    document.AddMember("operator", operator_, document.GetAllocator());
    document.AddMember("duration", duration, document.GetAllocator());
    document.AddMember("reset_schedule", reset_schedule, document.GetAllocator());
    document.AddMember("title", "Daily Dash", document.GetAllocator());
    document.AddMember("description", "Dash past your opponents for high scores and big rewards!", document.GetAllocator());
    document.AddMember("category", 1, document.GetAllocator());
    document.AddMember("start_time", start_time, document.GetAllocator());
    document.AddMember("end_time", end_time, document.GetAllocator());
    document.AddMember("max_size", max_size, document.GetAllocator());
    document.AddMember("max_num_score", max_num_score, document.GetAllocator());
    document.AddMember("join_required", join_required, document.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    string json = buffer.GetString();

    const NRpc& rpc = test.rtClient->rpcAsync("clientrpc.create_tournament", json).get();

    NLOG_INFO("rpc response: " + rpc.payload);

            rapidjson::Document responseDocument;
            if (responseDocument.Parse(rpc.payload).HasParseError())
            {
                test.stopTest();
            }
            else
            {
                auto& jsonTournamentId = responseDocument["tournament_id"];

                if (jsonTournamentId.IsString())
                {
                    string tournamentId = jsonTournamentId.GetString();

                    auto successCallback = [&, tournamentId]()
                    {
                        NLOG_INFO("Successfully joined tournament");

                        auto successCallback = [&](const NRpc& /*rpc*/)
                        {
                            NLOG_INFO("tournament deleted.");
                            test.stopTest(true);
                        };

                        test.rtClient->rpc(
                            "clientrpc.delete_tournament",
                            "{\"tournament_id\":\"" + tournamentId + "\"}",
                            successCallback);
                    };

                    test.client->joinTournament(
                        session,
                        tournamentId,
                        successCallback);
                }
                else
                {
                    test.stopTest();
                }
            }
        };
    }

}
