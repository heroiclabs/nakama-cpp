#include "test_main.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_getAccount();

void setWorkingClientParameters(DefaultClientParameters& parameters)
{
}

void test_authenticateDevice()
{
    cout << endl << __func__ << endl;

    DefaultClientParameters parameters;

    setWorkingClientParameters(parameters);

    ClientPtr client = createDefaultClient(parameters);
    bool continue_loop = true;

    auto successCallback = [&continue_loop](NSessionPtr session)
    {
        std::cout << "session token: " << session->getAuthToken() << std::endl;
        continue_loop = false;
    };

    auto errorCallback = [&continue_loop](const NError& error)
    {
        std::cout << "error: " << error.GetErrorMessage() << std::endl;
        continue_loop = false;
    };

    client->authenticateDevice("mytestdevice0000", "", true, successCallback, errorCallback);

    std::chrono::milliseconds sleep_period(15);

    while (continue_loop)
    {
        client->tick();

        std::this_thread::sleep_for(sleep_period);
    }
}

void test_authenticateDevice2()
{
    cout << endl << __func__ << endl;

    DefaultClientParameters parameters;

    setWorkingClientParameters(parameters);

    ClientPtr client = createDefaultClient(parameters);
    bool continue_loop = true;

    auto successCallback = [&continue_loop](NSessionPtr session)
    {
        std::cout << "session token: " << session->getAuthToken() << std::endl;
        continue_loop = false;
    };

    auto errorCallback = [&continue_loop](const NError& error)
    {
        std::cout << "error: " << error.GetErrorMessage() << std::endl;
        continue_loop = false;
    };

    client->authenticateDevice("mytestdevice0001", "", false, successCallback, errorCallback);

    std::chrono::milliseconds sleep_period(15);

    while (continue_loop)
    {
        client->tick();

        std::this_thread::sleep_for(sleep_period);
    }
}

void test_connectError()
{
    cout << endl << __func__ << endl;

    DefaultClientParameters parameters;

    parameters.port = 1111;

    ClientPtr client = createDefaultClient(parameters);
    bool continue_loop = true;

    auto successCallback = [&continue_loop](NSessionPtr session)
    {
        std::cout << "session token: " << session->getAuthToken() << std::endl;
        continue_loop = false;
    };

    auto errorCallback = [&continue_loop](const NError& error)
    {
        std::cout << "error: " << error.GetErrorMessage() << std::endl;
        continue_loop = false;
    };

    client->authenticateDevice("mytestdevice0001", "", false, successCallback, errorCallback);

    std::chrono::milliseconds sleep_period(15);

    while (continue_loop)
    {
        client->tick();

        std::this_thread::sleep_for(sleep_period);
    }
}

void test_disconnect()
{
    cout << endl << __func__ << endl;

    DefaultClientParameters parameters;

    ClientPtr client = createDefaultClient(parameters);
    bool continue_loop = true;

    auto successCallback = [&continue_loop](NSessionPtr session)
    {
        std::cout << "session token: " << session->getAuthToken() << std::endl;
        continue_loop = false;
    };

    auto errorCallback = [&continue_loop](const NError& error)
    {
        std::cout << "error: " << error.GetErrorMessage() << std::endl;
        continue_loop = false;
    };

    client->authenticateDevice("mytestdevice0001", "", false, successCallback, errorCallback);

    client->disconnect();

    std::chrono::milliseconds sleep_period(15);

    while (continue_loop)
    {
        client->tick();

        std::this_thread::sleep_for(sleep_period);
    }
}

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
