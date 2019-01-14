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

#pragma once

#include "nakama-cpp/NClientInterface.h"

namespace Nakama {

    /**
     * @param serverKey The key used to authenticate with the server without a session. Defaults to "defaultkey".
     * @param host The host address of the server. Defaults to "127.0.0.1".
     * @param port The port number of the server. Defaults to 7349.
     */
    struct DefaultClientParameters
    {
        std::string serverKey = "defaultkey";
        std::string host = "127.0.0.1";
        int port = 7349;
    };

    /**
     * Creates a default client to interact with Nakama server.
     */
    NClientPtr createDefaultClient(const DefaultClientParameters& parameters);

}
