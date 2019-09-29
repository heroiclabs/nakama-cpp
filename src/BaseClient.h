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
#include "nakama-cpp/ClientFactory.h"

namespace Nakama {

    /**
     * Base client class
     */
    class BaseClient : public NClientInterface
    {
    public:
        void setErrorCallback(ErrorCallback errorCallback) override { _defaultErrorCallback = errorCallback; }

        void setUserData(void* userData) override { _userData = userData; }
        void* getUserData() const override { return _userData; }

        NRtClientPtr createRtClient(int32_t port, NRtTransportPtr transport) override;
        NRtClientPtr createRtClient(const RtClientParameters& parameters, NRtTransportPtr transport) override;

    protected:
        std::string _host;
        bool _ssl = false;
        std::string _basicAuthMetadata;
        ErrorCallback _defaultErrorCallback;
        void* _userData = nullptr;
    };
}
