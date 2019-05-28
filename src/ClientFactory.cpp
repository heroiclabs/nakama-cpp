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

#ifdef BUILD_GRPC_CLIENT
    #include "GrpcClient.h"
#endif

#ifdef BUILD_REST_CLIENT
    #include "RestClient.h"
#endif

#ifdef BUILD_HTTP_CPPREST
    #include "NHttpClientCppRest.h"
#endif

namespace Nakama {

NClientPtr createDefaultClient(const NClientParameters& parameters)
{
#ifdef BUILD_REST_CLIENT

    return createRestClient(parameters);

#elif defined(BUILD_GRPC_CLIENT)

    return createGrpcClient(parameters);

#else

    NLOG_ERROR("No default client is available");
    return nullptr;

#endif
}

NClientPtr createGrpcClient(const NClientParameters& parameters)
{
#ifdef BUILD_GRPC_CLIENT

    NClientPtr client(new GrpcClient(parameters));
    return client;

#else

    NLOG_ERROR("gRPC client is not available");
    return nullptr;

#endif
}

NClientPtr createRestClient(const NClientParameters& parameters, NHttpTransportPtr httpTransport)
{
#ifdef BUILD_REST_CLIENT

    if (!httpTransport)
    {
        httpTransport = createDefaultHttpTransport();

        if (!httpTransport)
        {
            NLOG_ERROR("Unable create REST client without transport");
            return nullptr;
        }
    }

    NClientPtr client(new RestClient(parameters, httpTransport));
    return client;

#else

    NLOG_ERROR("REST client is not available");
    return nullptr;

#endif
}

NHttpTransportPtr createDefaultHttpTransport()
{
#ifdef BUILD_HTTP_CPPREST

    return NHttpTransportPtr(new NHttpClientCppRest());

#else

    NLOG_ERROR("Default HTTP transport is not available");
    return nullptr;

#endif
}

}
