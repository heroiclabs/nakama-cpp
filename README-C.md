Nakama C client
=============
> General C client for Nakama server.

The C client is just wrapper on top of C++ client.

Here is described only C client usage.

For other information please follow main README.md.

## Usage

The client object has many methods to execute various features in the server or open realtime socket connections with the server.

Include nakama header.

```c
#include "nakama-c/Nakama.h"
```

Use the connection credentials to build a client object.

```c
NClient client;
tNClientParameters parameters;
parameters.serverKey = "defaultkey";
parameters.host = "127.0.0.1";
parameters.port = NDEFAULT_PORT;
parameters.ssl = false;
client = createDefaultNakamaClient(&parameters);
```

The `createDefaultNakamaClient` will create HTTP/1.1 client to use REST API.

### Tick

The `tick` method pumps requests queue and executes callbacks in your thread. You must call it periodically (recommended every 50ms) in your thread.

```c
NClient_tick(client);
if (rtClient)
    NRtClient_tick(rtClient);
```

Without this the default client and realtime client will not work, and you will not receive responses from the server.

### Authenticate

There's a variety of ways to [authenticate](https://heroiclabs.com/docs/authentication) with the server. Authentication can create a user if they don't already exist with those credentials. It's also easy to authenticate with a social profile from Google Play Games, Facebook, Game Center, etc.

```c
void successCallback(NClient client, NClientReqData reqData, NSession session)
{
    const char* token = NSession_getAuthToken(session);

    printf("session token: %s\n", token);

    NSession_destroy(session);
}

void errorCallback(NClient client, NClientReqData reqData, const sNError* error)
{
};

NClient_authenticateEmail(
        client,
        "super@heroes.com", // mail
        "batsignal",        // password
        "test",             // username
        true,               // create if not created yet
        NULL,               // session vars
        0,                  // custom req data which you will receive back in callback
        successCallback,
        errorCallback
    );
```

### Sessions

When authenticated the server responds with an auth token (JWT) which contains useful properties and gets deserialized into a `NSession` object.

```c
const char* token = NSession_getAuthToken(session);
const char* userId = NSession_getUserId(session);
const char* username = NSession_getUsername(session);
bool expired = NSession_isExpired(session);
NTimestamp expireTime = NSession_getExpireTime(session);
```

It is recommended to store the auth token from the session and check at startup if it has expired. If the token has expired you must reauthenticate. The expiry time of the token can be changed as a setting in the server.

```c
const char* authtoken = "restored from somewhere";
NSession session = restoreNakamaSession(authtoken);
if (NSession_isExpired(session)) {
    printf("Session has expired. Must reauthenticate!\n");
}
```

### Requests

The client includes lots of builtin APIs for various features of the game server. These can be accessed with the async methods. It can also call custom logic as RPC functions on the server. These can also be executed with a socket object.

All requests are sent with a session object which authorizes the client.

```c
void successCallback(NClient client, NClientReqData reqData, const sNAccount* account)
{
    printf("user id : %s\n", account->user.id);
    printf("username: %s\n", account->user.username);
    printf("wallet  : %s\n", account->wallet);
};

NClient_getAccount(client, session, 0, successCallback, errorCallback);
```

### Realtime client

The client can create one or more realtime clients with the server. Each realtime client can have it's own events listener registered for responses received from the server.

```c
void connectCallback(NRtClient client)
{
    printf("Socket connected\n");
}

bool createStatus = true; // if the socket should show the user as online to others.
NRtClient rtClient = NClient_createRtClient(client, NDEFAULT_PORT);
NRtClient_setConnectCallback(rtClient, connectCallback);
NRtClient_connect(rtClient, session, createStatus, NRtClientProtocol_Protobuf);
```

Don't forget to call `tick`. See [Tick](#tick) section for details.

### Clean up

Destroy the session when you don't need it anymore.

```c
NSession_destroy(session);
```

Destroy the client when you don't need it anymore.

```c
destroyNakamaClient(client);
```

Destroy the real-time client when you don't need it anymore.

```c
NRtClient_destroy(rtClient);
```

### Logging

Client logging is off by default.

To enable logs output to console with debug logging level:

```c
NLogger_initWithConsoleSink(NLogLevel_Debug);
```

To enable logs output to custom sink with debug logging level:

```c
void logSink(eNLogLevel level, const char* message, const char* func)
{
    printf("%s: %s\n", func, message);
}

NLogger_init(logSink, NLogLevel_Debug);
```

## Build

We don't distribute releases with C language support, so you have to build it by self.

Do not worry, it's not that hard.

Please follow build section from main README.md and edit `build/build_config.py`:

```py
BUILD_C_API = True
```

## License

This project is licensed under the [Apache-2 License](https://github.com/heroiclabs/nakama-dotnet/blob/master/LICENSE).

## Special Thanks

Thanks to [@dimon4eg](https://github.com/dimon4eg) for this excellent support on developing Nakama C/C++, Cocos2d-x and Unreal client libraries.
