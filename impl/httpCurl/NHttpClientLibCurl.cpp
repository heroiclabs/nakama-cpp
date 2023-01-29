#include <string.h>
#include <curl/curl.h>
#include <memory.h>
#include <nakama-cpp/log/NLogger.h>
#include <nakama-cpp/NHttpTransportInterface.h>
#include "NHttpClientLibCurl.h"
#include "log/NLogger.h"

static size_t write_callback(char* buffer, size_t size, size_t nmemb, void* user_ctx)
{
    Nakama::NHttpClientLibCurlContext* curl_ctx = (Nakama::NHttpClientLibCurlContext*) user_ctx;
    NLOG(Nakama::NLogLevel::Info, "received response... \n %s \n", buffer);

    if (buffer != NULL)
    {
        curl_ctx->set_body(std::string(buffer));
    }

    return nmemb * size;
}

namespace Nakama {

NHttpClientLibCurl::NHttpClientLibCurl(const NPlatformParameters& platformParameters) : _curl_multi(curl_multi_init(), curl_multi_cleanup)
{
}

void NHttpClientLibCurl::request(const NHttpRequest& req, const NHttpResponseCallback& callback = nullptr) noexcept
{
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_easy(curl_easy_init(), curl_easy_cleanup);

    std::string uri = _base_uri + req.path + (req.queryArgs.empty() ? "" : "?");
    for (auto p: req.queryArgs) {
        uri += p.first + "=" + p.second + "&";
    };

    curl_slist* headers_list = NULL;

    for (auto it = req.headers.begin(); it != req.headers.end(); it++)
    {
        headers_list = curl_slist_append(headers_list, it->first.c_str());
        headers_list = curl_slist_append(headers_list, it->second.c_str());
    }

    const char *callMethod = nullptr;
    switch (req.method) {
        case NHttpReqMethod::POST:
            callMethod = "POST";
            break;
        case NHttpReqMethod::GET:
            callMethod = "GET";
            break;
        case NHttpReqMethod::PUT:
            callMethod = "PUT";
            break;
        case NHttpReqMethod::DEL:
            callMethod = "DELETE";
            break;
    }

    CURLcode curl_code = curl_easy_setopt(curl_easy.get(), CURLOPT_URL, uri.c_str());
    if (curl_code != CURLE_OK)
    {
        handle_curl_easy_set_opt_error("setting url", curl_code, callback);
        return;
    }

    if (req.body != "")
    {
        curl_code = curl_easy_setopt(curl_easy.get(), CURLOPT_COPYPOSTFIELDS, req.body.c_str());
        if (curl_code != CURLE_OK)
        {
            handle_curl_easy_set_opt_error("copying post fields", curl_code, callback);
            return;
        }
    }

    curl_code = curl_easy_setopt(curl_easy.get(), CURLOPT_CUSTOMREQUEST, callMethod);
    if (curl_code != CURLE_OK)
    {
        handle_curl_easy_set_opt_error("adding call method", curl_code, callback);
        return;
    }

    curl_code = curl_easy_setopt(curl_easy.get(), CURLOPT_WRITEFUNCTION, write_callback);
    if (curl_code != CURLE_OK)
    {
        handle_curl_easy_set_opt_error("adding write function", curl_code, callback);
        return;
    }

    std::unique_ptr<NHttpClientLibCurlContext> curl_ctx(new NHttpClientLibCurlContext(callback, headers_list));
    curl_code = curl_easy_setopt(curl_easy.get(), CURLOPT_WRITEDATA, curl_ctx.get());
    if (curl_code != CURLE_OK)
    {
        handle_curl_easy_set_opt_error("adding curl context", curl_code, callback);
        return;
    }

    CURLMcode curl_multi_code = curl_multi_add_handle(_curl_multi.get(), curl_easy.get());
    if (curl_multi_code != CURLM_OK)
    {
        auto response = std::shared_ptr<NHttpResponse>(new NHttpResponse());
        response->errorMessage = "curl_multi_add_handle() error while adding handle, code: " + std::string(curl_multi_strerror(curl_multi_code));
        response->statusCode = -1;
        callback(response);
        return;
    };


    std::lock_guard(_contextsMutex);
    _contexts.emplace(std::pair(curl_easy, curl_ctx));
}

void NHttpClientLibCurl::setBaseUri(const std::string& uri)
{
    _base_uri = uri;
}

void NHttpClientLibCurl::tick()
{
    int running_handles = 0;
    CURLMcode mc = curl_multi_perform(_curl_multi.get(), &running_handles);
    if (mc)
    {
        // log but do not return -- still see if any pending messages
        NLOG(Nakama::NLogLevel::Error, "curl_multi_poll() failed, code %d.\n", (int)mc);
    }

    struct CURLMsg* m = nullptr;

    do
    {
        int msgq = 0;
        m = curl_multi_info_read(_curl_multi.get(), &msgq);
        if (m && (m->msg == CURLMSG_DONE)) {
            if (m->data.result != CURLE_OK) {
                NLOG(Nakama::NLogLevel::Error, "curl_multi_info_read() error: %s", curl_easy_strerror(m->data.result));
                continue;
            }

            CURL* e = m->easy_handle;

            mc = curl_multi_remove_handle(_curl_multi.get(), e);
            if (mc)
            {
                NLOG(Nakama::NLogLevel::Error, "curl_multi_remove_handle() failed, code %d.\n", (int)mc);
            }

            std::lock_guard(_contextsMutex);

            auto it = _contexts.begin();
            while (it != _contexts.end())
            {
                if (it->first.get() == e)
                {
                    break;
                }

                it++;
            }

            if (it == _contexts.end())
            {
                NLOG(Nakama::NLogLevel::Error, "curl_multi_info_read() error: untracked easy handle.");
                continue;
            }

            if (it->second->get_callback() != NULL)
            {
                auto response = std::shared_ptr<NHttpResponse>(new NHttpResponse());
                response->body = it->second->get_body();

                int response_code;
                CURLcode curl_code = curl_easy_getinfo(e, CURLINFO_RESPONSE_CODE, &response_code);
                response->statusCode = response_code;

                if (curl_code != CURLE_OK)
                {
                    NLOG(Nakama::NLogLevel::Error, "curl_easy_getinfo() failed when getting response code, code %d.\n", (int)curl_code);
                }

                it->second->get_callback()(response);
            }

            _contexts.remove(*it);
        }
    }
    while(m);
}

void NHttpClientLibCurl::cancelAllRequests()
{
    std::lock_guard(_contextsMutex);
    for (auto it = _contexts.begin(); it != _contexts.end(); it++)
    {
        NHttpResponsePtr responsePtr(new NHttpResponse());

        responsePtr->statusCode = InternalStatusCodes::CANCELLED_BY_USER;
        responsePtr->errorMessage = "cancelled by user";

        it->second->get_callback()(responsePtr);
        CURLMcode mc = curl_multi_remove_handle(_curl_multi.get(), it->first.get());
        if (mc != CURLM_OK)
        {
            NLOG(Nakama::NLogLevel::Error, "curl_multi_remove_handle() failed, code %d.\n", (int)mc);
        }
    }

    _contexts.clear();
}

void NHttpClientLibCurl::handle_curl_easy_set_opt_error(std::string action, CURLcode code, const NHttpResponseCallback& callback)
{
    auto response = std::shared_ptr<NHttpResponse>(new NHttpResponse());
    response->errorMessage = "curl_easy_getinfo() error while : " + action + ", code: " + std::string(curl_easy_strerror(code));
    response->statusCode = -1;
    callback(response);
}

};
