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

#pragma once

#include <nakama-cpp/NHttpTransportInterface.h>
#include "nakama-cpp/NPlatformParams.h"
#include "cpprest/http_client.h"
#include <list>
#include <mutex>

namespace Nakama {

    class NHttpClientCppRestContext;

    /**
     * HTTP client using C++ REST SDK (https://github.com/microsoft/cpprestsdk)
     */
    class NHttpClientCppRest : public NHttpTransportInterface
    {
    public:
        using ReqId = uint64_t;

        NHttpClientCppRest(const NPlatformParameters& platformParams);
        ~NHttpClientCppRest();

        void setBaseUri(const std::string& uri) override;

        void tick() override;

        void request(const NHttpRequest& req, const NHttpResponseCallback& callback = nullptr) override;

        void cancelAllRequests() override;

    protected:
        friend class NHttpClientCppRestContext;

        struct ReqContext
        {
            ReqContext(ReqId _id) : id(_id) {}

            ReqId id;
            NHttpResponseCallback callback;
            NHttpResponsePtr response;
        };

        using ReqContextPtr = std::shared_ptr<ReqContext>;

        std::shared_ptr<ReqContext> createReqContext();
        void finishReq(ReqId id, NHttpResponsePtr response);
        void finishReqWithError(ReqId id, int statusCode, std::string&& reason);
        void removePendingReq(ReqId id);
        ReqContextPtr popFinishedReq();

    protected:
        std::shared_ptr<NHttpClientCppRestContext> _context;
        std::unique_ptr<web::http::client::http_client> _client;
        std::list<ReqContextPtr> _pendingRequests;
        std::list<ReqContextPtr> _finishedRequests;
        utility::string_t _baseUri;
    };

}
