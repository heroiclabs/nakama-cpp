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

#include <optional>

namespace Nakama {
namespace Test {

void test_createAndDeleteNotifications() {
  bool threadedTick = true;
  NTest test(__func__, threadedTick);
  test.runTest();
  NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  test.addSession(session);
  bool createStatus = false;
  test.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();

  std::promise<void> notificationsPromise;
  test.listener.setNotificationsCallback([&test, session, &notificationsPromise](const NNotificationList& list) {
    NLOG_INFO("Received notifications: " + std::to_string(list.notifications.size()));

    for (auto& notification : list.notifications) {
      NLOG_INFO("Notification code: " + std::to_string(notification.code));
      NLOG_INFO("\tcontent: " + notification.content);
    }

    notificationsPromise.set_value();
  });

  try {
    test.rtClient->rpcAsync("clientrpc.send_notification", "{\"user_id\":\"" + session->getUserId() + "\"}").get();
  } catch (const std::exception& e) {
    NLOG_INFO("rpcAsync failed: " + std::string(e.what()));
    test.stopTest(false);
    return;
  }

  notificationsPromise.get_future().get();
  test.stopTest(true);
}

void test_createListAndDeleteNotifications() {
  bool threadedTick = true;
  NTest test(__func__, threadedTick);
  test.runTest();
  NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
  test.addSession(session);
  bool createStatus = false;
  test.rtClient->connectAsync(session, createStatus, NTest::RtProtocol).get();

  // Wait for the notification to arrive via the RT listener before listing
  std::promise<void> notificationArrived;
  test.listener.setNotificationsCallback([&notificationArrived](const NNotificationList&) {
    notificationArrived.set_value();
  });

  try {
    test.rtClient->rpcAsync("clientrpc.send_notification", "{\"user_id\":\"" + session->getUserId() + "\"}").get();
  } catch (const std::exception& e) {
    NLOG_INFO("rpcAsync failed: " + std::string(e.what()));
    test.stopTest(false);
    return;
  }

  notificationArrived.get_future().get();

  NNotificationListPtr list = test.client->listNotificationsAsync(session, std::nullopt, std::nullopt).get();
  if (list->notifications.size() > 0) {
    for (auto& notification : list->notifications) {
      NLOG_INFO("Notification code: " + std::to_string(notification.code));
      NLOG_INFO("\tcontent: " + notification.content);
      test.client->deleteNotificationsAsync(session, {notification.id}).get();
    }
    test.stopTest(true);
  } else {
    test.stopTest(false);
  }
}

void test_notifications() {
  // Notifications tests reqire following lua modules:
  // download clientrpc.lua and debug_utils.lua from
  // https://github.com/heroiclabs/nakama/tree/master/data/modules
  // put to nakama-server/data/modules
  // restart server

  test_createAndDeleteNotifications();
  test_createListAndDeleteNotifications();
}

} // namespace Test
} // namespace Nakama
