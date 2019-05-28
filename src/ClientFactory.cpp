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

#include "nakama-cpp/ClientFactory.h"
#include "nakama-cpp/log/NLogger.h"
#include "GrpcClient.h"

#ifdef BUILD_REST_CLIENT
#include "RestClient.h"
#include "NHttpClientCppRest.h"
#endif

namespace Nakama {

NClientPtr createDefaultClient(const NClientParameters& parameters)
{
    //return createGrpcClient(parameters);
    return createRestClient(parameters);
}

NClientPtr createGrpcClient(const NClientParameters& parameters)
{
    NClientPtr client(new GrpcClient(parameters));
    return client;
}

NClientPtr createRestClient(const NClientParameters& parameters, NHttpClientPtr httpClient)
{
#ifdef BUILD_REST_CLIENT
    if (!httpClient)
    {
        httpClient.reset(new NHttpClientCppRest());
    }

    NClientPtr client(new RestClient(parameters, httpClient));
    return client;
#else
    NLOG_ERROR("REST client is not available");
    return nullptr;
#endif
}

}
