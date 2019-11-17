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
#include "TaskExecutor.h"
#include "nakama-cpp/NUtils.h"

eClientType g_clientType = ClientType_Unknown;

extern "C"
{
    extern void c_test_pure();

    eClientType getClientType(void)
    {
        return g_clientType;
    }
}

using namespace std;

namespace Nakama {
namespace Test {

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
void ctest_realtime();

// wrapper tests
void wrapper_test_authentication();
void wrapper_test_account();
void wrapper_test_realtime();

void setWorkingClientParameters(NClientParameters& parameters)
{
    parameters.host      = SERVER_HOST;
    parameters.port      = SERVER_PORT;
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
    if (getClientType() == ClientType_Grpc)
        client = createGrpcClient(parameters);
    else
        client = createRestClient(parameters);

    client->setErrorCallback([this](const NError& error)
    {
        stopTest();
    });
}

void NCppTest::tick()
{
    client->tick();
    TaskExecutor::instance().tick();
}

// *************************************************************

int runAllTests()
{
    test_authentication();
    test_getAccount();
    test_disconnect();
    test_errors();
    test_restoreSession();
    test_storage();
    test_groups();
    test_realtime();

    ctest_authentication();
    ctest_realtime();

    c_test_pure();

    wrapper_test_authentication();
    wrapper_test_account();
    wrapper_test_realtime();

    // total stats
    printTotalStats();

    return getFailedCount() == 0 ? 0 : -1;
}

// will try to connect to server until connected
class NConnectTest : public NCppTest
{
public:
    NConnectTest() : NCppTest("NConnectTest") {}

    void connect(uint32_t retryPeriodMs)
    {
        createWorkingClient();
        client->setErrorCallback([this, retryPeriodMs](const NError& error)
        {
            cout << "Not connected. Will retry in " << retryPeriodMs << " msec..." << endl;
            _retryAt = getUnixTimestampMs() + retryPeriodMs;
        });
        auth();
        runTest();
    }

    void auth()
    {
        cout << "Connecting..." << endl;

        auto successCallback = [this](NSessionPtr session)
        {
            cout << "Connected" << endl;
            stopTest(true);
        };
        client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);
    }

    void tick() override
    {
        NCppTest::tick();

        if (_retryAt != 0 && getUnixTimestampMs() >= _retryAt)
        {
            _retryAt = 0;
            auth();
        }
    }

private:
    NTimestamp _retryAt = 0;
};

} // namespace Test
} // namespace Nakama

int main()
{
    int res = 0;

    cout << "running nakama tests..." << endl;
    cout << endl;
    cout << "server config:" << endl;
    cout << "host     : " << SERVER_HOST << endl;
    cout << "gRPC port: " << SERVER_GRPC_PORT << endl;
    cout << "HTTP port: " << SERVER_HTTP_PORT << endl;
    cout << "key      : " << SERVER_KEY << endl;
    cout << "ssl      : " << (SERVER_SSL ? "true" : "false") << endl;
    cout << endl;

    Nakama::NLogger::initWithConsoleSink(Nakama::NLogLevel::Debug);

    Nakama::Test::NConnectTest connectTest;
    connectTest.connect(2000);

    // REST client tests
    if (Nakama::createRestClient({}))
    {
        g_clientType = ClientType_Rest;
        res = Nakama::Test::runAllTests();
        if (res != 0)
            return res;
    }

    // gRPC client tests
    if (Nakama::createGrpcClient({}))
    {
        g_clientType = ClientType_Grpc;
        res = Nakama::Test::runAllTests();
    }

    return res;
}
