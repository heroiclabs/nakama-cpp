#pragma once

#include <string>
#include <curl/curl.h>
#include <nakama-cpp/log/NLogger.h>
#include "NHttpTransportInterface.h"
#include "NHttpClientLibCurl.h"
#include "NHttpClientLibCurlContext.h"

static size_t header_callback(char* buffer, size_t size, size_t nitems, void* user_ctx)
{
    Nakama::NHttpClientLibCurlContext* curl_ctx = (Nakama::NHttpClientLibCurlContext*) user_ctx;

    char* saveptr = NULL;
    char* first_token = strtok_r(buffer, " ", &saveptr);

    if (!strcmp(first_token, "HTTP/1.1") || !strcmp(first_token, "HTTP/2"))
    {
        // look for http response code
        char* code = strtok_r(NULL, " ", &saveptr);
        if (code == NULL)
        {
            curl_ctx->http_response_code = -1;
            curl_ctx->error_message = "Could not find HTTP response code";
            curl_ctx->invokeCallback();
            free(curl_ctx);
        }
        else
        {
            curl_ctx->http_response_code = atoi(code);
        }
    }

    return size * nitems;
}

static size_t write_callback(char* buffer, size_t size, size_t nmemb, void* user_ctx)
{
    NLOG(Nakama::NLogLevel::Info, "curl received response... \n %s \n", buffer);
    Nakama::NHttpClientLibCurlContext* curl_ctx = (Nakama::NHttpClientLibCurlContext*) user_ctx;
    curl_ctx->body = buffer;
    curl_ctx->invokeCallback();
    free(curl_ctx); // todo is this right place to free
    return nmemb * size;
}

namespace Nakama {

NHttpClientLibCurl::NHttpClientLibCurl(const NPlatformParameters& platformParameters) :
{
    _curl_multi = std::unique_ptr<CURLM, decltype(&curl_multi_cleanup)>(curl_multi_init(), curl_multi_cleanup);
}

void NHttpClientLibCurl::request(const NHttpRequest& req, const NHttpResponseCallback& callback = nullptr) noexcept
{
    CURL* curl_easy = curl_easy_init();

    std::string uri = _base_uri + req.path + (req.queryArgs.empty() ? "" : "?");
    for (auto p: req.queryArgs) {
        uri += p.first + "=" + p.second + "&";
    };

    curl_easy_setopt(curl_easy, CURLOPT_URL, uri.c_str());

    struct curl_slist* headers_list = NULL;

    for (auto it = req.headers.begin(); it != req.headers.end(); it++)
    {
        headers_list = curl_slist_append(headers_list, it->first.c_str());
        headers_list = curl_slist_append(headers_list, it->second.c_str());
    }

    curl_easy_setopt(curl_easy, CURLOPT_HTTPHEADER, headers_list);

    // TODO: curl_slist_free_all

    if (req.body != "")
    {
        curl_easy_setopt(curl_easy, CURLOPT_COPYPOSTFIELDS, req.body.c_str());
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

    // TODO free.
    Nakama::NHttpClientLibCurlContext* curl_ctx = new Nakama::NHttpClientLibCurlContext(callback);

    curl_easy_setopt(curl_easy, CURLOPT_CUSTOMREQUEST, callMethod);

    curl_easy_setopt(curl_easy, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl_easy, CURLOPT_HEADERDATA, curl_ctx);

    curl_easy_setopt(curl_easy, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_easy, CURLOPT_WRITEDATA, curl_ctx);

    curl_multi_add_handle(_curl_multi.get(), curl_easy);
}

void NHttpClientLibCurl::setBaseUri(const std::string& uri)
{
    _base_uri = base_uri;
}

void NHttpClientLibCurl::tick()
{
    int running_handles = 0;
    CURLMcode mc = curl_multi_perform(curl->multi_handle, &running_handles);
    if (mc)
    {
        NLOG(Nakama::NLogLevel::Error, "curl_multi_poll() failed, code %d.\n", (int)mc);
    }
}

void NHttpClientLibCurl::cancelAllRequests()
{
    NHttpResponsePtr responsePtr(new NHttpResponse());
    responsePtr->statusCode = InternalStatusCodes::CANCELLED_BY_USER;
    responsePtr->errorMessage = "cancelled by user";
    ctx->callback(responsePtr);
}

};
}
