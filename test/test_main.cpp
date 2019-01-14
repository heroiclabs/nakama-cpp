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

namespace Nakama {
namespace Test {

using namespace std;

void test_getAccount();
void test_authenticateDevice();
void test_authenticateDevice2();
void test_connectError();
void test_disconnect();

void setWorkingClientParameters(DefaultClientParameters& parameters)
{
}

// *************************************************************

NTest::NTest(const char * name)
{
    cout << endl << endl;
    cout << "*************************************" << endl;
    cout << "Running " << name << endl;
    cout << "*************************************" << endl;
}

void NTest::createWorkingClient()
{
    DefaultClientParameters parameters;

    setWorkingClientParameters(parameters);

    client = createDefaultClient(parameters);
}

void NTest::createClientWithParameters(const DefaultClientParameters & parameters)
{
    client = createDefaultClient(parameters);
}

void NTest::runTest()
{
    std::chrono::milliseconds sleep_period(15);

    while (_continue_loop)
    {
        client->tick();

        std::this_thread::sleep_for(sleep_period);
    }
}

void NTest::stopTest()
{
    _continue_loop = false;
}

// *************************************************************

int runAllTests()
{
    test_authenticateDevice();
    test_authenticateDevice2();
    test_getAccount();
    test_connectError();
    test_disconnect();

    return 0;
}

} // namespace Test
} // namespace Nakama

int main()
{
    return Nakama::Test::runAllTests();
}
