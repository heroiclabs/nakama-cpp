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

#include "DefaultClient.h"
#include "nakama-cpp/StrUtil.h"
#include <grpc++/create_channel.h>
#include <sstream>

using namespace std;

namespace Nakama {

ClientPtr createDefaultClient(const DefaultClientParameters& parameters)
{
    ClientPtr client(new DefaultClient(parameters));
    return client;
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
    disconnect();
}

void DefaultClient::disconnect()
{
    _cq.Shutdown();
}

void DefaultClient::tick()
{
    bool ok;
    void* tag;
    bool continueLoop = true;
    gpr_timespec timespec;

    timespec.clock_type = GPR_TIMESPAN;
    timespec.tv_sec = 0;
    timespec.tv_nsec = 0;

    do {
        switch (_cq.AsyncNext(&tag, &ok, timespec))
        {
        case grpc::CompletionQueue::SHUTDOWN:
            std::cerr << "The completion queue unexpectedly shutdown." << std::endl;
            continueLoop = false;
            break;

        case grpc::CompletionQueue::GOT_EVENT:
            std::cout << "call completed " << (int)(tag) << " ok: " << ok << std::endl;
            onResponse(tag, ok);
            break;

        case grpc::CompletionQueue::TIMEOUT:
            continueLoop = false;
            break;
        }
    } while (continueLoop);
}

RpcRequest * DefaultClient::createRpcRequest(const NSession* session)
{
    RpcRequest* rpcRequest = new RpcRequest();

    if (session)
    {
        rpcRequest->context.AddMetadata("authorization", "Bearer " + session->token);
    }
    else
    {
        rpcRequest->context.AddMetadata("authorization", _basicAuthMetadata);
    }

    _requests.emplace(rpcRequest);
    return rpcRequest;
}

void DefaultClient::onResponse(void * tag, bool ok)
{
    auto it = _requests.find((RpcRequest*)tag);

    if (it != _requests.end())
    {
        RpcRequest* reqStatus = *it;

        if (ok)
        {
            if (reqStatus->status.ok())
            {
                if (reqStatus->successCallback)
                {
                    reqStatus->successCallback();
                }
            }
            else if (reqStatus->errorCallback)
            {
                std::stringstream ss;

                ss << "grpc call failed" << std::endl;
                ss << "code: "    << reqStatus->status.error_code() << std::endl;
                ss << "message: " << reqStatus->status.error_message() << std::endl;
                ss << "details: " << reqStatus->status.error_details();

                reqStatus->errorCallback(NError(
                    ss.str(),
                    ErrorCode::GrpcCallFailed
                ));
            }
        }
        else if (reqStatus->errorCallback)
        {
            reqStatus->errorCallback(NError(
                "grpc call failed",
                ErrorCode::GrpcCallFailed
            ));
        }

        delete reqStatus;
        _requests.erase(it);
    }
    else
    {
        std::cout << "DefaultClient::onResponse: not found tag " << tag << std::endl;
    }
}

void DefaultClient::authenticateDevice(
    const std::string& id,
    const std::string& username,
    bool create,
    std::function<void(const NSession&)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(nullptr);
    auto sessionData(make_shared<nakama::api::Session>());

    if (successCallback)
    {
        rpcRequest->successCallback = [sessionData, successCallback]()
        {
            NSession session;
            session.created = sessionData->created();
            session.token = sessionData->token();
            successCallback(session);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    nakama::api::AuthenticateDeviceRequest req;

    req.mutable_account()->set_id(id);

    if (!username.empty())
        req.set_username(username);

    req.mutable_create()->set_value(create);

    auto responseReader = _stub->AsyncAuthenticateDevice(&rpcRequest->context, req, &_cq);

    responseReader->Finish(&(*sessionData), &rpcRequest->status, (void*)rpcRequest);
}

void DefaultClient::getAccount(
    const NSession& session,
    std::function<void(const NAccount&)> successCallback,
    ErrorCallback errorCallback
)
{
    RpcRequest* rpcRequest = createRpcRequest(&session);
    auto accoutData(make_shared<nakama::api::Account>());

    if (successCallback)
    {
        rpcRequest->successCallback = [accoutData, successCallback]()
        {
            NAccount account;

            account.user.id = accoutData->user().id();
            account.custom_id = accoutData->custom_id();
            account.email = accoutData->email();
            //account.user = accoutData->user();
            account.verify_time = accoutData->verify_time().seconds() * 1000;
            account.wallet = accoutData->wallet();
            
            successCallback(account);
        };
    }
    rpcRequest->errorCallback = errorCallback;

    auto responseReader = _stub->AsyncGetAccount(&rpcRequest->context, google::protobuf::Empty(), &_cq);

    responseReader->Finish(&(*accoutData), &rpcRequest->status, (void*)rpcRequest);
}

}
