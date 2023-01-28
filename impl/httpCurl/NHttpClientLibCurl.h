/*
 * Copyright 2022 The Nakama Authors
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

#include <map>
#include <nakama-cpp/NHttpTransportInterface.h>
#include <nakama-cpp/NPlatformParams.h>
#include <curl/curl.h>
#include "NHttpClientLibCurlContext.h"

namespace Nakama {
    class NHttpClientLibCurl: public NHttpTransportInterface
    {
        public:
            NHttpClientLibCurl(const NPlatformParameters& platformParameters);
            ~NHttpClientLibCurl() noexcept;

            void setBaseUri(const std::string& uri) override;
            void tick() override;
            void request(const NHttpRequest& req, const NHttpResponseCallback& callback = nullptr) override;
            void cancelAllRequests() override;
        private:
            std::unique_ptr<CURLM, decltype(&curl_multi_cleanup)> _curl_multi;
            std::string _base_uri;
            std::map<CURL*, std::unique_ptr<NHttpClientLibCurlContext>> _curl_easys;
            void handle_curl_easy_set_opt_error(std::string action, CURLcode code, const NHttpResponseCallback& callback);
    };
}
