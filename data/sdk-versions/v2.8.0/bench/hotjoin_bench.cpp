#include <nakama-cpp/ClientFactory.h>
#include <nakama-cpp/NClientInterface.h>
#include <nakama-cpp/realtime/NRtDefaultClientListener.h>
#include <nakama-cpp/log/NLogger.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <vector>

using namespace Nakama;

static const int NUM_ITERATIONS = 5;
static const int TICK_MS = 10;

class Ticker {
public:
    void add(NClientPtr client, NRtClientPtr rtClient = nullptr) {
        _clients.push_back(client);
        if (rtClient) _rtClients.push_back(rtClient);
    }
    void start() {
        _running = true;
        _thread = std::thread([this]{
            while (_running) {
                for (auto& c : _clients) c->tick();
                for (auto& r : _rtClients) r->tick();
                std::this_thread::sleep_for(std::chrono::milliseconds(TICK_MS));
            }
        });
    }
    void stop() {
        _running = false;
        if (_thread.joinable()) _thread.join();
    }
    ~Ticker() { stop(); }
private:
    std::vector<NClientPtr> _clients;
    std::vector<NRtClientPtr> _rtClients;
    std::atomic<bool> _running{false};
    std::thread _thread;
};

int main() {
    NLogger::initWithConsoleSink(NLogLevel::Info);
    std::cout << "=== v2.8.0 Hot-join match benchmark ===" << std::endl;
    std::cout << "Iterations: " << NUM_ITERATIONS << std::endl;

    NClientParameters params;
    params.serverKey = "defaultkey";
    params.host = "127.0.0.1";
    params.port = 7350;

    auto hostClient = createDefaultClient(params);
    auto hostRt = hostClient->createRtClient();
    NRtDefaultClientListener hostListener;
    hostRt->setListener(&hostListener);

    Ticker hostTicker;
    hostTicker.add(hostClient, hostRt);
    hostTicker.start();

    // Authenticate host
    NSessionPtr hostSession = hostClient->authenticateCustomAsync("hotjoin-host-v280", std::string(), true).get();
    bool createStatus = false;
    hostRt->connectAsync(hostSession, createStatus, NRtClientProtocol::Protobuf).get();
    std::cout << "Host connected" << std::endl;

    // Create match
    NMatch match = hostRt->createMatchAsync().get();
    std::string matchId = match.matchId;
    std::cout << "Match created: " << matchId << std::endl;

    long long totalJoinMs = 0;
    long long minJoinMs = std::numeric_limits<long long>::max();
    long long maxJoinMs = 0;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        auto joinerClient = createDefaultClient(params);
        auto joinerRt = joinerClient->createRtClient();
        NRtDefaultClientListener joinerListener;
        joinerRt->setListener(&joinerListener);

        Ticker joinerTicker;
        joinerTicker.add(joinerClient, joinerRt);
        joinerTicker.start();

        NSessionPtr joinerSession = joinerClient->authenticateCustomAsync(
            "hotjoin-joiner-v280-" + std::to_string(i), std::string(), true).get();
        joinerRt->connectAsync(joinerSession, createStatus, NRtClientProtocol::Protobuf).get();

        // Time the hot-join
        auto t0 = std::chrono::high_resolution_clock::now();
        NMatch joined = joinerRt->joinMatchAsync(matchId, {}).get();
        auto t1 = std::chrono::high_resolution_clock::now();

        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        totalJoinMs += ms;
        if (ms < minJoinMs) minJoinMs = ms;
        if (ms > maxJoinMs) maxJoinMs = ms;

        std::cout << "  Iteration " << (i + 1) << ": joinMatch = " << ms << "ms"
                  << " (presences: " << joined.presences.size() << ")" << std::endl;

        joinerRt->leaveMatchAsync(matchId).get();
        joinerRt->disconnectAsync().get();
        joinerTicker.stop();
    }

    std::cout << "=== Hot-join results (v2.8.0) ===" << std::endl;
    std::cout << "  Min:  " << minJoinMs << "ms" << std::endl;
    std::cout << "  Max:  " << maxJoinMs << "ms" << std::endl;
    std::cout << "  Avg:  " << (totalJoinMs / NUM_ITERATIONS) << "ms" << std::endl;

    hostRt->disconnectAsync().get();
    hostTicker.stop();

    return 0;
}
