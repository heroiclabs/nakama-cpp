/*
* Copyright 2018 The Nakama Authors
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

#include <string>

namespace nakama {

    /**
     * A client interface to interact with Nakama server.
     */
    class ClientInterface
    {
    public:
        virtual ~ClientInterface() {}

        /**
         * Authenticate a user with a device id.
         * @param id A device identifier usually obtained from a platform API.
         * @param username A username used to create the user. Defaults to empty string.
         * @param create True if the user should be created when authenticated. Defaults to false.
         */
        virtual void authenticateDevice(
            const std::string& id,
            const std::string& username = std::string(),
            bool create = false
        ) = 0;

    };
}
