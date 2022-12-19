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

#include "realtime/RtClientTestBase.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_createAndDeleteNotifications()
{
    NRtClientTest test(__func__);

    test.onRtConnect = [&test]()
    {
        test.listener.setNotificationsCallback([&test](const NNotificationList& list)
        {
            NLOG_INFO("Received notifications: " + std::to_string(list.notifications.size()));

            auto removedNotificationCallback = [&test]()
            {
                NLOG_INFO("Notification removed.");
                test.stopTest(true);
            };

            for (auto& notification : list.notifications)
            {
                NLOG_INFO("Notification code: " + std::to_string(notification.code));
                NLOG_INFO("\tcontent: " + notification.content);

                test.client->deleteNotifications(
                    test.session,
                    { notification.id },
                    removedNotificationCallback);
            }
        });

        auto successCallback = [](const NRpc& rpc)
        {
            NLOG_INFO("rpc response: " + rpc.payload);
        };

        test.rtClient->rpc(
            "clientrpc.send_notification",
            "{\"user_id\":\"" + test.session->getUserId() + "\"}",
            successCallback);
    };

    test.runTest();
}

void test_createListAndDeleteNotifications()
{
    NRtClientTest test(__func__);

    test.onRtConnect = [&test]()
    {
        auto successCallback = [&test](const NRpc& rpc)
        {
            NLOG_INFO("rpc response: " + rpc.payload);

            auto listCallback = [&test](NNotificationListPtr list)
            {
                if (list->notifications.size() > 0)
                {
                    auto removedNotificationCallback = [&test]()
                    {
                        NLOG_INFO("Notification removed.");
                        test.stopTest(true);
                    };

                    for (auto& notification : list->notifications)
                    {
                        NLOG_INFO("Notification code: " + std::to_string(notification.code));
                        NLOG_INFO("\tcontent: " + notification.content);

                        test.client->deleteNotifications(
                            test.session,
                            { notification.id },
                            removedNotificationCallback);
                    }
                }
                else
                    test.stopTest();
            };

            test.client->listNotifications(test.session,
                opt::nullopt,
                opt::nullopt,
                listCallback);
        };

        test.rtClient->rpc(
            "clientrpc.send_notification",
            "{\"user_id\":\"" + test.session->getUserId() + "\"}",
            successCallback);
    };

    test.runTest();
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
