/*
 * Copyright 2019 The Nakama Authors
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

#include "NTest.h"
#include "TestGuid.h"
#include "nakama-cpp/log/NLogger.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace Nakama {
namespace Test {

using namespace std;

void test_rt_create_match() {
  const bool threadedTick = true;
  NTest test1(__func__, threadedTick);

  test1.runTest();

  NSessionPtr session = test1.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  bool createStatus = false;
  test1.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();

  NMatch match = test1.rtClient->createMatchAsync().get();

  test1.stopTest(true);

  NLOG_INFO("stopped create match");
}

void test_rt_matchmaker() {
  NLOG_INFO("started testing matchmaker");

  bool threadedTick = true;
  NTest test1(__func__, threadedTick);
  NTest test2(std::string(__func__) + std::string("2"), threadedTick);

  test1.setTestTimeoutMs(20000);
  test2.setTestTimeoutMs(20000);

  test1.runTest();
  test2.runTest();

  NLOG_INFO("authing");

  NSessionPtr session = test1.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  bool createStatus = false;
  test1.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();

  NLOG_INFO("done 1");

  NSessionPtr session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  test2.rtClient->connectAsync(session2, createStatus, NTest::RtProtocol).get();

  NLOG_INFO("done 2");
  NLOG_INFO("connected");

  auto matchedPromise = std::promise<NMatchmakerMatchedPtr>();
  auto matched2Promise = std::promise<NMatchmakerMatchedPtr>();

  test1.listener.setMatchmakerMatchedCallback(
      [&test1, &matchedPromise](NMatchmakerMatchedPtr matched) { matchedPromise.set_value(matched); });

  test2.listener.setMatchmakerMatchedCallback(
      [&test2, &matched2Promise](NMatchmakerMatchedPtr matched) { matched2Promise.set_value(matched); });

  const int minCount = 2;
  const int maxCount = 2;
  const std::string query = "*";
  const NStringMap stringProperties = {};
  const NStringDoubleMap numericProperties = {};
  const int countMultiple = 1;

  NLOG_INFO("adding matchmaker");

  NMatchmakerTicket ticket =
      test1.rtClient->addMatchmakerAsync(minCount, maxCount, query, stringProperties, numericProperties, countMultiple)
          .get();
  NMatchmakerTicket ticket2 =
      test2.rtClient->addMatchmakerAsync(minCount, maxCount, query, stringProperties, numericProperties, countMultiple)
          .get();

  NLOG_INFO("got tickets");

  NMatchmakerMatchedPtr matched = matchedPromise.get_future().get();
  NMatchmakerMatchedPtr matched2 = matched2Promise.get_future().get();

  NLOG_INFO("got matched");
  NLOG_INFO(matched->matchId);
  NLOG_INFO(matched2->matchId);

  test1.rtClient->joinMatchByTokenAsync(matched->token).get();
  test2.rtClient->joinMatchByTokenAsync(matched2->token).get();

  test1.stopTest(true);
  test2.stopTest(true);
}

void test_rt_sendmatchdata_throughput() {
  NLOG_INFO("started testing SendMatchData throughput");

  bool threadedTick = true;
  NTest test1(__func__, threadedTick);
  NTest test2(std::string(__func__) + std::string("2"), threadedTick);

  test1.setTestTimeoutMs(20000);
  test2.setTestTimeoutMs(20000);

  test1.runTest();
  test2.runTest();

  NLOG_INFO("authing");

  NSessionPtr session = test1.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  bool createStatus = false;
  test1.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();

  NLOG_INFO("done 1");

  NSessionPtr session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  test2.rtClient->connectAsync(session2, createStatus, NTest::RtProtocol).get();

  NLOG_INFO("done 2");
  NLOG_INFO("connected");

  auto matchedPromise = std::promise<NMatchmakerMatchedPtr>();
  auto matched2Promise = std::promise<NMatchmakerMatchedPtr>();

  test1.listener.setMatchmakerMatchedCallback(
      [&test1, &matchedPromise](NMatchmakerMatchedPtr matched) { matchedPromise.set_value(matched); });

  test2.listener.setMatchmakerMatchedCallback(
      [&test2, &matched2Promise](NMatchmakerMatchedPtr matched) { matched2Promise.set_value(matched); });

  const int minCount = 2;
  const int maxCount = 2;
  const std::string query = "*";
  const NStringMap stringProperties = {};
  const NStringDoubleMap numericProperties = {};
  const int countMultiple = 1;

  NLOG_INFO("adding matchmaker");

  NMatchmakerTicket ticket =
      test1.rtClient->addMatchmakerAsync(minCount, maxCount, query, stringProperties, numericProperties, countMultiple)
          .get();
  NMatchmakerTicket ticket2 =
      test2.rtClient->addMatchmakerAsync(minCount, maxCount, query, stringProperties, numericProperties, countMultiple)
          .get();

  NLOG_INFO("got tickets");

  NMatchmakerMatchedPtr matched = matchedPromise.get_future().get();
  NMatchmakerMatchedPtr matched2 = matched2Promise.get_future().get();

  NLOG_INFO("got matched");
  NLOG_INFO(matched->matchId);
  NLOG_INFO(matched2->matchId);

  test1.rtClient->joinMatchByTokenAsync(matched->token).get();
  test2.rtClient->joinMatchByTokenAsync(matched2->token).get();

  std::string testMessages[3];
  for (int i = 0; i < 3; ++i) {
    // Generate around 1MB messages to stress test throughput
    while (testMessages[i].size() < 100000000) {
      testMessages[i] +=
        "Certe, inquam, pertinax non ero tibique, si mihi probabis ea, quae dices, libenter assentiar. Probabo, inquit, modo ista sis aequitate, quam ostendis. sed uti oratione perpetua malo quam interrogare aut interrogari. Ut placet, inquam. Tum dicere exorsus est. Primum igitur, inquit, sic agam, ut ipsi auctori huius disciplinae placet: constituam, quid et quale sit id, de quo quaerimus, non quo ignorare vos arbitrer, sed ut ratione et via procedat oratio. quaerimus igitur, quid sit extremum et ultimum bonorum, quod omnium philosophorum sententia tale debet esse, ut ad id omnia referri oporteat, ipsum autem nusquam. hoc Epicurus in voluptate ponit, quod summum bonum esse vult, summumque malum dolorem, idque instituit docere sic: Omne animal, simul atque natum sit, voluptatem appetere eaque gaudere ut summo bono, dolorem aspernari ut summum malum et, quantum possit, a se repellere, idque facere nondum depravatum ipsa natura incorrupte atque integre iudicante. itaque negat opus esse ratione neque disputatione, quam ob rem voluptas expetenda, fugiendus dolor sit. sentiri haec putat, ut calere ignem, nivem esse albam, dulce mel. quorum nihil oportere exquisitis rationibus confirmare, tantum satis esse admonere. interesse enim inter argumentum conclusionemque rationis et inter mediocrem animadversionem atque admonitionem. altera occulta quaedam et quasi involuta aperiri, altera prompta et aperta iudicari. etenim quoniam detractis de homine sensibus reliqui nihil est, necesse est, quid aut ad naturam aut contra sit, a natura ipsa iudicari. ea quid percipit aut quid iudicat, quo aut petat aut fugiat aliquid, praeter voluptatem et dolorem? Sunt autem quidam e nostris, qui haec subtilius velint tradere et negent satis esse, quid bonum sit aut quid malum, sensu iudicari, sed animo etiam ac ratione intellegi posse et voluptatem ipsam per se esse expetendam et dolorem ipsum per se esse fugiendum. itaque aiunt hanc quasi naturalem atque insitam in animis nostris inesse notionem, ut alterum esse appetendum, alterum aspernandum sentiamus. Alii autem, quibus ego assentior, cum a philosophis compluribus permulta dicantur, cur nec voluptas in bonis sit numeranda nec in malis dolor, non existimant oportere nimium nos causae confidere, sed et argumentandum et accurate disserendum et rationibus conquisitis de voluptate et dolore disputandum putant. Sed ut perspiciatis, unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam eaque ipsa, quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt, explicabo. nemo enim ipsam voluptatem, quia voluptas sit, aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos, qui ratione voluptatem sequi nesciunt, neque porro quisquam est, qui dolorem ipsum, quia dolor sit, amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt, ut labore et dolore magnam aliquam quaerat voluptatem. ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? quis autem vel eum iure reprehenderit, qui in ea voluptate velit esse, quam nihil molestiae consequatur, vel illum, qui dolorem eum fugiat, quo voluptas nulla pariatur? At vero eos et accusamus et iusto odio dignissimos ducimus, qui blanditiis praesentium voluptatum deleniti atque corrupti, quos dolores et quas molestias excepturi sint, obcaecati cupiditate non provident, similique sunt in culpa, qui officia deserunt mollitia animi, id est laborum et dolorum fuga. et harum quidem rerum facilis est et expedita distinctio. nam libero tempore, cum soluta nobis est eligendi optio, cumque nihil impedit, quo minus id, quod maxime placeat, facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet, ut et voluptates repudiandae sint et molestiae non recusandae. itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus "
        "asperiores repellat."
        + std::to_string(i);
    }
  }

  auto matchDataPromise = std::promise<void>();

  test1.listener.setMatchDataCallback(
      [&test1, &test2, &matchDataPromise](const NMatchData& matchData) {
  NLOG_INFO("TEST DEBUG 4.");
        matchDataPromise.set_value();
  NLOG_INFO("TEST DEBUG 5.");
      });
  test2.listener.setMatchDataCallback(
      [&test1, &test2, &matchDataPromise](const NMatchData& matchData) {
  NLOG_INFO("TEST DEBUG 4b.");
        matchDataPromise.set_value();
  NLOG_INFO("TEST DEBUG 5b.");
      });

  NLOG_INFO("Send match data timing start.");

  std::chrono::time_point<chrono::steady_clock> timer_begin = std::chrono::high_resolution_clock::now();
  NLOG_INFO("TEST DEBUG 1.");
  test1.rtClient->sendMatchDataAsync(matched->matchId,101, testMessages[0]).get();
  NLOG_INFO("TEST DEBUG 2.");
  //std::future<void> matchDataSendFinish2 = test1.rtClient->sendMatchDataAsync(matched->matchId,101, testMessages[1]);//.get();
  //std::future<void> matchDataSendFinish3 = test1.rtClient->sendMatchDataAsync(matched->matchId,101, testMessages[2]);//.get();
  //matchDataSendFinish2.get();
  //matchDataSendFinish3.get();
  //matchDataPromise.get_future().get();
  NLOG_INFO("TEST DEBUG 3.");
  std::chrono::time_point<chrono::steady_clock> timer_end = std::chrono::high_resolution_clock::now();

  NLOG_INFO("Send match data time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_begin).count()) + "ms.");

  test1.stopTest(true);
  test2.stopTest(true);
}

void test_rt_throughput_with_tick_interval(int tickIntervalMs) {
  const int NUM_CHUNKS = 3;
  const int CHUNK_SIZE = 1 * 1024 * 1024; // 1 MB
  const int TOTAL_BYTES = NUM_CHUNKS * CHUNK_SIZE;
  std::string label = "throughput_tick_" + std::to_string(tickIntervalMs) + "ms";

  NLOG_INFO("=== Throughput benchmark (tick=" + std::to_string(tickIntervalMs) + "ms) ===");

  bool threadedTick = true;
  NTest testSender(label + "_sender", threadedTick);
  NTest testReceiver(label + "_receiver", threadedTick);
  testSender.setTickIntervalMs(tickIntervalMs);
  testReceiver.setTickIntervalMs(tickIntervalMs);
  testSender.setTestTimeoutMs(60000);
  testReceiver.setTestTimeoutMs(60000);
  testSender.runTest();
  testReceiver.runTest();

  // Authenticate and connect both
  NSessionPtr senderSession = testSender.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  NSessionPtr receiverSession = testReceiver.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  bool createStatus = false;
  testSender.rtClient->connectAsync(senderSession, createStatus, NTest::RtProtocol).get();
  testReceiver.rtClient->connectAsync(receiverSession, createStatus, NTest::RtProtocol).get();

  // Sender creates match, receiver joins
  NMatch match = testSender.rtClient->createMatchAsync().get();
  std::string matchId = match.matchId;
  testReceiver.rtClient->joinMatchAsync(matchId, {}).get();

  // Prepare 3 x 1MB chunks
  std::string chunk(CHUNK_SIZE, 'X');

  // Track received chunks
  std::mutex mtx;
  std::condition_variable cv;
  std::atomic<int> chunksReceived{0};

  testReceiver.listener.setMatchDataCallback(
      [&](const NMatchData& matchData) {
        int count = chunksReceived.fetch_add(1) + 1;
        if (count >= NUM_CHUNKS) {
          std::lock_guard<std::mutex> lock(mtx);
          cv.notify_all();
        }
      });

  // Send 3 chunks in parallel and measure wall-clock time until all received
  auto t0 = std::chrono::high_resolution_clock::now();

  std::future<void> f1 = testSender.rtClient->sendMatchDataAsync(matchId, 1, chunk);
  std::future<void> f2 = testSender.rtClient->sendMatchDataAsync(matchId, 2, chunk);
  std::future<void> f3 = testSender.rtClient->sendMatchDataAsync(matchId, 3, chunk);
  f1.get();
  f2.get();
  f3.get();

  auto tSent = std::chrono::high_resolution_clock::now();

  // Wait for receiver to get all chunks
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, std::chrono::seconds(30), [&]{ return chunksReceived.load() >= NUM_CHUNKS; });
  }

  auto tRecv = std::chrono::high_resolution_clock::now();

  long long sendMs = std::chrono::duration_cast<std::chrono::milliseconds>(tSent - t0).count();
  long long totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(tRecv - t0).count();
  double throughputMBs = (totalMs > 0) ? (double)TOTAL_BYTES / (1024.0 * 1024.0) / ((double)totalMs / 1000.0) : 0.0;

  // Format throughput to 1 decimal place
  char throughputStr[32];
  snprintf(throughputStr, sizeof(throughputStr), "%.1f", throughputMBs);

  NLOG_INFO("=== Throughput results (tick=" + std::to_string(tickIntervalMs) + "ms) ==="
            + " Send: " + std::to_string(sendMs) + "ms"
            + " Total: " + std::to_string(totalMs) + "ms"
            + " Throughput: " + throughputStr + " MB/s"
            + " Chunks received: " + std::to_string(chunksReceived.load()));

  testSender.stopTest(true);
  testReceiver.stopTest(true);
}

void test_rt_hotjoin_with_tick_interval(int tickIntervalMs) {
  const int NUM_ITERATIONS = 5;
  std::string label = "hotjoin_tick_" + std::to_string(tickIntervalMs) + "ms";

  NLOG_INFO("=== Hot-join benchmark (tick=" + std::to_string(tickIntervalMs) + "ms) ===");

  bool threadedTick = true;
  NTest testHost(label + "_host", threadedTick);
  testHost.setTickIntervalMs(tickIntervalMs);
  testHost.setTestTimeoutMs(30000);
  testHost.runTest();

  NSessionPtr hostSession = testHost.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  bool createStatus = false;
  testHost.rtClient->connectAsync(hostSession, createStatus, NTest::RtProtocol).get();

  NMatch match = testHost.rtClient->createMatchAsync().get();
  std::string matchId = match.matchId;

  long long totalJoinMs = 0;
  long long minJoinMs = std::numeric_limits<long long>::max();
  long long maxJoinMs = 0;

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    NTest testJoiner(label + "_joiner_" + std::to_string(i), threadedTick);
    testJoiner.setTickIntervalMs(tickIntervalMs);
    testJoiner.setTestTimeoutMs(20000);
    testJoiner.runTest();

    NSessionPtr joinerSession = testJoiner.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
    testJoiner.rtClient->connectAsync(joinerSession, createStatus, NTest::RtProtocol).get();

    auto t0 = std::chrono::high_resolution_clock::now();
    NMatch joinedMatch = testJoiner.rtClient->joinMatchAsync(matchId, {}).get();
    auto t1 = std::chrono::high_resolution_clock::now();

    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    totalJoinMs += ms;
    if (ms < minJoinMs) minJoinMs = ms;
    if (ms > maxJoinMs) maxJoinMs = ms;

    NLOG_INFO("  [" + std::to_string(tickIntervalMs) + "ms tick] Iteration " + std::to_string(i + 1)
              + ": joinMatch = " + std::to_string(ms) + "ms");

    testJoiner.rtClient->leaveMatchAsync(matchId).get();
    testJoiner.stopTest(true);
  }

  NLOG_INFO("=== Results (tick=" + std::to_string(tickIntervalMs) + "ms) ==="
            + " Min: " + std::to_string(minJoinMs) + "ms"
            + " Max: " + std::to_string(maxJoinMs) + "ms"
            + " Avg: " + std::to_string(totalJoinMs / NUM_ITERATIONS) + "ms");

  testHost.stopTest(true);
}

void test_rt_concurrent_tick() {
  const int NUM_TICK_THREADS = 4;
  const int NUM_SENDS = 20;
  const int CHUNK_SIZE = 64 * 1024; // 64 KB per message

  NLOG_INFO("=== Concurrent tick() stress test ===");
  NLOG_INFO("Threads: " + std::to_string(NUM_TICK_THREADS)
            + ", Sends: " + std::to_string(NUM_SENDS)
            + ", Chunk: " + std::to_string(CHUNK_SIZE / 1024) + " KB");

  // Create sender and receiver with NO threaded tick — we'll tick manually from multiple threads
  NTest testSender("concurrent_tick_sender", false);
  NTest testReceiver("concurrent_tick_receiver", false);
  testSender.setTestTimeoutMs(30000);
  testReceiver.setTestTimeoutMs(30000);

  // Authenticate (tick manually during async waits)
  auto senderAuthFuture = testSender.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true);
  while (senderAuthFuture.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
    testSender.client->tick();
  }
  NSessionPtr senderSession = senderAuthFuture.get();

  auto receiverAuthFuture = testReceiver.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true);
  while (receiverAuthFuture.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
    testReceiver.client->tick();
  }
  NSessionPtr receiverSession = receiverAuthFuture.get();

  // Connect RT (tick both clients during connect)
  bool createStatus = false;
  auto senderConnFuture = testSender.rtClient->connectAsync(senderSession, createStatus, NTest::RtProtocol);
  while (senderConnFuture.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
    testSender.tick();
  }
  senderConnFuture.get();

  auto receiverConnFuture = testReceiver.rtClient->connectAsync(receiverSession, createStatus, NTest::RtProtocol);
  while (receiverConnFuture.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
    testReceiver.tick();
  }
  receiverConnFuture.get();

  // Create match and join
  auto createFuture = testSender.rtClient->createMatchAsync();
  while (createFuture.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
    testSender.tick();
  }
  NMatch match = createFuture.get();
  std::string matchId = match.matchId;

  auto joinFuture = testReceiver.rtClient->joinMatchAsync(matchId, {});
  while (joinFuture.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
    testReceiver.tick();
    testSender.tick();
  }
  joinFuture.get();

  NLOG_INFO("Both connected and in match: " + matchId);

  // Track received messages
  std::atomic<int> chunksReceived{0};
  std::atomic<int> errors{0};
  std::mutex cvMtx;
  std::condition_variable cv;

  testReceiver.listener.setMatchDataCallback(
      [&](const NMatchData& matchData) {
        int count = chunksReceived.fetch_add(1) + 1;
        if (count >= NUM_SENDS) {
          std::lock_guard<std::mutex> lock(cvMtx);
          cv.notify_all();
        }
      });

  // Prepare chunk
  std::string chunk(CHUNK_SIZE, 'Z');

  // Launch N threads that all call tick() concurrently on the SAME rtClient + client
  std::atomic<bool> running{true};
  std::vector<std::thread> tickThreads;

  for (int t = 0; t < NUM_TICK_THREADS; t++) {
    tickThreads.emplace_back([&, t]() {
      NLOG_INFO("  Tick thread " + std::to_string(t) + " started");
      while (running.load()) {
        testSender.tick();
        testReceiver.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      NLOG_INFO("  Tick thread " + std::to_string(t) + " stopped");
    });
  }

  // Send messages from the main thread while tick threads are running
  auto t0 = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < NUM_SENDS; i++) {
    try {
      testSender.rtClient->sendMatchData(matchId, 100 + i, chunk);
    } catch (const std::exception& e) {
      errors.fetch_add(1);
      NLOG_INFO("  Send error on message " + std::to_string(i) + ": " + e.what());
    }
  }

  // Wait for receiver to get all chunks (or timeout)
  bool allReceived = false;
  {
    std::unique_lock<std::mutex> lock(cvMtx);
    allReceived = cv.wait_for(lock, std::chrono::seconds(15),
                               [&]{ return chunksReceived.load() >= NUM_SENDS; });
  }

  auto t1 = std::chrono::high_resolution_clock::now();

  // Stop tick threads
  running.store(false);
  for (auto& t : tickThreads) {
    t.join();
  }

  long long totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

  NLOG_INFO("=== Concurrent tick() results ===");
  NLOG_INFO("  Threads:  " + std::to_string(NUM_TICK_THREADS));
  NLOG_INFO("  Sent:     " + std::to_string(NUM_SENDS));
  NLOG_INFO("  Received: " + std::to_string(chunksReceived.load()) + "/" + std::to_string(NUM_SENDS));
  NLOG_INFO("  Errors:   " + std::to_string(errors.load()));
  NLOG_INFO("  Time:     " + std::to_string(totalMs) + "ms");
  NLOG_INFO("  Status:   " + std::string(allReceived ? "ALL RECEIVED" : "INCOMPLETE/TIMEOUT"));

  testSender.stopTest(allReceived && errors.load() == 0);
  testReceiver.stopTest(allReceived && errors.load() == 0);
}

void test_rt_hotjoin() {
  const int tickRates[] = {1, 2, 5, 10, 16, 20, 50};

  NLOG_INFO("=== Throughput tick rate correlation test (3 x 1MB parallel) ===");
  for (int tickMs : tickRates) {
    test_rt_throughput_with_tick_interval(tickMs);
  }

  NLOG_INFO("=== Hot-join tick rate correlation test ===");
  for (int tickMs : tickRates) {
    test_rt_hotjoin_with_tick_interval(tickMs);
  }

  NLOG_INFO("=== All tick rate correlation tests complete ===");
}

void test_rt_match() {
  //test_rt_create_match();
  //test_rt_matchmaker();
  //test_rt_sendmatchdata_throughput();
  test_rt_hotjoin();
  // Disabled by default because it intentionally pushes websocket limits
  // and can terminate the process when large payloads exceed server/client bounds.
  // test_rt_concurrent_tick();
}

} // namespace Test
} // namespace Nakama
