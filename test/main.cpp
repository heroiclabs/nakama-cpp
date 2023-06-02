/*
 * Copyright 2023 The Nakama Authors
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

#include <memory.h>
#include "nakama-cpp/realtime/NWebsocketsFactory.h"
#include "NTestLib.h"

#if defined(__ANDROID__)
#include <jni.h>
#endif

#if defined(_MSC_VER)
#pragma warning(disable:4447)
#endif

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

#ifdef __cplusplus
extern "C" {
#endif

#define SERVER_KEY           "defaultkey"
#define SERVER_HTTP_KEY      "defaulthttpkey"
#define SERVER_HOST          "127.0.0.1"
#define SERVER_HTTP_PORT     7350
#define SERVER_SSL           false

#define SERVER_PORT SERVER_HTTP_PORT

#ifdef __cplusplus
}
#endif


#if defined(__ANDROID__)
extern "C"
{
    JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
    {
        Nakama::Test::runAllTests();
        return JNI_VERSION_1_4;
    }
}
#else
int main(int argc, char *argv[])
{
    auto clientFactory = [](Nakama::NClientParameters parameters)->Nakama::NClientPtr{
        return Nakama::createDefaultClient({SERVER_KEY, SERVER_HOST, SERVER_PORT, SERVER_SSL});
    };

    auto rtClientFactory = [](Nakama::NClientPtr client)->Nakama::NRtClientPtr{
        return client->createRtClient();
    };

    return Nakama::Test::runAllTests(clientFactory, rtClientFactory, SERVER_HTTP_KEY);
}
#endif
