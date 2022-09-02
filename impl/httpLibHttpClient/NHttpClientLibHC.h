//
// Created by yamlcoder on 17/03/2022.
//

#pragma once

#include <httpClient/httpClient.h>
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
