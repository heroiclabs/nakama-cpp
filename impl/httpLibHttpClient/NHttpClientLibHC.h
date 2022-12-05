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

#include <httpClient.h>
#include <nakama-cpp/NHttpTransportInterface.h>
#include <nakama-cpp/NPlatformParams.h>
#include <atomic>

namespace Nakama {
    class NHttpClientLibHC: public NHttpTransportInterface
    {
    public:
        NHttpClientLibHC(const NPlatformParameters& platformParameters);
        ~NHttpClientLibHC() noexcept;

        void setBaseUri(const std::string& uri) override;

        void tick() override;

        void request(const NHttpRequest& req, const NHttpResponseCallback& callback = nullptr) override;

        void cancelAllRequests() override;
    private:
        void submit_cb(const NHttpResponseCallback &cb, int statusCode, std::string body, std::string err = "") noexcept;
        HRESULT prep_hc_call(const NHttpRequest& req, std::unique_ptr<HC_CALL, decltype(&HCHttpCallCloseHandle)>& call);

        std::unique_ptr<std::remove_pointer<XTaskQueueHandle>::type, decltype(&XTaskQueueCloseHandle)> m_queue;
        std::string m_baseUri;
        std::atomic<bool> m_queue_terminated;
    };
}
