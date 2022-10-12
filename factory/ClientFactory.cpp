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
    #include "../core/core-grpc/GrpcClient.h"
#endif

#ifdef BUILD_REST_CLIENT
    #include "../core/core-rest/RestClient.h"
#endif

#ifdef BUILD_HTTP_LIBHTTPCLIENT
#include "../../impl/httpLibHttpClient/NHttpClientLibHC.h"
#endif

#ifdef BUILD_HTTP_SONY
#include "httpSony.h"
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

#ifdef BUILD_GRPC_CLIENT
NClientPtr createGrpcClient(const NClientParameters& parameters)
{
    NClientPtr client(new GrpcClient(parameters));
    return client;
}
#endif

#ifdef BUILD_REST_CLIENT
NClientPtr createRestClient(const NClientParameters& parameters, NHttpTransportPtr httpTransport)
{

    if (!httpTransport)
    {
        httpTransport = createDefaultHttpTransport(parameters.platformParams);

        if (!httpTransport)
        {
            NLOG_ERROR("Error creating HTTP transport");
            return nullptr;
        }
    }

    NClientPtr client(new RestClient(parameters, httpTransport));
    return client;
}
#endif

#ifdef BUILD_REST_CLIENT
NHttpTransportPtr createDefaultHttpTransport(const NPlatformParameters& platformParams)
{
    (void)platformParams;  // silence unused variable warning on some platforms
    // Compilation error if no implementation is selected
#if defined(BUILD_HTTP_LIBHTTPCLIENT)
    return NHttpTransportPtr(new NHttpClientLibHC(platformParams));
#elif defined(BUILD_HTTP_SONY)
    return NHttpTransportPtr(NHttpClientSony::New());
#elif defined(BUILD_HTTP_IS_BLANK)
    NLOG_ERROR("No default transport included, users must provide their own explicitly");
    return nullptr;
#else
#error "New impl is not listed here, fix it"
#endif
}
#endif //BUILD_REST_CLIENT

}
