#pragma once

#include <string>
#include <curl/curl.h>
#include <nakama-cpp/log/NLogger.h>
#include "NHttpTransportInterface.h"
#include "NHttpClientLibCurl.h"

NAKAMA_NAMESPACE_BEGIN

NHttpClientLibCurl::NHttpClientLibCurl(const NPlatformParameters& platformParameters) :
        _curl(nullptr, curl_easy_cleanup),
        _curlm(curl_multi_init(), curl_multi_cleanup),
        _curl_url(nullptr, curl_url_cleanup)
{
}
    // should establish SSL-aware connection
void NHttpClientLibCurl::request(const NHttpRequest& req, const NHttpResponseCallback& callback = nullptr) noexcept {
    CURL* curl_easy = curl_easy_init();
    curl_easy_setopt(curl_easy, CURLOPT_URL, uri);

    struct curl_slist* headers_list = nk_map_to_curl_headers(req.headers);
    headers_list = curl_slist_append(headers_list, "Accept: application/json");

    curl_easy_setopt(curl_easy, CURLOPT_HTTPHEADER, headers_list);

    fprintf(stderr, "uri is... \n %s \n", uri);

    if (body != NULL)
    {
        char* content = json_dumps(body, 0);
        fprintf(stderr, "sending body... \n %s \n", content);
        curl_easy_setopt(curl_easy, CURLOPT_POSTFIELDS, content);
    }

    curl_easy_setopt(curl_easy, CURLOPT_CUSTOMREQUEST, method);

    curl_easy_setopt(curl_easy, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl_easy, CURLOPT_HEADERDATA, curl_ctx);

    curl_easy_setopt(curl_easy, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_easy, CURLOPT_WRITEDATA, curl_ctx);
    curl_easy_setopt(curl_easy, CURLOPT_TIMEOUT, timeout_sec);

    curl_multi_add_handle(curl->multi_handle, curl_easy);


}
    }

    NetIOAsyncResult connect_tick() {
        int running_handles;

        if (CURLMcode res = curl_multi_perform(_curlm.get(), &running_handles); res != CURLM_OK) {
            NLOG(Nakama::NLogLevel::Error, "libcurl error: %s", curl_multi_strerror(res));
            return NetIOAsyncResult::ERROR;
        }

        // done connecting (or failed) , either way take easy_handle out of multi
        if (running_handles == 0) {
            int msgq;

            struct CURLMsg* m = curl_multi_info_read(_curlm.get(), &msgq);
            if (!(m && m->msg == CURLMSG_DONE)) {
                NLOG(Nakama::NLogLevel::Error, "unexpected libcurl error: curl_multi_info_read had no completed messages");
                return NetIOAsyncResult::ERROR;
            }

            assert(msgq == 0);  // there should be at most 1 easy handle in multi handle
            assert(m->easy_handle == _curl.get());

            if (m->data.result != CURLE_OK) {
                NLOG(Nakama::NLogLevel::Error, "libcurl error: %s", curl_easy_strerror(m->data.result));
                return NetIOAsyncResult::ERROR;
            }

            return NetIOAsyncResult::DONE;
        }

        return NetIOAsyncResult::AGAIN;
    }

    // returns number of bytes sent or negative error code
    int send(const void* buf, size_t len, int *would_block) noexcept {
        size_t sent;
        CURLcode res = curl_easy_send(_curl.get(), buf, len, &sent);
        *would_block = 0;
        if (res == CURLE_AGAIN) {
            *would_block=1;
            return 0;
        }
        else if (res != CURLE_OK) {
            NLOG(Nakama::NLogLevel::Error, "libcurl send error: %s", curl_easy_strerror(res));
            return -1;
        }
        else {  // CURLE_OK
            return sent;
        }
    }

    // returns number of bytes consumed
    int recv(void* buf, ssize_t len, int *would_block) noexcept {
        size_t nbytes;
        CURLcode res = curl_easy_recv(_curl.get(), buf, len, &nbytes);
        *would_block = 0;
        if (res == CURLE_AGAIN) {
            *would_block=1;
            return 0;
        }
        else if (res != CURLE_OK) {
            NLOG(Nakama::NLogLevel::Error, "libcurl recv error: %s", curl_easy_strerror(res));
            return -1;
        }
        else {  // CURLE_OK
            return nbytes;
        }
    }

    void close() noexcept {
        if (_curl) {
            curl_multi_remove_handle(_curlm.get(), _curl.get());
        }
        _curl.reset(nullptr);
        _curl_url.reset(nullptr);
    }

private:
};

NAKAMA_NAMESPACE_END