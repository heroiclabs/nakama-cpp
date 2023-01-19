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

#include <nakama-cpp/NHttpTransportInterface.h>
#include <nakama-cpp/NPlatformParams.h>
#include <curl/curl.h>

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
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> _curl;
        std::unique_ptr<CURLM, decltype(&curl_multi_cleanup)> _curlm;
        std::unique_ptr<CURLU, decltype(&curl_url_cleanup)> _curl_url;
    };
}
