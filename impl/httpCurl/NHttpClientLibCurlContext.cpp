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

#include <string>
#include <curl/curl.h>
#include "nakama-cpp/NHttpTransportInterface.h"
#include "NHttpClientLibCurlContext.h"

namespace Nakama
{
    NHttpClientLibCurlContext::NHttpClientLibCurlContext(const NHttpResponseCallback callback, curl_slist* headers) : _callback(callback), _headers(headers)
    {
    }

    NHttpResponseCallback NHttpClientLibCurlContext::get_callback()
    {
        return _callback;
    }

    NHttpClientLibCurlContext::~NHttpClientLibCurlContext()
    {
        if (_headers != NULL)
        {
            curl_slist_free_all(_headers);
        }
    }

    curl_slist* NHttpClientLibCurlContext::get_headers()
    {
        return _headers;
    }

    std::string NHttpClientLibCurlContext::get_body()
    {
        return _body;
    }

    void NHttpClientLibCurlContext::append_body(std::string body)
    {
        _body += body;
    }
}
