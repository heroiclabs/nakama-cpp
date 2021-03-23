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

#include "test_main.h"

namespace Nakama {
namespace Test {

using namespace std;

void test_writeStorageInvalidArgument()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        std::cout << "Authenticated successfully" << std::endl;

        std::vector<NStorageObjectWrite> objects;
        NStorageObjectWrite obj;

        obj.collection = "candies";
        obj.key = "test";
        obj.value = "25";

        objects.push_back(obj);

        auto errorCallback = [&test](const NError& error)
        {
            test.stopTest(error.code == ErrorCode::InvalidArgument);
        };

        test.client->writeStorageObjects(session, objects, nullptr, errorCallback);
    };

    test.client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

    test.runTest();
}

void test_writeStorage()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        auto writeSuccessCallback = [&test, session](const NStorageObjectAcks& acks)
        {
            if (acks.size() == 1)
            {
                std::cout << "write ok. version: " << acks[0].version << std::endl;

                auto successCallback = [&test](NStorageObjectListPtr list)
                {
                    std::cout << "objects count: " << list->objects.size() << std::endl;

                    test.stopTest(list->objects.size() > 0);
                };

                test.client->listUsersStorageObjects(session, "candies", session->getUserId(), {}, {}, successCallback);
            }
            else
            {
                test.stopTest();
            }
        };

        std::vector<NStorageObjectWrite> objects;
        NStorageObjectWrite obj;

        obj.collection = "candies";
        obj.key = "Ice cream";
        obj.value = "{ \"price\": 25 }";
        obj.permissionRead = NStoragePermissionRead::OWNER_READ;
        obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;

        objects.push_back(obj);

        test.client->writeStorageObjects(session, objects, writeSuccessCallback);
    };

    test.client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

    test.runTest();
}


void test_writeStorageCursor()
{
    NCppTest test(__func__);

    test.createWorkingClient();

    auto successCallback = [&test](NSessionPtr session)
    {
        int numCandies = 25;

        auto writeSuccessCallback = [&test, session, numCandies](const NStorageObjectAcks& acks)
        {
            if (acks.size() == numCandies)
            {
                std::cout << "write ok. version: " << acks[0].version << std::endl;

                auto firstListCallback = [&test, session](NStorageObjectListPtr list)
                {
                    std::cout << "cursor : " << list->cursor << std::endl;

                    auto secondListCallback = [&test, session](NStorageObjectListPtr list)
                    {
                        test.stopTest(list->objects.size() > 0);
                    };

                    test.client->listUsersStorageObjects(session, "candies", session->getUserId(), 10, list->cursor, secondListCallback);
                };

                test.client->listUsersStorageObjects(session, "candies", session->getUserId(), 10, {}, firstListCallback);
            }
            else
            {
                test.stopTest();
            }
        };

        std::vector<NStorageObjectWrite> objects;

        for (int i = 0; i < numCandies; i++)
        {
            NStorageObjectWrite obj;
            obj.collection = "candies";
            obj.key = "Ice cream " + std::to_string(i);
            obj.value = "{ \"price\": 25 }";
            obj.permissionRead = NStoragePermissionRead::OWNER_READ;
            obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
            objects.push_back(obj);
        }

        test.client->writeStorageObjects(session, objects, writeSuccessCallback);
    };

    test.client->authenticateDevice("mytestdevice0000", opt::nullopt, true, {}, successCallback);

    test.runTest();
}

void test_storage()
{
    test_writeStorageInvalidArgument();
    test_writeStorage();
    test_writeStorageCursor();

}

} // namespace Test
} // namespace Nakama
