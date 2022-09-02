//
// Created by yamlcoder on 17/03/2022.
//

#include <httpClient/httpClient.h>
#include <nakama-cpp/log/NLogger.h>
#include "NHttpClientLibHC.h"
namespace Nakama {

HC_DEFINE_TRACE_AREA(httpTransportLibHC, HCTraceLevel::Error);

// Converts our loglevel to HCTracelevel
static HCTraceLevel HCLevelForNLevel(NLogLevel lvl) {
    switch (lvl) {
        case NLogLevel::Debug:
            return HCTraceLevel::Verbose;
        case NLogLevel::Info:
            return HCTraceLevel::Information;
        case NLogLevel::Warn:
            return HCTraceLevel::Warning;
        case NLogLevel::Error:
            return HCTraceLevel::Error;
        case NLogLevel::Fatal:
            return HCTraceLevel::Error;
    }

    return HCTraceLevel::Off;
}

void configureNLogger() {
    static bool configured = false;
    auto l = NLogger::getSink();
    if (!l) {
        return;
    }

    if (configured) return;
    configured = true;

    HCSettingsSetTraceLevel(HCLevelForNLevel(l->getLevel()));
    HCTraceSetClientCallback([](
            const char* areaName, HCTraceLevel level, uint64_t /*threadId*/, uint64_t /*timestamp*/, const char* message) {
        switch (level) {
            case HCTraceLevel::Verbose:
                return NLogger::Debug(message, areaName);
            case HCTraceLevel::Information:
                return NLogger::Info(message, areaName);
            case HCTraceLevel::Important:
            case HCTraceLevel::Warning:
                return NLogger::Warn(message, areaName);
            case HCTraceLevel::Error:
                return NLogger::Error(message, areaName);
            case HCTraceLevel::Off:
                return;
        }
    });
}

NHttpClientLibHC::NHttpClientLibHC(const NPlatformParameters& platformParams):
    m_queue(nullptr, &XTaskQueueCloseHandle),
    m_baseUri(),
    m_queue_terminated(false)
{
    configureNLogger();

#ifdef __ANDROID__
    HCInitArgs initArgs{
        .javaVM = platformParams.javaVM,
        .applicationContext = platformParams.applicationContext
    };
    HCInitArgs* initArgsParam = &initArgs;
#else
    (void)platformParams;
    HCInitArgs* initArgsParam = nullptr;
#endif


    HRESULT hr = HCInitialize(initArgsParam);
    if (FAILED(hr)) {
        HC_TRACE_ERROR_HR(httpTransportLibHC, hr, "HCInitialize failed");
    }
    XTaskQueueHandle q;

    XTaskQueueCreate(XTaskQueueDispatchMode::ThreadPool,
                     XTaskQueueDispatchMode::Manual,  // callbacks from ticks
                     &q
                     );
    m_queue.reset(q);
}

NHttpClientLibHC::~NHttpClientLibHC() noexcept
{
    HCCleanup();
}

void NHttpClientLibHC::setBaseUri(const std::string& uri)
{
    m_baseUri = uri;
}

#define CHECK_CB(exp, msg, cb) { \
    HRESULT hr = exp; if (FAILED(hr)) { \
        HC_TRACE_ERROR_HR(httpTransportLibHC, hr, msg); \
        submit_cb(cb, InternalStatusCodes::INTERNAL_TRANSPORT_ERROR, "", msg); \
        return; \
    }}

#define CHECK_AND_LOG(hr, msg) { if (FAILED(hr)) { HC_TRACE_ERROR_HR(httpTransportLibHC, hr, msg); return hr; } }


using call_ptr = std::unique_ptr<HC_CALL, decltype(&HCHttpCallCloseHandle)>;
HRESULT NHttpClientLibHC::prep_hc_call(const NHttpRequest& req, call_ptr& call) {
    HCCallHandle pcall = nullptr;
    CHECK_AND_LOG(HCHttpCallCreate(&pcall), "HCHttpCallCreate failed");
    call.reset(pcall);

    const char *callMethod = nullptr;
    switch (req.method) {
        case NHttpReqMethod::POST:
            callMethod = "POST";
            break;
        case NHttpReqMethod::GET :
            callMethod = "GET";
            break;
        case NHttpReqMethod::PUT :
            callMethod = "PUT";
            break;
        case NHttpReqMethod::DEL :
            callMethod = "DELETE";
            break;
    }
    std::string uri = m_baseUri + req.path + (req.queryArgs.empty() ? "" : "?");
    for (auto p: req.queryArgs) {
        uri += p.first + "=" + p.second + "&";
    };

    CHECK_AND_LOG(HCHttpCallRequestSetUrl(pcall, callMethod, uri.c_str()), "HCHttpCallRequestSetUrl failed");

    if (req.body.size()) {
        CHECK_AND_LOG(HCHttpCallRequestSetRequestBodyBytes(pcall, reinterpret_cast<const uint8_t *>(req.body.data()),
                                                           uint32_t(req.body.size())), "HCHttpCallRequestSetRequestBodyBytes");
    }

    for (auto p: req.headers) {
        HCHttpCallRequestSetHeader(pcall, p.first.c_str(), p.second.c_str(), true);
    }

    return S_OK;
}

using ctx_t = std::tuple<call_ptr, NHttpResponseCallback, std::string>;
static HRESULT setup_hc_response_to_string(ctx_t &ctx)
{
    HC_CALL *call = std::get<0>(ctx).get();
    HCHttpCallResponseBodyWriteFunction write_func = [](HCCallHandle /*call*/, const uint8_t* source, size_t bytesAvailable, void* context) {
        ctx_t *ctx = static_cast<ctx_t *>(context);
        std::get<2>(*ctx).append(reinterpret_cast<const char *>(source), bytesAvailable);
        return S_OK;
    };
    return HCHttpCallResponseSetResponseBodyWriteFunction(call, write_func, static_cast<void *>(&ctx));
}

