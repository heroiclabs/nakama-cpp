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

#include "test_main.h"
#include "test_serverConfig.h"

extern "C"
{
    extern void c_test_pure();
}

namespace Nakama {
namespace Test {

using namespace std;

// C++ tests
void test_getAccount();
void test_authentication();
void test_errors();
void test_disconnect();
void test_restoreSession();
void test_storage();
void test_groups();
void test_realtime();

// C tests
void ctest_authentication();

// wrapper tests
void wrapper_test_authentication();
void wrapper_test_account();

void setWorkingClientParameters(NClientParameters& parameters)
{
    parameters.host      = SERVER_HOST;
    parameters.port      = SERVER_GRPC_PORT;
    parameters.serverKey = SERVER_KEY;
    parameters.ssl       = SERVER_SSL;
}

// *************************************************************
// NCppTest
// *************************************************************
NCppTest::NCppTest(const char* name) : NTest(name)
{
}

NCppTest::~NCppTest()
{
}

void NCppTest::createWorkingClient()
{
    NClientParameters parameters;

    setWorkingClientParameters(parameters);

    createClient(parameters);
}

void NCppTest::createClient(const NClientParameters& parameters)
{
    client = createDefaultClient(parameters);

    client->setErrorCallback([this](const NError& error)
    {
        stopTest();
    });
}

void NCppTest::tick()
{
    client->tick();
}

// *************************************************************

int runAllTests()
{
    NLogger::initWithConsoleSink(NLogLevel::Debug);

    test_authentication();
    test_getAccount();
    test_disconnect();
    test_errors();
    test_restoreSession();
    test_storage();
    test_groups();
    test_realtime();

    ctest_authentication();

    c_test_pure();

    wrapper_test_authentication();
    wrapper_test_account();

    // total stats
    printTotalStats();

    return getFailedCount() == 0 ? 0 : -1;
}

} // namespace Test
} // namespace Nakama

int main()
{
    return Nakama::Test::runAllTests();
}
