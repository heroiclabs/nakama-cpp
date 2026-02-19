#include <nakama-cpp/Nakama.h>
#include <nakama-cpp/NakamaVersion.h>

#include <chrono>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <cstdio>

using namespace Nakama;

static const int NUM_ITERATIONS = 5;
static const int DEFAULT_TICK_MS = 10;

// Simple promise-like helper for callback-based API
template<typename T>
class CallbackResult {
public:
    void set(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        result = std::move(value);
        ready = true;
        cv.notify_all();
    }
    void setError(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        error = msg;
        ready = true;
        cv.notify_all();
    }
    T get(int timeoutMs = 15000) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]{ return ready; });
        if (!ready) throw std::runtime_error("Timeout waiting for callback");
        if (!error.empty()) throw std::runtime_error(error);
        return result;
    }
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
    T result{};
    std::string error;
};

template<>
class CallbackResult<void> {
public:
    void set() {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
        cv.notify_all();
    }
    void setError(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        error = msg;
        ready = true;
        cv.notify_all();
    }
    void get(int timeoutMs = 15000) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]{ return ready; });
        if (!ready) throw std::runtime_error("Timeout waiting for callback");
        if (!error.empty()) throw std::runtime_error(error);
    }
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
    std::string error;
};

// Tick a client + rtClient in a background thread
class Ticker {
public:
    Ticker(NClientPtr client, NRtClientPtr rtClient, int tickMs = DEFAULT_TICK_MS)
        : _client(client), _rtClient(rtClient), _tickMs(tickMs), _running(true) {
        _thread = std::thread([this]{
            while (_running) {
                _client->tick();
                if (_rtClient) _rtClient->tick();
                std::this_thread::sleep_for(std::chrono::milliseconds(_tickMs));
            }
        });
    }
    ~Ticker() { stop(); }
    void stop() {
        _running = false;
        if (_thread.joinable()) _thread.join();
    }
private:
    NClientPtr _client;
    NRtClientPtr _rtClient;
    int _tickMs;
    std::atomic<bool> _running;
    std::thread _thread;
};