// This call back is called libHC when request is completed or failed
// Prior to performing request we stashed ctx_t (HC_CALL, SDK callbacks, body)
// in the asyncBlock context. Here we extract relevant information
// to call SDK callback. Once this function is done final cleanup is
// done via unique_ptr<HC_CALL> destructor implicitly.
static void hc_req_completed_cb(XAsyncBlock *ab) noexcept {
    const std::unique_ptr<XAsyncBlock> asyncBlock(ab);
    const std::unique_ptr<ctx_t> ctx(static_cast<ctx_t*>(asyncBlock->context));

    HC_CALL* call = std::get<0>(*ctx).get();
    auto &callback = std::get<1>(*ctx);
    {
        NHttpResponse resp{InternalStatusCodes::CANCELLED_BY_USER, "", ""};
        HRESULT hr = XAsyncGetStatus(asyncBlock.get(), false);
        if (E_ABORT == hr) {
            HC_TRACE_ERROR_HR(httpTransportLibHC, hr, "Aborted XAsyncGetStatus");
            resp.statusCode = InternalStatusCodes::CANCELLED_BY_USER;
            callback(std::make_shared<NHttpResponse>(resp));
        }
//        else if (FAILED(hr)) {
//            HC_TRACE_ERROR_HR(httpTransportLibHC, hr, "XAsyncGetStatus");
//            resp.statusCode = InternalStatusCodes::INTERNAL_TRANSPORT_ERROR;
//            callback(std::make_shared<NHttpResponse>(resp));
//        }
    }

    HRESULT errCode = 0;
    uint32_t platErrCode = 0;
    uint32_t hc_statusCode = 0;
    int statusCode = 0;
    std::string responseString;
    std::string errMessage;

    HCHttpCallResponseGetNetworkErrorCode(call, &errCode, &platErrCode);
    HCHttpCallResponseGetStatusCode(call, &hc_statusCode);
    statusCode = hc_statusCode;

    if (FAILED(errCode)) {
        const char *errMsg;
        HCHttpCallResponseGetPlatformNetworkErrorMessage(call, &errMsg);
        errMessage = errMsg;
        statusCode = InternalStatusCodes::CONNECTION_ERROR; //FIXME: need a way to indicate request timeout
    }

    callback(std::make_shared<NHttpResponse>(
            NHttpResponse{statusCode,
                          std::move(std::get<2>(*ctx)),
                          std::move(errMessage)}));
};

void NHttpClientLibHC::request(const NHttpRequest& req, const NHttpResponseCallback& callback)
{
    if (m_baseUri.empty())
    {
        submit_cb(callback, InternalStatusCodes::NOT_INITIALIZED_ERROR, "", "[NHttpClientLibHC::request] base uri is not set");
        return;
    }
    call_ptr call(nullptr, &HCHttpCallCloseHandle);
    CHECK_CB(prep_hc_call(req, call), "Error preparing HC_CALL", callback);

    XAsyncBlock *asyncBlock = new XAsyncBlock{};
    ctx_t *ctx = new ctx_t(std::move(call), callback, std::string());
    asyncBlock->context = ctx;
    asyncBlock->queue = m_queue.get();  //unique_ptr m_queue will outlive asyncBlock, so thats fine
    if (callback) {
        CHECK_CB(setup_hc_response_to_string(*ctx), "HCHttpCallResponseSetResponseBodyWriteFunction failed", callback);
        asyncBlock->callback = &hc_req_completed_cb;
    };
    CHECK_CB(HCHttpCallPerformAsync(std::get<0>(*ctx).get(), asyncBlock), "HCHttpCallPerformAsync failed", callback);
}

void NHttpClientLibHC::tick()
{
    while(XTaskQueueDispatch(m_queue.get(), XTaskQueuePort::Completion, 0)) {};
}

void NHttpClientLibHC::submit_cb(const NHttpResponseCallback &cb, int statusCode, std::string body, std::string err) noexcept
{
    if (cb) {
        using xtask_cb_ctx_t = std::tuple<NHttpResponseCallback, NHttpResponse>;
        xtask_cb_ctx_t* ctx = new auto(std::make_tuple(cb, NHttpResponse{statusCode, body, err}));
        XTaskQueueSubmitCallback(m_queue.get(), XTaskQueuePort::Completion, ctx, [](void *ctx, bool /*cancelled*/) {
            //We deliberately ignore _cancelled and deliver callback as it was requested
            std::unique_ptr<xtask_cb_ctx_t> context(static_cast<xtask_cb_ctx_t *>(ctx));
            auto &cb = std::get<0>(*context);
            auto &response = std::get<1>(*context);
            cb(std::make_shared<NHttpResponse>(std::move(response)));
        });
    }
}

void NHttpClientLibHC::cancelAllRequests() {
    // We can't use `wait` because if we are called from the same thread tick is being called,
    // then we deadlock should there be any outstanding requests. So instead we use termination
    // callback which will be a last callback on the Completion port
    XTaskQueueTerminate(m_queue.get(), false, &m_queue_terminated, [](void *ctx){
        static_cast<std::atomic<bool>*>(ctx)->store(true, std::memory_order_relaxed);
    });

    // Pump completion queue until our callback is executed.  tick() can still be called in
    // parallel thread, then it will be set from there, but we do our pumping here in case
    // tick is not called anymore. That is also why we have atomic bool, not just bool.
    while(!m_queue_terminated.load(std::memory_order_relaxed)) {
        XTaskQueueDispatch(m_queue.get(), XTaskQueuePort::Completion, 50);
    }
}
}