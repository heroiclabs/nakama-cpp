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

#include "nakama-cpp/realtime/NWebsocketsFactory.h"
#include "nakama-cpp/log/NLogger.h"
#include "nakama-cpp/NPlatformParams.h"
// Private builds provide their own factory
#if !defined(WITH_PRIVATE_WS)

#if defined(WITH_WS_LIBHTTPC)
#include "../../impl/wsLibHttpClient/NWebsocketLibHC.h"
#elif defined(WITH_WS_WSLAY)
#include "NWebsocketWslay.h"
    #if defined(CFG_WSLAY_CURL_IO)
#include "WslayIOCurl.h"
    #endif
#elif defined(WITH_WS_CPPREST)
#include "NWebsocketCppRest.h"
#endif

namespace Nakama {

NRtTransportPtr createDefaultWebsocket([[maybe_unused]] const NPlatformParameters& platformParams)
{
    #if defined(WITH_WS_LIBHTTPC)
    return NRtTransportPtr(NWebsocketLibHC::New(platformParams));
    #elif defined(WITH_WS_WSLAY) && defined(CFG_WSLAY_CURL_IO)
    return NRtTransportPtr(new NWebsocketWslay(std::move(std::unique_ptr<WslayIOInterface>(new WslayIOCurl()))));
    #elif defined(WITH_WS_CPPREST)
    return NRtTransportPtr(new NWebsocketCppRest());
    #else
        #error Could not find default web socket transport or IO for platform.
    #endif
}
} // namespace Nakama

#endif
