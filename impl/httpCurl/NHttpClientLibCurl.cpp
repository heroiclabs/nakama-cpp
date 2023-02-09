#include <string>
#include <curl/curl.h>
#include <memory.h>
#include <nakama-cpp/log/NLogger.h>
#include <nakama-cpp/NHttpTransportInterface.h>
#include "NHttpClientLibCurl.h"
#include "nakama-cpp/log/NLogger.h"
#include "AndroidCA.h"

static int debug_callback(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp)
{
    const char *text;
    (void)handle; /* prevent compiler warning */
    (void)userp;

    switch (type) {
    case CURLINFO_TEXT:
        NLOG(Nakama::NLogLevel::Debug, "libcurl debug info <=> Text: %s", data);
    default: /* in case a new one is introduced to shock us */
        return 0;

    case CURLINFO_HEADER_OUT:
        text = "=> Send header";
        break;
    case CURLINFO_DATA_OUT:
        text = "=> Send data";
        break;
    case CURLINFO_SSL_DATA_OUT:
        text = "=> Send SSL data";
        break;
    case CURLINFO_HEADER_IN:
        text = "<= Recv header";
        break;
    case CURLINFO_DATA_IN:
        text = "<= Recv data";
        break;
    case CURLINFO_SSL_DATA_IN:
        text = "<= Recv SSL data";
        break;
    }

    NLOG(Nakama::NLogLevel::Debug, "libcurl debug info: %s", text);
    return 0;
}

static size_t write_callback(char* buffer, size_t size, size_t nmemb, void* user_ctx)
{
    Nakama::NHttpClientLibCurlContext* curl_ctx = (Nakama::NHttpClientLibCurlContext*) user_ctx;
    NLOG(Nakama::NLogLevel::Info, "received response... \n %s", buffer);

    if (buffer != NULL)
    {
        // char buffer from libcurl is not null terminated.
        curl_ctx->append_body(std::string(buffer, nmemb * size));
    }

    return nmemb * size;
}

namespace Nakama {

NHttpClientLibCurl::NHttpClientLibCurl(const NPlatformParameters& platformParameters) : _curl_multi(curl_multi_init(), curl_multi_cleanup)
{
}

void NHttpClientLibCurl::request(const NHttpRequest& req, const NHttpResponseCallback& callback) noexcept
{
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_easy(curl_easy_init(), curl_easy_cleanup);

    std::string uri = _base_uri + req.path + (req.queryArgs.empty() ? "" : "?");
    for (auto p: req.queryArgs) {
        uri += p.first + "=" + p.second + "&";
    };

    curl_slist* headers_list = NULL;

    for (auto it = req.headers.begin(); it != req.headers.end(); it++)
    {
        headers_list = curl_slist_append(headers_list, (it->first + ": " + it->second).c_str());
        if (headers_list == NULL)
        {
            NLOG(Nakama::NLogLevel::Error, "error writing header: %s", it->first.c_str());
            return;
        }
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

    curl_code = curl_easy_setopt(curl_easy.get(), CURLOPT_HTTPHEADER, headers_list);
    if (curl_code != CURLE_OK)
    {
        handle_curl_easy_set_opt_error("adding headers", curl_code, callback);
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

#if __ANDROID__
    CACertificateData data = Nakama::getCaCertificates();
    struct curl_blob blob;
    blob.data = reinterpret_cast<char*>(data.data.get());
    blob.len = data.len;
    blob.flags = CURL_BLOB_COPY;
    curl_easy_setopt(curl_easy.get(), CURLOPT_CAINFO_BLOB, &blob);
#endif

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

    /* ask libcurl to show us the verbose output */
    curl_code = curl_easy_setopt(curl_easy.get(), CURLOPT_VERBOSE, 1L);
    if (curl_code != CURLE_OK)
    {
        handle_curl_easy_set_opt_error("setting verbose output", curl_code, callback);
        return;
    }

    curl_code = curl_easy_setopt(curl_easy.get(), CURLOPT_DEBUGFUNCTION, debug_callback);
    if (curl_code != CURLE_OK)
    {
        handle_curl_easy_set_opt_error("adding debug function", curl_code, callback);
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

    const std::lock_guard lock(_contextsMutex);
    std::pair<std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>, std::unique_ptr<NHttpClientLibCurlContext>> pair(std::move(curl_easy), std::move(curl_ctx));
    _contexts.emplace_back(std::move(pair));
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
        NLOG(Nakama::NLogLevel::Error, "curl_multi_perform() failed, code %d.\n", (int)mc);
    }

    struct CURLMsg* m = nullptr;

    do
    {
        int msgq = 0;
        m = curl_multi_info_read(_curl_multi.get(), &msgq);
        if (m && (m->msg == CURLMSG_DONE)) {
            CURL* e = m->easy_handle;

            mc = curl_multi_remove_handle(_curl_multi.get(), e);
            if (mc)
            {
                NLOG(Nakama::NLogLevel::Error, "curl_multi_remove_handle() failed, code %d.\n", (int)mc);
            }

            std::unique_ptr<NHttpClientLibCurlContext> context = NULL;
            {
                const std::lock_guard lock(_contextsMutex);

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

                context = std::move(it->second);
                _contexts.remove(*it);
            }

            auto callback = context->get_callback();
            if (callback != NULL)
            {
                auto response = std::shared_ptr<NHttpResponse>(new NHttpResponse());
                response->body = context->get_body();

                if (m->data.result != CURLE_OK)
                {
                    NLOG(Nakama::NLogLevel::Error, "curl easy handle returned code: %d \n", (int) m->data.result);
                    response->statusCode = InternalStatusCodes::CONNECTION_ERROR;
                }
                else
                {
                    int response_code;
                    CURLcode curl_code = curl_easy_getinfo(e, CURLINFO_RESPONSE_CODE, &response_code);
                    response->statusCode = response_code;

                    if (curl_code != CURLE_OK)
                    {
                        NLOG(Nakama::NLogLevel::Error, "curl_easy_getinfo() failed when getting response code, code %d.\n", (int)curl_code);
                    }
                }

                callback(response);
            }
        }
    }
    while(m);
}

void NHttpClientLibCurl::cancelAllRequests()
{
    const std::lock_guard lock(_contextsMutex);
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
