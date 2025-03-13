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
#include <atomic>
#include <mutex>
#include <list>
#include <nakama-cpp/NHttpTransportInterface.h>
#include <nakama-cpp/NPlatformParams.h>
#include <curl/curl.h>
#include "NHttpClientLibCurlContext.h"
#if __ANDROID__
#include <jni.h>
#endif

namespace Nakama {
    class NHttpClientLibCurl: public NHttpTransportInterface
    {
        public:
            NHttpClientLibCurl(const NPlatformParameters& platformParameters);

            void setBaseUri(const std::string& uri) override;
            void setTimeout(int seconds) override;
            void tick() override;
            void request(const NHttpRequest& req, const NHttpResponseCallback& callback = nullptr) noexcept override;
            void cancelAllRequests() override;
        private:
            std::unique_ptr<CURLM, decltype(&curl_multi_cleanup)> _curl_multi;
            std::string _base_uri;
            int _timeout = -1;
            // TODO implement curl_easy reuse.
            // TODO would be more performant but less safe as a map with CURL* as key
            std::list<std::pair<std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>, std::unique_ptr<NHttpClientLibCurlContext>>> _contexts;
            void handle_curl_easy_set_opt_error(std::string action, CURLcode code, const NHttpResponseCallback& callback);
            std::mutex _mutex;
    };
}