void test_rt_throughput_with_tick_interval(const NClientParameters& params, int tickIntervalMs) {
    const int NUM_CHUNKS = 3;
    const int CHUNK_SIZE = 1 * 1024 * 1024;
    const int TOTAL_BYTES = NUM_CHUNKS * CHUNK_SIZE;
    std::string label = "throughput_tick_" + std::to_string(tickIntervalMs) + "ms";

    std::cout << "=== Throughput benchmark (tick=" << tickIntervalMs << "ms) ===" << std::endl;

    NClientPtr testSenderClient = createRestClient(params);
    NRtClientPtr testSenderRt = testSenderClient->createRtClient(7350);
    NRtDefaultClientListener testSenderListener;
    testSenderRt->setListener(&testSenderListener);

    NClientPtr testReceiverClient = createRestClient(params);
    NRtClientPtr testReceiverRt = testReceiverClient->createRtClient(7350);
    NRtDefaultClientListener testReceiverListener;
    testReceiverRt->setListener(&testReceiverListener);

    Ticker testSenderTicker(testSenderClient, testSenderRt, tickIntervalMs);
    Ticker testReceiverTicker(testReceiverClient, testReceiverRt, tickIntervalMs);

    CallbackResult<NSessionPtr> senderAuthResult;
    testSenderClient->authenticateCustom(
        label + "_sender-" + std::to_string(std::rand()),
        "", true, {},
        [&](NSessionPtr session) { senderAuthResult.set(session); },
        [&](const NError& err) { senderAuthResult.setError(err.message); }
    );
    NSessionPtr senderSession = senderAuthResult.get();

    CallbackResult<NSessionPtr> receiverAuthResult;
    testReceiverClient->authenticateCustom(
        label + "_receiver-" + std::to_string(std::rand()),
        "", true, {},
        [&](NSessionPtr session) { receiverAuthResult.set(session); },
        [&](const NError& err) { receiverAuthResult.setError(err.message); }
    );
    NSessionPtr receiverSession = receiverAuthResult.get();

    CallbackResult<void> senderConnResult;
    testSenderListener.setConnectCallback([&]{ senderConnResult.set(); });
    testSenderRt->connect(senderSession, false, NRtClientProtocol::Protobuf);
    senderConnResult.get();

    CallbackResult<void> receiverConnResult;
    testReceiverListener.setConnectCallback([&]{ receiverConnResult.set(); });
    testReceiverRt->connect(receiverSession, false, NRtClientProtocol::Protobuf);
    receiverConnResult.get();

    CallbackResult<NMatch> createMatchResult;
    testSenderRt->createMatch(
        [&](const NMatch& m) { createMatchResult.set(m); },
        [&](const NRtError& err) { createMatchResult.setError(err.message); }
    );
    NMatch match = createMatchResult.get();
    std::string matchId = match.matchId;

    CallbackResult<NMatch> joinResult;
    testReceiverRt->joinMatch(
        matchId, {},
        [&](const NMatch& m) { joinResult.set(m); },
        [&](const NRtError& err) { joinResult.setError(err.message); }
    );
    joinResult.get();

    std::string chunk(CHUNK_SIZE, 'X');
    NBytes chunkBytes(chunk.begin(), chunk.end());

    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> chunksReceived{0};
    testReceiverListener.setMatchDataCallback(
        [&](const NMatchData&) {
            int count = chunksReceived.fetch_add(1) + 1;
            if (count >= NUM_CHUNKS) {
                std::lock_guard<std::mutex> lock(mtx);
                cv.notify_all();
            }
        }
    );

    auto t0 = std::chrono::high_resolution_clock::now();
    testSenderRt->sendMatchData(matchId, 1, chunkBytes);
    testSenderRt->sendMatchData(matchId, 2, chunkBytes);
    testSenderRt->sendMatchData(matchId, 3, chunkBytes);
    auto tSent = std::chrono::high_resolution_clock::now();

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, std::chrono::seconds(30), [&]{ return chunksReceived.load() >= NUM_CHUNKS; });
    }

    auto tRecv = std::chrono::high_resolution_clock::now();

    long long sendMs = std::chrono::duration_cast<std::chrono::milliseconds>(tSent - t0).count();
    long long totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(tRecv - t0).count();
    double throughputMBs = (totalMs > 0)
        ? (double)TOTAL_BYTES / (1024.0 * 1024.0) / ((double)totalMs / 1000.0)
        : 0.0;

    char throughputStr[32];
    snprintf(throughputStr, sizeof(throughputStr), "%.1f", throughputMBs);

    std::cout << "=== Throughput results (tick=" << tickIntervalMs << "ms) ==="
              << " Send: " << sendMs << "ms"
              << " Total: " << totalMs << "ms"
              << " Throughput: " << throughputStr << " MB/s"
              << " Chunks received: " << chunksReceived.load()
              << std::endl;

    testReceiverRt->disconnect();
    testSenderRt->disconnect();
    testReceiverTicker.stop();
    testSenderTicker.stop();
}

