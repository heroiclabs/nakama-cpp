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
#include <chrono>

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

void test_rt_match() {
  //test_rt_create_match();
  //test_rt_matchmaker();
  test_rt_sendmatchdata_throughput();
}

} // namespace Test
} // namespace Nakama
