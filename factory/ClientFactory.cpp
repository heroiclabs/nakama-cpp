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
#include "nakama-cpp/NClientInterface.h"

#ifdef WITH_GRPC_CLIENT
    #include "../core/core-grpc/GrpcClient.h"
#endif

#include "../core/core-rest/RestClient.h"

#ifdef WITH_HTTP_LIBHTTPC
#include "../../impl/httpLibHttpClient/NHttpClientLibHC.h"
#elif defined(WITH_HTTP_CPPREST)
#include "../../impl/httpCppRest/NHttpClientCppRest.h"
#elif defined(WITH_HTTP_CURL)
#include "../../impl/httpCurl/NHttpClientLibCurl.h"
#endif

namespace Nakama {

#if defined(HAVE_DEFAULT_TRANSPORT_FACTORY) || defined(WITH_GRPC_CLIENT)
NClientPtr createDefaultClient(const NClientParameters& parameters)
{
    #if defined(WITH_GRPC_CLIENT)
    return createGrpcClient(parameters);
    #else
    return createRestClient(parameters, createDefaultHttpTransport(parameters.platformParams));
    #endif
}
#endif

#ifdef WITH_GRPC_CLIENT

NClientPtr createGrpcClient(const NClientParameters& parameters)
{
    NClientPtr client(new GrpcClient(parameters));
    return client;
}

#endif

NClientPtr createRestClient(const NClientParameters& parameters, NHttpTransportPtr httpTransport)
{
    if (!httpTransport)
    {
        NLOG_ERROR("HTTP transport cannot be null.");
        return nullptr;
    }

    NClientPtr client(new RestClient(parameters, httpTransport));
    return client;
}

#ifndef WITH_PRIVATE_HTTP
NHttpTransportPtr createDefaultHttpTransport(const NPlatformParameters& platformParams)
{
    (void)platformParams;  // silence unused variable warning on some platforms
    // Compilation error if no implementation is selected
    #if defined(WITH_HTTP_LIBHTTPC)
    return NHttpTransportPtr(new NHttpClientLibHC(platformParams));
    #elif defined(WITH_HTTP_CPPREST)
    return NHttpTransportPtr(new NHttpClientCppRest(platformParams));
    #elif defined(WITH_HTTP_CURL)
    return NHttpTransportPtr(new NHttpClientLibCurl(platformParams));
    #else
        #error Could not find default http transport for platform.
    #endif
}
#endif //WITH_EXTRNAL_HTTP

}
