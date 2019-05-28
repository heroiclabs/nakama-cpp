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

#ifdef BUILD_WEBSOCKET_CPPREST
    #include "NWebsocketCppRest.h"
#endif

namespace Nakama {

NRtTransportPtr createDefaultWebsocket()
{
    NRtTransportPtr transport;
    
#if defined(BUILD_WEBSOCKET_CPPREST)
    transport = createWebsocketCppRest();
    
#else
    NLOG_ERROR("No default websocket available for this platform.");
#endif
    
    return transport;
}

NRtTransportPtr createWebsocketCppRest()
{
    NRtTransportPtr transport;
    
#ifdef BUILD_WEBSOCKET_CPPREST
    transport.reset(new NWebsocketCppRest());
#else
    NLOG_ERROR("WebsocketCppRest is not available for this platform.");
#endif
    
    return transport;
}

} // namespace Nakama
