#include "nakama-cpp/default_client.h"

using namespace nakama;

int main()
{
    DefaultClientParameters parameters;

    ClientInterface* client = createDefaultClient(parameters);

    client->authenticateDevice("mytestdevice0000");

    delete client;
    return 0;
}
