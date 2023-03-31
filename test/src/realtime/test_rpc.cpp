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

#include <exception>
#include "nakama-cpp/NError.h"
#include "globals.h"
#include "NTest.h"
#include "test_serverConfig.h"
#include "nakama-cpp/NException.h"
#include "nakama-cpp/realtime/rtdata/NRtException.h"
#include "TestGuid.h"

namespace Nakama {
    namespace Test {

    using namespace std;

    void test_rpc_with_http_key()
    {
        bool threadedTick = true;
        NTest test1(__func__, threadedTick);
        NSessionPtr session1 = test1.client->authenticateCustomAsync(TestGuid::newGuid()).get();
        bool createStatus = false;
        test1.rtClient->connectAsync(session1, createStatus).get();

        NTest test2(__func__, threadedTick);
        NSessionPtr session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid()).get();
        test1.rtClient->connectAsync(session2, createStatus).get();

        test1.runTest();
        test2.runTest();

        string json = "{\"v\":\"test\"}";
        auto rpc1 = test1.client->rpcAsync(SERVER_HTTP_KEY, "clientrpc.rpc", json).get();
        NTEST_ASSERT(!rpc1.payload.empty());

        auto rpc2 = test2.client->rpcAsync(SERVER_HTTP_KEY, "clientrpc.rpc", opt::nullopt).get();
        NTEST_ASSERT(rpc2.payload.empty());
    }

    void test_rpc_with_auth()
    {
        const bool threadedTick = true;
        NTest test(__func__, threadedTick);
        test.setTestTimeoutMs(100000);
        test.runTest();

        NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid()).get();

        auto rpc1 = test.client->rpcAsync(session, "clientrpc.rpc", opt::nullopt).get();
        NTEST_ASSERT(rpc1.payload.empty());

        string json = "{\"user_id\":\"" + session->getUserId() + "\"}";
        auto rpc2 = test.client->rpcAsync(session, "clientrpc.rpc", json).get();
        NTEST_ASSERT(rpc2.payload == json);

        try
        {
            auto rpc3 = test.client->rpcAsync(session, "clientrpc.rpc_error", "{}").get();
        }
        catch (const NException& e)
        {
            NTEST_ASSERT(e.error.code == ErrorCode::InternalError);
        }

        auto rpc4 = test.client->rpcAsync(session, "clientrpc.rpc_get",  "{}").get();
        NTEST_ASSERT(!rpc1.payload.empty());

        auto rpc5 = test.client->rpcAsync(session, "clientrpc.send_notification", "{\"user_id\":\"" + session->getUserId() + "\"}").get();
        NTEST_ASSERT(rpc5.payload.empty());

        auto rpc6 = test.rtClient->rpcAsync("clientrpc.rpc", opt::nullopt);
        json = "{\"user_id\":\"" + session->getUserId() + "\"}";

        auto rpc7 = test.rtClient->rpcAsync("clientrpc.rpc", json).get();
        NTEST_ASSERT(rpc7.payload == json);

        try
        {
            test.rtClient->rpcAsync("clientrpc.rpc_error", "{}").get();
        }
        catch(const NRtException& e)
        {
            std::cerr << e.what() << '\n';
            NTEST_ASSERT(e.error.code == RtErrorCode::RUNTIME_FUNCTION_EXCEPTION);
        }

        auto rpc8 = test.rtClient->rpcAsync("clientrpc.rpc_get", "{}").get();
        NTEST_ASSERT(!rpc8.payload.empty());
    }

    void test_rpc()
    {
        test_rpc_with_http_key();
        test_rpc_with_auth();
    }

    } // namespace Test
} // namespace Nakama
