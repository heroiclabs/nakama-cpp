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
            std::cout << "Received notifications: " << list.notifications.size() << std::endl;

            auto errorCallback = [&test](const NError& error)
            {
                test.stopTest();
            };

            auto removedNotificationCallback = [&test, errorCallback]()
            {
                std::cout << "Notification removed." << std::endl;
                test.stopTest(true);
            };

            for (auto& notification : list.notifications)
            {
                std::cout << "Notification code: " << notification.code << std::endl;
                std::cout << "\tcontent: " << notification.content << std::endl;

                test.client->deleteNotifications(
                    test.session,
                    { notification.id },
                    removedNotificationCallback,
                    errorCallback);
            }
        });

        auto errorCallback = [&test](const NRtError& error)
        {
            test.stopTest();
        };

        auto successCallback = [&test, errorCallback](const NRpc& rpc)
        {
            std::cout << "rpc response: " << rpc.payload << std::endl;
        };

        test.rtClient->rpc(
            "clientrpc.send_notification",
            "{\"user_id\":\"" + test.session->getUserId() + "\"}",
            successCallback,
            errorCallback);
    };

    test.runTest();
}

void test_createListAndDeleteNotifications()
{
    NRtClientTest test(__func__);

    test.onRtConnect = [&test]()
    {
        auto errorCallback = [&test](const NRtError& error)
        {
            test.stopTest();
        };

        auto successCallback = [&test](const NRpc& rpc)
        {
            std::cout << "rpc response: " << rpc.payload << std::endl;

            auto errorCallback = [&test](const NError& error)
            {
                test.stopTest();
            };

            auto listCallback = [&test, errorCallback](NNotificationListPtr list)
            {
                if (list->notifications.size() > 0)
                {
                    auto removedNotificationCallback = [&test, errorCallback]()
                    {
                        std::cout << "Notification removed." << std::endl;
                        test.stopTest(true);
                    };

                    for (auto& notification : list->notifications)
                    {
                        std::cout << "Notification code: " << notification.code << std::endl;
                        std::cout << "\tcontent: " << notification.content << std::endl;

                        test.client->deleteNotifications(
                            test.session,
                            { notification.id },
                            removedNotificationCallback,
                            errorCallback);
                    }
                }
                else
                    test.stopTest();
            };

            test.client->listNotifications(test.session,
                opt::nullopt,
                opt::nullopt,
                listCallback,
                errorCallback);
        };

        test.rtClient->rpc(
            "clientrpc.send_notification",
            "{\"user_id\":\"" + test.session->getUserId() + "\"}",
            successCallback,
            errorCallback);
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
