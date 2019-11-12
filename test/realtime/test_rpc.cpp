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
#include "TaskExecutor.h"

namespace Nakama {
namespace Test {

using namespace std;

// rpc tests reqire following lua modules:
// download them from
// https://github.com/heroiclabs/nakama/tree/master/data/modules
// put to nakama-server/data/modules
// restart server
void test_rpc()
{
    NRtClientTest test(__func__);

    test.onRtConnect = [&test]()
    {
        TaskExecutor& taskExecutor = TaskExecutor::instance();

        /////////////////////////////////////////////////////////////////
        // REST or gRPC rpc tests
        /////////////////////////////////////////////////////////////////
        taskExecutor.addTask([&test]()
        {
            string json = "{\"user_id\":\"" + test.session->getUserId() + "\"}";

            auto successCallback = [&test, json](const NRpc& rpc)
            {
                std::cout << "rpc response: " << rpc.payload << std::endl;
                NTEST_ASSERT(rpc.payload == json);
                TaskExecutor::instance().currentTaskCompleted();
            };

            test.client->rpc(
                test.session,
                "clientrpc.rpc",
                json,
                successCallback);
        });

        taskExecutor.addTask([&test]()
        {
            auto successCallback = [&test](const NRpc& rpc)
            {
                // this rpc call must fail
                NTEST_ASSERT(false);
                TaskExecutor::instance().currentTaskCompleted();
            };

            auto errorCallback = [](const NError& error)
            {
                NTEST_ASSERT(error.code == ErrorCode::InternalError);
                TaskExecutor::instance().currentTaskCompleted();
            };

            test.client->rpc(
                test.session,
                "clientrpc.rpc_error",
                "{}",
                successCallback,
                errorCallback);
        });

        taskExecutor.addTask([&test]()
        {
            auto successCallback = [&test](const NRpc& rpc)
            {
                std::cout << "rpc response: " << rpc.payload << std::endl;
                NTEST_ASSERT(!rpc.payload.empty());
                TaskExecutor::instance().currentTaskCompleted();
            };

            test.client->rpc(
                test.session,
                "clientrpc.rpc_get",
                "{}",
                successCallback);
        });

        taskExecutor.addTask([&test]()
        {
            auto successCallback = [&test](const NRpc& rpc)
            {
                std::cout << "rpc response: " << rpc.payload << std::endl;
                TaskExecutor::instance().currentTaskCompleted();
            };

            test.client->rpc(
                test.session,
                "clientrpc.send_notification",
                "{\"user_id\":\"" + test.session->getUserId() + "\"}",
                successCallback);
        });

        /////////////////////////////////////////////////////////////////
        // real-time rpc tests
        /////////////////////////////////////////////////////////////////
        taskExecutor.addTask([&test]()
        {
            string json = "{\"user_id\":\"" + test.session->getUserId() + "\"}";

            auto successCallback = [&test, json](const NRpc& rpc)
            {
                std::cout << "rpc response: " << rpc.payload << std::endl;
                NTEST_ASSERT(rpc.payload == json);
                TaskExecutor::instance().currentTaskCompleted();
            };

            test.rtClient->rpc(
                "clientrpc.rpc",
                json,
                successCallback);
        });

        taskExecutor.addTask([&test]()
        {
            auto successCallback = [&test](const NRpc& rpc)
            {
                // this rpc call must fail
                NTEST_ASSERT(false);
                TaskExecutor::instance().currentTaskCompleted();
            };

            auto errorCallback = [](const NRtError& error)
            {
                NTEST_ASSERT(error.code == RtErrorCode::RUNTIME_FUNCTION_EXCEPTION);
                TaskExecutor::instance().currentTaskCompleted();
            };

            test.rtClient->rpc(
                "clientrpc.rpc_error",
                "{}",
                successCallback,
                errorCallback);
        });

        taskExecutor.addTask([&test]()
        {
            auto successCallback = [&test](const NRpc& rpc)
            {
                std::cout << "rpc response: " << rpc.payload << std::endl;
                NTEST_ASSERT(!rpc.payload.empty());
                TaskExecutor::instance().currentTaskCompleted();
            };

            test.rtClient->rpc(
                "clientrpc.rpc_get",
                "{}",
                successCallback);
        });

        // test completion task, must be last task
        taskExecutor.addTask([&test]()
        {
            test.stopTest(true);
            TaskExecutor::instance().currentTaskCompleted();
        });
    };

    test.runTest();
}

} // namespace Test
} // namespace Nakama
