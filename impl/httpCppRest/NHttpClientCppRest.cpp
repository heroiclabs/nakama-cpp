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

#include "NHttpClientCppRest.h"
#include "CppRestUtils.h"

namespace Nakama {

using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features

class NHttpClientCppRestContext
{
public:
    NHttpClientCppRestContext(NHttpClientCppRest& c) : client(c) {}

    void finishReq(NHttpClientCppRest::ReqId id, NHttpResponsePtr response)
    {
        std::lock_guard<std::mutex> guard(mutex);
        if (alive)
            client.finishReq(id, response);
    }

    void finishReqWithError(NHttpClientCppRest::ReqId id, int statusCode, std::string&& reason)
    {
        std::lock_guard<std::mutex> guard(mutex);
        if (alive)
            client.finishReqWithError(id, statusCode, std::move(reason));
    }

    void removePendingReq(NHttpClientCppRest::ReqId id)
    {
        std::lock_guard<std::mutex> guard(mutex);
        if (alive)
            client.removePendingReq(id);
    }

    NHttpClientCppRest& client;
    NHttpClientCppRest::ReqId nextReqId = 1;
    std::mutex mutex;
    bool alive = true;
};

///////////////////////////////////////////////////////////////
NHttpClientCppRest::NHttpClientCppRest()
    : _context(std::make_shared<NHttpClientCppRestContext>(*this))
{
}

NHttpClientCppRest::~NHttpClientCppRest()
{
    {
        std::lock_guard<std::mutex> guard(_context->mutex);
        _context->alive = false;
    }

    _client.reset();
}

void NHttpClientCppRest::setBaseUri(const std::string& uri)
{
    _baseUri = FROM_STD_STR(uri);

    if (_client && _client->base_uri().to_string() == _baseUri)
    {
        // the same
        return;
    }

    _client.reset();
}

void NHttpClientCppRest::tick()
{
    ReqContextPtr ctx;

    while ((ctx = popFinishedReq()))
    {
        if (ctx->callback)
            ctx->callback(ctx->response);
    }
}

void NHttpClientCppRest::request(const NHttpRequest& req, const NHttpResponseCallback& callback)
{
    ReqContext* ctx = createReqContext();
    ReqId reqId = ctx->id;
    bool hasCallback = !!callback;

    ctx->callback = callback;

    if (!_client)
    {
        if (_baseUri.empty())
        {
            _context->finishReqWithError(reqId, InternalStatusCodes::NOT_INITIALIZED_ERROR, "[NHttpClientCppRest::request] base uri is not set");
            return;
        }

        _client.reset(new http_client(_baseUri));
    }

    // Build request URI and start the request.
    uri_builder builder(FROM_STD_STR(req.path));

    for (auto p : req.queryArgs)
    {
        builder.append_query(FROM_STD_STR(p.first), FROM_STD_STR(p.second));
    }

    web::http::method theMethod;

    switch (req.method)
    {
    case NHttpReqMethod::POST: theMethod = methods::POST; break;
    case NHttpReqMethod::GET : theMethod = methods::GET; break;
    case NHttpReqMethod::PUT : theMethod = methods::PUT; break;
    case NHttpReqMethod::DEL : theMethod = methods::DEL; break;
    }

    http_request request;

    for (auto p : req.headers)
    {
        request.headers().add(FROM_STD_STR(p.first), FROM_STD_STR(p.second));
    }

    request.set_request_uri(builder.to_string());
    request.set_body(FROM_STD_STR(req.body));
    request.set_method(theMethod);

    std::weak_ptr<NHttpClientCppRestContext> context_wptr(_context);

    auto task = _client->request(request);
    // Task-based continuation
    (void) task.then([context_wptr, reqId, hasCallback](pplx::task<http_response> previousTask)
    {
        try
        {
            http_response response = previousTask.get();

            if (hasCallback)
            {
                NHttpResponsePtr responsePtr(new NHttpResponse());

                responsePtr->statusCode = response.status_code();

                if (responsePtr->statusCode != 200)
                {
                    responsePtr->errorMessage = TO_STD_STR(response.reason_phrase());
                }

                responsePtr->body = response.extract_utf8string().get();

                auto context = context_wptr.lock();
                if (context)
                    context->finishReq(reqId, responsePtr);
            }
            else
            {
                auto context = context_wptr.lock();
                if (context)
                    context->removePendingReq(reqId);
            }
        }
        catch (const std::exception & e)
        {
            auto context = context_wptr.lock();
            if (context)
                context->finishReqWithError(reqId, InternalStatusCodes::CONNECTION_ERROR, "[NHttpClientCppRest::request] exception: " + std::string(e.what()));
        }
    });
}

void NHttpClientCppRest::cancelAllRequests()
{
    std::lock_guard<std::mutex> guard(_context->mutex);

    _client.reset();

    while (!_pendingRequests.empty())
    {
        auto& ctx = _pendingRequests.front();
        if (ctx->callback)
        {
            NHttpResponsePtr responsePtr(new NHttpResponse());

            responsePtr->statusCode = InternalStatusCodes::CANCELLED_BY_USER;
            responsePtr->errorMessage = "cancelled by user";

            ctx->callback(responsePtr);
        }

        _pendingRequests.pop_front();
    }
}

NHttpClientCppRest::ReqContext* NHttpClientCppRest::createReqContext()
{
    ReqContext* ctx = new ReqContext(_context->nextReqId++);

    std::lock_guard<std::mutex> guard(_context->mutex);
    _pendingRequests.emplace_back(ReqContextPtr(ctx));

    return ctx;
}

void NHttpClientCppRest::finishReq(ReqId id, NHttpResponsePtr response)
{
    for (auto it = _pendingRequests.begin(); it != _pendingRequests.end(); ++it)
    {
        if (it->get()->id == id)
        {
            it->get()->response = std::move(response);
            _finishedRequests.emplace_back(std::move(*it));
            _pendingRequests.erase(it);
            break;
        }
    }
}

void NHttpClientCppRest::finishReqWithError(ReqId id, int statusCode, std::string&& reason)
{
    NHttpResponsePtr responsePtr(new NHttpResponse());

    responsePtr->statusCode = statusCode;
    responsePtr->errorMessage = std::move(reason);

    finishReq(id, responsePtr);
}

void NHttpClientCppRest::removePendingReq(ReqId id)
{
    for (auto it = _pendingRequests.begin(); it != _pendingRequests.end(); ++it)
    {
        if (it->get()->id == id)
        {
            _pendingRequests.erase(it);
            break;
        }
    }
}

NHttpClientCppRest::ReqContextPtr NHttpClientCppRest::popFinishedReq()
{
    std::lock_guard<std::mutex> guard(_context->mutex);

    if (_finishedRequests.empty())
        return nullptr;
    
    ReqContextPtr ctx = std::move(_finishedRequests.front());
    _finishedRequests.pop_front();

    return ctx;
}

}