int main() {
    NLogger::initWithConsoleSink(NLogLevel::Info);
    std::cout << "=== v2.4.1 Hot-join match benchmark ===" << std::endl;
    std::cout << "SDK version: " << Nakama::getNakamaSdkVersion() << std::endl;
    std::cout << "Iterations: " << NUM_ITERATIONS << std::endl;

    NClientParameters params;
    params.serverKey = "defaultkey";
    params.host = "127.0.0.1";
    params.port = 7350;

    const int tickRates[] = {1, 2, 5, 10, 16, 20, 50};
    std::cout << "=== Throughput tick rate correlation test (3 x 1MB parallel) ===" << std::endl;
    for (int tickMs : tickRates) {
        test_rt_throughput_with_tick_interval(params, tickMs);
    }

    std::cout << "=== Hot-join tick rate correlation test ===" << std::endl;

    // Create host client
    NClientPtr hostClient = createRestClient(params);
    NRtClientPtr hostRt = hostClient->createRtClient(7350);
    NRtDefaultClientListener hostListener;
    hostRt->setListener(&hostListener);

    Ticker hostTicker(hostClient, hostRt, DEFAULT_TICK_MS);

    // Authenticate host
    CallbackResult<NSessionPtr> hostAuthResult;
    hostClient->authenticateCustom(
        "hotjoin-host-" + std::to_string(std::rand()),
        "", true, {},
        [&](NSessionPtr session) { hostAuthResult.set(session); },
        [&](const NError& err) { hostAuthResult.setError(err.message); }
    );
    NSessionPtr hostSession = hostAuthResult.get();
    std::cout << "Host authenticated" << std::endl;

    // Connect host RT
    CallbackResult<void> hostConnResult;
    hostListener.setConnectCallback([&]{ hostConnResult.set(); });
    hostRt->connect(hostSession, false, NRtClientProtocol::Protobuf);
    hostConnResult.get();
    std::cout << "Host connected" << std::endl;

    // Create match
    CallbackResult<NMatch> createMatchResult;
    hostRt->createMatch(
        [&](const NMatch& match) { createMatchResult.set(match); },
        [&](const NRtError& err) { createMatchResult.setError(err.message); }
    );
    NMatch match = createMatchResult.get();
    std::string matchId = match.matchId;
    std::cout << "Match created: " << matchId << std::endl;

    long long totalJoinMs = 0;
    long long minJoinMs = 999999;
    long long maxJoinMs = 0;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        NClientPtr joinerClient = createRestClient(params);
        NRtClientPtr joinerRt = joinerClient->createRtClient(7350);
        NRtDefaultClientListener joinerListener;
        joinerRt->setListener(&joinerListener);

        Ticker joinerTicker(joinerClient, joinerRt, DEFAULT_TICK_MS);

        // Authenticate joiner
        CallbackResult<NSessionPtr> joinerAuthResult;
        joinerClient->authenticateCustom(
            "hotjoin-joiner-" + std::to_string(i) + "-" + std::to_string(std::rand()),
            "", true, {},
            [&](NSessionPtr session) { joinerAuthResult.set(session); },
            [&](const NError& err) { joinerAuthResult.setError(err.message); }
        );
        NSessionPtr joinerSession = joinerAuthResult.get();

        // Connect joiner RT
        CallbackResult<void> joinerConnResult;
        joinerListener.setConnectCallback([&]{ joinerConnResult.set(); });
        joinerRt->connect(joinerSession, false, NRtClientProtocol::Protobuf);
        joinerConnResult.get();

        // Time the hot-join
        CallbackResult<NMatch> joinResult;
        auto t0 = std::chrono::high_resolution_clock::now();
        joinerRt->joinMatch(
            matchId, {},
            [&](const NMatch& m) { joinResult.set(m); },
            [&](const NRtError& err) { joinResult.setError(err.message); }
        );
        NMatch joined = joinResult.get();
        auto t1 = std::chrono::high_resolution_clock::now();

        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        totalJoinMs += ms;
        if (ms < minJoinMs) minJoinMs = ms;
        if (ms > maxJoinMs) maxJoinMs = ms;

        std::cout << "  Iteration " << (i + 1) << ": joinMatch = " << ms << "ms"
                  << " (presences: " << joined.presences.size() << ")" << std::endl;

        // Leave match
        CallbackResult<void> leaveResult;
        joinerRt->leaveMatch(matchId,
            [&]{ leaveResult.set(); },
            [&](const NRtError& err) { leaveResult.setError(err.message); }
        );
        leaveResult.get();

        joinerRt->disconnect();
        joinerTicker.stop();
    }

    std::cout << "=== Hot-join results (v2.4.1) ===" << std::endl;
    std::cout << "  Min:  " << minJoinMs << "ms" << std::endl;
    std::cout << "  Max:  " << maxJoinMs << "ms" << std::endl;
    std::cout << "  Avg:  " << (totalJoinMs / NUM_ITERATIONS) << "ms" << std::endl;

    hostRt->disconnect();
    hostTicker.stop();

    return 0;
}
