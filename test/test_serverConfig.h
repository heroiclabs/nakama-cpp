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
#define SERVER_HOST          "127.0.0.1"
#define SERVER_GRPC_PORT     7349
#define SERVER_HTTP_PORT     7350
#define SERVER_SSL           false

typedef enum
{
    ClientType_Unknown,
    ClientType_Rest,
    ClientType_Grpc
} eClientType;

eClientType getClientType(void);

#define SERVER_PORT    getClientType() == ClientType_Grpc ? SERVER_GRPC_PORT : SERVER_HTTP_PORT

#ifdef __cplusplus
}
#endif
