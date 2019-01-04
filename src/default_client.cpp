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

#include "nakama-cpp/default_client.h"
#include "default_client.h"
#include <grpc++/create_channel.h>
#include "nakama-cpp/strutil.h"

namespace nakama {

ClientInterface* createDefaultClient(const DefaultClientParameters& parameters)
{
    return new DefaultClient(parameters);
}

DefaultClient::DefaultClient(const DefaultClientParameters& parameters)
{
    std::string target = parameters.host + ":" + std::to_string(parameters.port);

    auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());

    _stub = nakama::api::Nakama::NewStub(channel);

    _basicAuthMetadata = "Basic " + base64_encode(parameters.serverKey + ":");
}

DefaultClient::~DefaultClient()
{
}

void DefaultClient::authenticateDevice(
    const std::string& id,
    const std::string& username,
    bool create
)
{
    grpc::ClientContext context;

    context.AddMetadata("authorization", _basicAuthMetadata);

    nakama::api::AuthenticateDeviceRequest req;
    nakama::api::Session session;

    req.mutable_account()->set_id(id);

    if (!username.empty())
        req.set_username(username);

    req.mutable_create()->set_value(create);

    auto status = _stub->AuthenticateDevice(&context, req, &session);
}

}
