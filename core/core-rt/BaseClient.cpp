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

#include <future>
#include "BaseClient.h"
#include "NRtClient.h"
#include "nakama-cpp/realtime/NWebsocketsFactory.h"
#include "nakama-cpp/log/NLogger.h"

#undef NMODULE_NAME
#define NMODULE_NAME "Nakama::BaseClient"

using namespace std;

namespace Nakama {

#if defined(BUILD_WEBSOCKET_WSLAY) && defined(BUILD_CURL_IO)
NRtClientPtr BaseClient::createRtClient()
{
    return createRtClient(createDefaultWebsocket(_platformParams));
}
#endif

NRtClientPtr BaseClient::createRtClient(NRtTransportPtr transport)
{
    RtClientParameters parameters;
    parameters.host = _host;
    parameters.port = _port;
    parameters.ssl  = _ssl;
    parameters.platformParams = _platformParams;

    if (!transport)
    {
        NLOG_ERROR("No websockets transport passed. Please set transport.");
        return nullptr;
    }

    NRtClientPtr client(new NRtClient(transport, parameters.host, parameters.port, parameters.ssl));
    return client;
}

#if NAKAMA_FUTURES

std::future<NFriendListPtr> RestClient::listFriendsAsync(
    NSessionPtr session,
    const opt::optional<int32_t>& limit,
    const opt::optional<NFriend::State>& state,
    const std::string& cursor
) {
    std::promise<NFriendListPtr> promise;

    listFriends(session, limit, state, cursor,
        [&](NFriendListPtr friendList) {
            promise.set_value(friendList);
        },
        [&](const NError& error) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(error.getMessage())));
        });

    return promise.get_future();
}

#endif

}
