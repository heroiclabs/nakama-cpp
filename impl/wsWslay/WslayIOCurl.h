#pragma once
#include <string>
#include <memory>
#include <curl/curl.h>
#include <nakama-cpp/log/NLogger.h>
#include "StrUtil.h"
#include "nakama-cpp/realtime/wslay/WslayIOInterface.h"

#if __ANDROID__
#include "AndroidCA.h"
#endif

NAKAMA_NAMESPACE_BEGIN

class WslayIOCurl : public WslayIOInterface {
    public:
        ~WslayIOCurl() override {

        }

        WslayIOCurl() noexcept:
            _curl(nullptr, curl_easy_cleanup),
            _curlm(curl_multi_init(), curl_multi_cleanup),
            _curl_url(nullptr, curl_url_cleanup)
            {}

        // should establish SSL-aware connection
        NetIOAsyncResult connect_init(const URLParts& urlParts) noexcept override {
            decltype(_curl_url) u(curl_url(), curl_url_cleanup);

            CURLUcode rc = curl_url_set(u.get(), CURLUPART_URL , urlParts.url.c_str(), CURLU_NON_SUPPORT_SCHEME);
            if (rc) {
                NLOG(Nakama::NLogLevel::Error, "invalid URL(%s): %s", urlParts.url.c_str(), curl_url_strerror(rc));
                return NetIOAsyncResult::ERR;
            }
            if (urlParts.scheme == "wss") {
                curl_url_set(u.get(), CURLUPART_SCHEME, "https", 0);
            }
            else {
                curl_url_set(u.get(), CURLUPART_SCHEME, "http", 0);
            }

            _curl.reset(curl_easy_init());
            curl_easy_setopt(_curl.get(), CURLOPT_CURLU, u.get());
            curl_easy_setopt(_curl.get(), CURLOPT_CONNECT_ONLY, 1);
    #ifdef CURL_DEBUG
            curl_easy_setopt(_curl.get(), CURLOPT_VERBOSE, 1);
    #endif
            curl_easy_setopt(_curl.get(), CURLOPT_CONNECTTIMEOUT, 5);

    #if __ANDROID__
            CACertificateData* data = Nakama::getCaCertificates();
    if (data == nullptr) {
      // Has System.loadLibrary("nakama-sdk") been called?
      NLOG(Nakama::NLogLevel::Error, "libcurl error: could not access CA Certificates.");
      return NetIOAsyncResult::ERR;
    } else {
      struct curl_blob blob;
      blob.data = reinterpret_cast<char*>(data->data);
      blob.len = data->len;
      blob.flags = CURL_BLOB_COPY;
      curl_easy_setopt(_curl.get(), CURLOPT_CAINFO_BLOB, &blob);
    }
    #endif

            // only way to do async connect is via curl_multi interface
            if (CURLMcode res = curl_multi_add_handle(_curlm.get(), _curl.get()); res != CURLM_OK) {
                NLOG(Nakama::NLogLevel::Error, "libcurl error: %s", curl_multi_strerror(res));
                _curl.reset();
                return NetIOAsyncResult::ERR;
            }

            //save curl_url because it must stay alive as long as curl_easy uses it
            _curl_url.swap(u);

            return NetIOAsyncResult::DONE;
        }

        NetIOAsyncResult connect_tick() override {
            int running_handles;

            if (CURLMcode res = curl_multi_perform(_curlm.get(), &running_handles); res != CURLM_OK) {
                NLOG(Nakama::NLogLevel::Error, "libcurl error: %s", curl_multi_strerror(res));
                return NetIOAsyncResult::ERR;
            }

            // done connecting (or failed) , either way take easy_handle out of multi
            if (running_handles == 0) {
                int msgq;

                struct CURLMsg* m = curl_multi_info_read(_curlm.get(), &msgq);
                if (!(m && m->msg == CURLMSG_DONE)) {
                    NLOG(Nakama::NLogLevel::Error, "unexpected libcurl error: curl_multi_info_read had no completed messages");
                    return NetIOAsyncResult::ERR;
                }

                assert(msgq == 0);  // there should be at most 1 easy handle in multi handle
                assert(m->easy_handle == _curl.get());

                if (m->data.result != CURLE_OK) {
                    NLOG(Nakama::NLogLevel::Error, "libcurl error: %s", curl_easy_strerror(m->data.result));
                    return NetIOAsyncResult::ERR;
                }

                return NetIOAsyncResult::DONE;
            }

            return NetIOAsyncResult::AGAIN;
        }

        // returns number of bytes sent or negative error code
        int send(const void* buf, size_t len, int *would_block) noexcept override {
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
        int recv(void* buf, size_t len, int* would_block) noexcept override {
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

        void close() noexcept override {
            if (_curl) {
                curl_multi_remove_handle(_curlm.get(), _curl.get());
            }
            _curl.reset(nullptr);
            _curl_url.reset(nullptr);
        }

private:
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> _curl;
    std::unique_ptr<CURLM, decltype(&curl_multi_cleanup)> _curlm;
    std::unique_ptr<CURLU, decltype(&curl_url_cleanup)> _curl_url;
};

NAKAMA_NAMESPACE_END