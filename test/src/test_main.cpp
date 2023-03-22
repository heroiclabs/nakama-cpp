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

#include "test_serverConfig.h"
#include "nakama-cpp/NUtils.h"
#include "nakama-cpp/NPlatformParams.h"
#include "nakama-cpp/ClientFactory.h"
#include "globals.h"

#if defined(__ANDROID__)
#include <jni.h>
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
void test_friends();
void test_listMatches();
void test_realtime();
void test_internals();

ostream& printPercent(ostream& os, uint32_t totalCount, uint32_t count)
{
    if (totalCount > 0)
    {
        os << count * 100 / totalCount << "%";
    }
    else
    {
        os << "0%";
    }

    return os;
}

int runAllTests()
{
//    test_internals();
//    test_authentication();
//    test_getAccount();
//    test_disconnect();
//    test_errors();
//    test_restoreSession();
//    test_storage();
//    test_groups();
//    test_friends();
    test_realtime();
//    test_listMatches();

    // total stats
    uint32_t testsPassed = (g_runTestsCount - g_failedTestsCount);

    NLOG_INFO("Total tests : " + std::to_string(g_runTestsCount));
    NLOG_INFO("Tests passed: " + std::to_string(testsPassed) +" (");
    printPercent(cout, g_runTestsCount, testsPassed);
    NLOG_INFO("Tests failed: " + std::to_string(g_failedTestsCount) + " (");
    printPercent(cout, g_runTestsCount, g_failedTestsCount);

    return g_failedTestsCount == 0 ? 0 : -1;
}


} // namespace Test
} // namespace Nakama


int mainHelper(int argc, char *argv[])
{
    int res = 0;

    Nakama::NLogger::initWithConsoleSink(Nakama::NLogLevel::Debug);

    NLOG(Nakama::NLogLevel::Info, "server config...");
    NLOG(Nakama::NLogLevel::Info, "host     : %s", SERVER_HOST);
    NLOG(Nakama::NLogLevel::Info, "HTTP port: %d", SERVER_HTTP_PORT);
    NLOG(Nakama::NLogLevel::Info, "key      : %s", SERVER_KEY);
    NLOG(Nakama::NLogLevel::Info, "ssl      : %s", (SERVER_SSL ? "true" : "false"));


    // REST client tests
    g_clientType = ClientType_Rest;
    res = Nakama::Test::runAllTests();
    if (res != 0) {
        return res;
    }

    return res;
}

#if defined(_MSC_VER)
#pragma warning(disable:4447)
#endif

#if defined(__ANDROID__)
extern "C"
{
    JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
    {
        mainHelper(1, nullptr);
        return JNI_VERSION_1_4;
    }

}
#else
int main(int argc, char *argv[])
{
    mainHelper(argc, argv);
}

#endif
