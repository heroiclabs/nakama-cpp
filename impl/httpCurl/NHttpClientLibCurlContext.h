/*
 * Copyright 2023 The Nakama Authors
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

#include <string>
#include <curl/curl.h>
#include "NHttpTransportInterface.h"

namespace Nakama
{

class NHttpClientLibCurlContext
{
    public:
        NHttpClientLibCurlContext(const NHttpResponseCallback callback, const curl_slist* headers);
        ~NHttpClientLibCurlContext();

        NHttpResponseCallback get_callback();
        curl_slist* get_headers();
        std::string get_body();
        std::string set_body(const std::string body);

    private:
        NHttpResponseCallback _callback;
        curl_slist* _headers;
        std::string _body;
};

}
