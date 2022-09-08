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

#if defined(BUILD_WEBSOCKET_LIBHTTPCLIENT)
#include "../../impl/wsLibHttpClient/NWebsocketLibHC.h"
#elif defined(BUILD_WEBSOCKET_WSLAY)
#include "NWebsocketWslay.h"
#endif

#if defined(WSLAY_NETIO_CURL)
#include "NetIOCurl.h"
#define WSLAY_NETIO_TYPE NetIOCurl
#elif defined(WSLAY_NETIO_SONY)
#include "NetIOSony.h"
#define WSLAY_NETIO_TYPE NetIOSony
#endif

namespace Nakama {

NRtTransportPtr createDefaultWebsocket(const NPlatformParameters& platformParams)
{
    (void)platformParams;  // silence unused variable warning on some platforms
#if defined(BUILD_WEBSOCKET_LIBHTTPCLIENT)
    return NRtTransportPtr(NWebsocketLibHC::New(platformParams));
#elif defined(BUILD_WEBSOCKET_WSLAY)
#if !defined(WSLAY_NETIO_TYPE)
#error wslay websocket layer need to be parameterized with WSLAY_NETIO_TYPE. Define WSLAY_NETIO_$IMPL in the CMake.
#endif
    return NRtTransportPtr(new NWebsocketWslay<WSLAY_NETIO_TYPE>());
#elif defined(BUILD_WEBSOCKET_IS_BLANK)
    NLOG_ERROR("No builtin Websocket transport available. Provide one explicitly.");
    return nullptr;
#endif
}
} // namespace Nakama
