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

#if defined(__ANDROID__)
#include <android_native_app_glue.h>
#include <jni.h>
#endif


#if defined(__UNREAL__)
#include "NakamaUnreal.h"
#endif

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
void test_internals();

static std::string g_serverHost = SERVER_HOST;

void setWorkingClientParameters(NClientParameters& parameters)
{
    parameters.host      = g_serverHost;
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

void NCppTest::createWorkingClient()
{
    NLOG_INFO("Creating a working client.");

    NClientParameters parameters;

    NLOG_INFO("setting params.");

    setWorkingClientParameters(parameters);

    NLOG_INFO("creating client.");

    createClient(parameters);
}

NClientPtr NCppTest::createClient(const NClientParameters& parameters)
{
#if !defined(__UNREAL__)
    NLOG_INFO("Creating default client.");

    client = createDefaultClient(parameters);

    NLOG_INFO("done creating default client.");

#else
        client = Nakama::Unreal::createNakamaClient(parameters, Nakama::NLogLevel::Debug);
#endif
    if (client)
    {
        client->setErrorCallback([this](const NError& error) { stopTest(error); });
    }
    return client;
}

void NCppTest::tick()
{
    client->tick();
    TaskExecutor::instance().tick();
}

// *************************************************************

int runAllTests()
{
    test_internals();
    test_authentication();
    test_getAccount();
    test_disconnect();
    test_errors();
    test_restoreSession();
    test_storage();
    test_groups();
    test_realtime();

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
        NLOG_INFO("connecting");
        createWorkingClient();

        client->setErrorCallback([this, retryPeriodMs](const NError& /*error*/)
        {
            NLOG(Nakama::NLogLevel::Info, "Not connected. Will retry in %d msec...",  retryPeriodMs);
            _retryAt = getUnixTimestampMs() + retryPeriodMs;
        });
        auth();
        runTest();
    }

    void auth()
    {
        NLOG_INFO("Connecting...");

        auto successCallback = [this](NSessionPtr /*session*/)
        {
            NLOG_INFO("Connected");
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


int mainHelper(int argc, char *argv[])
{
    int res = 0;

    if (argc > 1) {
        Nakama::Test::g_serverHost = argv[1];
    }

#if !defined(__UNREAL__)
    Nakama::NLogger::initWithConsoleSink(Nakama::NLogLevel::Debug);
#endif

    NLOG(Nakama::NLogLevel::Info, "server config...");
    NLOG(Nakama::NLogLevel::Info, "host     : %s", Nakama::Test::g_serverHost.c_str());
    NLOG(Nakama::NLogLevel::Info, "HTTP port: %d", SERVER_HTTP_PORT);
    NLOG(Nakama::NLogLevel::Info, "key      : %s", SERVER_KEY);
    NLOG(Nakama::NLogLevel::Info, "ssl      : %s", (SERVER_SSL ? "true" : "false"));


    Nakama::NLogger::Info("starting tests", "hc");
    Nakama::Test::NConnectTest connectTest;
    connectTest.connect(2000);
    Nakama::NLogger::Info("done calling connect", "hc");

    // REST client tests
    g_clientType = ClientType_Rest;
    if (Nakama::Test::NCppTest("").createClient({})) {
        res = Nakama::Test::runAllTests();
        if (res != 0) {
            return res;
        }
    };

    return res;
}

#if defined(_MSC_VER)
#pragma warning(disable:4447)
#endif

#ifdef __UNREAL__
int test_main(int argc, const char *argv[])
{
    mainHelper(0, nullptr);
}
#elif defined(__ANDROID__)
extern "C"
{
    void android_main(struct android_app* app)
    {
        mainHelper(0, nullptr);
    }
}
#else
int main(int argc, char *argv[])
{
    mainHelper(argc, argv);
}

#endif
