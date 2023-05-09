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

#include "nakama-cpp/log/NLogger.h"
#include "NTest.h"
#include "TestGuid.h"

namespace Nakama {
namespace Test {

void test_createAndDeleteNotifications()
{
    bool threadedTick = true;
    NTest test(__func__, threadedTick);
    test.runTest();
    NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
    bool createStatus = false;
    test.rtClient->connectAsync(session, createStatus, NRtClientProtocol::Json).get();

    test.listener.setNotificationsCallback([&test, session](const NNotificationList& list)
    {
        NLOG_INFO("Received notifications: " + std::to_string(list.notifications.size()));

        for (auto& notification : list.notifications)
        {
            NLOG_INFO("Notification code: " + std::to_string(notification.code));
            NLOG_INFO("\tcontent: " + notification.content);

            test.client->deleteNotificationsAsync(session, {notification.id}).get();
            test.stopTest(true);
        }


        test.rtClient->rpcAsync("clientrpc.send_notification", "{\"user_id\":\"" + session->getUserId() + "\"}");
    });

    test.waitUntilStop();
}

void test_createListAndDeleteNotifications()
{
    bool threadedTick = true;
    NTest test(__func__, threadedTick);
    test.runTest();
    NSessionPtr session = test.client->authenticateCustomAsync(TestGuid::newGuid(), std::string(), true).get();
    bool createStatus = false;
    test.rtClient->connectAsync(session, createStatus, NRtClientProtocol::Json).get();
    const Nakama::NRpc& rpc = test.rtClient->rpcAsync("clientrpc.send_notification", "{\"user_id\":\"" + session->getUserId() + "\"}").get();
    NNotificationListPtr list = test.client->listNotificationsAsync(session, opt::nullopt, opt::nullopt).get();
    if (list->notifications.size() > 0)
    {
        for (auto& notification : list->notifications)
        {
            NLOG_INFO("Notification code: " + std::to_string(notification.code));
            NLOG_INFO("\tcontent: " + notification.content);
            test.client->deleteNotificationsAsync(session, {notification.id});
            test.stopTest(true);
        }
    }
    else
    {
        test.stopTest();
    }
}

void test_notifications()
{
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
