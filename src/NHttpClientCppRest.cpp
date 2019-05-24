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

#ifdef BUILD_REST_CLIENT

#include "NHttpClientCppRest.h"
#include "CppRestUtils.h"

namespace Nakama {

using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features

NHttpClientCppRest::~NHttpClientCppRest()
{
    _client.reset();
}

void NHttpClientCppRest::setBaseUri(const std::string& uri)
{
    utility::string_t baseUri(FROM_STD_STR(uri));

    if (_client && _client->base_uri().to_string() == baseUri)
    {
        // the same
        return;
    }

    _client.reset(new http_client(baseUri));
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

    ctx->callback = callback;

    if (!_client)
    {
        finishReqWithError(ctx, InternalStatusCodes::NOT_INITIALIZED_ERROR, "[NHttpClientCppRest::request] base uri is not set");
        return;
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

    request.set_request_uri(builder.to_string());
    request.set_body(FROM_STD_STR(req.body));
    request.set_method(theMethod);

    for (auto p : req.headers)
    {
        request.headers().add(FROM_STD_STR(p.first), FROM_STD_STR(p.second));
    }

    auto task = _client->request(request);
    // Task-based continuation
    task.then([this, ctx](pplx::task<http_response> previousTask)
    {
        try
        {
            http_response response = previousTask.get();

            if (ctx->callback)
            {
                NHttpResponsePtr responsePtr(new NHttpResponse());

                responsePtr->statusCode = response.status_code();

                if (responsePtr->statusCode != 200)
                {
                    responsePtr->errorMessage = TO_STD_STR(response.reason_phrase());
                }

                responsePtr->body = response.extract_utf8string().get();

                finishReq(ctx, responsePtr);
            }
            else
            {
                removePendingReq(ctx);
            }
        }
        catch (const std::exception & e)
        {
            finishReqWithError(ctx, InternalStatusCodes::CONNECTION_ERROR, "[NHttpClientCppRest::request] exception: " + std::string(e.what()));
        }
    });
}

NHttpClientCppRest::ReqContext* NHttpClientCppRest::createReqContext()
{
    ReqContext* ctx = new ReqContext();

    std::lock_guard<std::mutex> guard(_mutex);
    _pendingRequests.emplace_back(ReqContextPtr(ctx));

    return ctx;
}

void NHttpClientCppRest::finishReq(ReqContext* ctx, NHttpResponsePtr response)
{
    std::lock_guard<std::mutex> guard(_mutex);

    ctx->response = std::move(response);

    for (auto it = _pendingRequests.begin(); it != _pendingRequests.end(); ++it)
    {
        if (it->get() == ctx)
        {
            _finishedRequests.emplace_back(std::move(*it));
            _pendingRequests.erase(it);
            break;
        }
    }
}

void NHttpClientCppRest::finishReqWithError(ReqContext* ctx, int statusCode, std::string&& reason)
{
    NHttpResponsePtr responsePtr(new NHttpResponse());

    responsePtr->statusCode = statusCode;
    responsePtr->errorMessage = std::move(reason);

    finishReq(ctx, responsePtr);
}

void NHttpClientCppRest::removePendingReq(ReqContext* ctx)
{
    std::lock_guard<std::mutex> guard(_mutex);

    for (auto it = _pendingRequests.begin(); it != _pendingRequests.end(); ++it)
    {
        if (it->get() == ctx)
        {
            _pendingRequests.erase(it);
            break;
        }
    }
}

NHttpClientCppRest::ReqContextPtr NHttpClientCppRest::popFinishedReq()
{
    std::lock_guard<std::mutex> guard(_mutex);

    if (_finishedRequests.empty())
        return nullptr;
    
    ReqContextPtr ctx = std::move(_finishedRequests.front());
    _finishedRequests.pop_front();

    return ctx;
}

}

#endif // BUILD_REST_CLIENT
