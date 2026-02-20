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

#include <nakama-cpp/NException.h>
#include <optional>

namespace Nakama {
namespace Test {

using namespace std;

void test_writeStorageInvalidArgument() {
  NTest test(__func__);

  auto successCallback = [&test](NSessionPtr session) {
    std::vector<NStorageObjectWrite> objects;
    NStorageObjectWrite obj;

    obj.collection = "candies";
    obj.key = "test";
    obj.value = "25";

    objects.push_back(obj);

    auto errorCallback = [&test](const NError& error) { test.stopTest(error.code == ErrorCode::InvalidArgument); };

    test.client->writeStorageObjects(session, objects, nullptr, errorCallback);
  };

  test.client->authenticateDevice("mytestdevice0000", std::nullopt, true, {}, successCallback);

  test.runTest();
}

void test_writeStorage() {
  NTest test(__func__);

  auto successCallback = [&test](NSessionPtr session) {
    auto writeSuccessCallback = [&test, session](const NStorageObjectAcks& acks) {
      if (acks.size() == 1) {
        NLOG_INFO("write ok. version: " + acks[0].version);

        auto successCallback = [&test](NStorageObjectListPtr list) {
          NLOG_INFO("objects count: " + std::to_string(list->objects.size()));

          test.stopTest(list->objects.size() > 0);
        };

        test.client->listUsersStorageObjects(session, "candies", session->getUserId(), {}, {}, successCallback);
      } else {
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

  test.client->authenticateDevice("mytestdevice0000", std::nullopt, true, {}, successCallback);

  test.runTest();
}

void test_writeStorageCursor() {
  NTest test(__func__);

  auto successCallback = [&test](NSessionPtr session) {
    size_t numCandies = 25;

    auto writeSuccessCallback = [&test, session, numCandies](const NStorageObjectAcks& acks) {
      if (acks.size() == numCandies) {
        NLOG_INFO("write ok. version: " + acks[0].version);

        auto firstListCallback = [&test, session](NStorageObjectListPtr list) {
          NLOG_INFO("cursor : " + list->cursor);

          auto secondListCallback = [&test, session](NStorageObjectListPtr list) {
            test.stopTest(list->objects.size() > 0);
          };

          test.client->listUsersStorageObjects(
              session, "candies", session->getUserId(), 10, list->cursor, secondListCallback);
        };

        test.client->listUsersStorageObjects(session, "candies", session->getUserId(), 10, {}, firstListCallback);
      } else {
        test.stopTest();
      }
    };

    std::vector<NStorageObjectWrite> objects;

    for (size_t i = 0; i < numCandies; i++) {
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

  test.client->authenticateDevice("mytestdevice0000", std::nullopt, true, {}, successCallback);

  test.runTest();
}

void test_readStorageObjects() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    string userId = session->getUserId();

    vector<NStorageObjectWrite> objects;
    NStorageObjectWrite obj;
    obj.collection = "test_collection";
    obj.key = "test_key";
    obj.value = "{ \"data\": \"hello\" }";
    obj.permissionRead = NStoragePermissionRead::OWNER_READ;
    obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
    objects.push_back(obj);

    test.client->writeStorageObjectsAsync(session, objects).get();

    vector<NReadStorageObjectId> readIds;
    NReadStorageObjectId readId;
    readId.collection = "test_collection";
    readId.key = "test_key";
    readId.userId = userId;
    readIds.push_back(readId);

    NStorageObjects result = test.client->readStorageObjectsAsync(session, readIds).get();
    // Nakama normalizes JSON whitespace, so compare against the compact form
    test.stopTest(result.size() == 1 && result[0].value.find("hello") != string::npos);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_deleteStorageObjects() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    vector<NStorageObjectWrite> objects;
    NStorageObjectWrite obj;
    obj.collection = "test_collection";
    obj.key = "to_delete";
    obj.value = "{ \"temp\": true }";
    obj.permissionRead = NStoragePermissionRead::OWNER_READ;
    obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
    objects.push_back(obj);

    NStorageObjectAcks acks = test.client->writeStorageObjectsAsync(session, objects).get();

    vector<NDeleteStorageObjectId> deleteIds;
    NDeleteStorageObjectId deleteId;
    deleteId.collection = "test_collection";
    deleteId.key = "to_delete";
    deleteId.version = acks[0].version;
    deleteIds.push_back(deleteId);

    test.client->deleteStorageObjectsAsync(session, deleteIds).get();

    auto list =
        test.client->listUsersStorageObjectsAsync(session, "test_collection", session->getUserId()).get();
    test.stopTest(list->objects.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_writeStorage_invalidJson() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    vector<NStorageObjectWrite> objects;
    NStorageObjectWrite obj;
    obj.collection = "test_collection";
    obj.key = "bad_json";
    obj.value = "not valid json";
    objects.push_back(obj);

    test.client->writeStorageObjectsAsync(session, objects).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed with unexpected exception: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_writeStorageMultiple() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    string collection = "multi_" + TestGuid::newGuid();

    vector<NStorageObjectWrite> objects;
    for (int i = 0; i < 3; i++) {
      NStorageObjectWrite obj;
      obj.collection = collection;
      obj.key = "key_" + to_string(i);
      obj.value = "{ \"index\": " + to_string(i) + " }";
      obj.permissionRead = NStoragePermissionRead::OWNER_READ;
      obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
      objects.push_back(obj);
    }

    NStorageObjectAcks acks = test.client->writeStorageObjectsAsync(session, objects).get();
    test.stopTest(acks.size() == 3);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_writeStorage_emptyCollection() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    vector<NStorageObjectWrite> objects;
    NStorageObjectWrite obj;
    obj.collection = "";
    obj.key = "test_key";
    obj.value = "{ \"data\": 1 }";
    objects.push_back(obj);

    test.client->writeStorageObjectsAsync(session, objects).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_writeStorage_emptyKey() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    vector<NStorageObjectWrite> objects;
    NStorageObjectWrite obj;
    obj.collection = "test_collection";
    obj.key = "";
    obj.value = "{ \"data\": 1 }";
    objects.push_back(obj);

    test.client->writeStorageObjectsAsync(session, objects).get();
    test.stopTest(false);
  } catch (const NException& e) {
    test.stopTest(e.error.code == ErrorCode::InvalidArgument);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_writeStorage_publicRead() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    string collection = "public_" + TestGuid::newGuid();

    vector<NStorageObjectWrite> objects;
    NStorageObjectWrite obj;
    obj.collection = collection;
    obj.key = "public_key";
    obj.value = "{ \"visible\": true }";
    obj.permissionRead = NStoragePermissionRead::PUBLIC_READ;
    obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
    objects.push_back(obj);

    test.client->writeStorageObjectsAsync(session, objects).get();

    // Second user reads the public object
    NTest test2("test_writeStorage_publicRead_reader", true);
    test2.runTest();
    auto session2 = test2.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test2.addSession(session2);

    auto list = test2.client->listStorageObjectsAsync(session2, collection).get();
    bool found = false;
    for (const auto& o : list->objects) {
      if (o.key == "public_key") {
        found = true;
        break;
      }
    }
    test2.stopTest(found);
    test.stopTest(found);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_readStorage_nonExistent() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);

    vector<NReadStorageObjectId> readIds;
    NReadStorageObjectId readId;
    readId.collection = "nonexistent_collection_" + TestGuid::newGuid();
    readId.key = "nonexistent_key";
    readId.userId = session->getUserId();
    readIds.push_back(readId);

    NStorageObjects result = test.client->readStorageObjectsAsync(session, readIds).get();
    test.stopTest(result.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_readStorage_multiple() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    string userId = session->getUserId();
    string collection = "readmulti_" + TestGuid::newGuid();

    vector<NStorageObjectWrite> objects;
    for (int i = 0; i < 2; i++) {
      NStorageObjectWrite obj;
      obj.collection = collection;
      obj.key = "key_" + to_string(i);
      obj.value = "{ \"i\": " + to_string(i) + " }";
      obj.permissionRead = NStoragePermissionRead::OWNER_READ;
      obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
      objects.push_back(obj);
    }

    test.client->writeStorageObjectsAsync(session, objects).get();

    vector<NReadStorageObjectId> readIds;
    for (int i = 0; i < 2; i++) {
      NReadStorageObjectId readId;
      readId.collection = collection;
      readId.key = "key_" + to_string(i);
      readId.userId = userId;
      readIds.push_back(readId);
    }

    NStorageObjects result = test.client->readStorageObjectsAsync(session, readIds).get();
    test.stopTest(result.size() == 2);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_deleteStorage_multiple() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    string collection = "delmulti_" + TestGuid::newGuid();

    vector<NStorageObjectWrite> objects;
    for (int i = 0; i < 2; i++) {
      NStorageObjectWrite obj;
      obj.collection = collection;
      obj.key = "key_" + to_string(i);
      obj.value = "{ \"i\": " + to_string(i) + " }";
      obj.permissionRead = NStoragePermissionRead::OWNER_READ;
      obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
      objects.push_back(obj);
    }

    NStorageObjectAcks acks = test.client->writeStorageObjectsAsync(session, objects).get();

    vector<NDeleteStorageObjectId> deleteIds;
    for (int i = 0; i < 2; i++) {
      NDeleteStorageObjectId deleteId;
      deleteId.collection = collection;
      deleteId.key = "key_" + to_string(i);
      deleteId.version = acks[i].version;
      deleteIds.push_back(deleteId);
    }

    test.client->deleteStorageObjectsAsync(session, deleteIds).get();

    auto list = test.client->listUsersStorageObjectsAsync(session, collection, session->getUserId()).get();
    test.stopTest(list->objects.empty());
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_listStorage_withLimit() {
  NTest test(__func__, true);
  test.runTest();

  try {
    auto session = test.client->authenticateCustomAsync(TestGuid::newGuid(), "", true).get();
    test.addSession(session);
    string collection = "limitcol_" + TestGuid::newGuid();

    vector<NStorageObjectWrite> objects;
    for (int i = 0; i < 5; i++) {
      NStorageObjectWrite obj;
      obj.collection = collection;
      obj.key = "key_" + to_string(i);
      obj.value = "{ \"i\": " + to_string(i) + " }";
      obj.permissionRead = NStoragePermissionRead::OWNER_READ;
      obj.permissionWrite = NStoragePermissionWrite::OWNER_WRITE;
      objects.push_back(obj);
    }

    test.client->writeStorageObjectsAsync(session, objects).get();

    auto list = test.client->listUsersStorageObjectsAsync(session, collection, session->getUserId(), 2).get();
    test.stopTest(list->objects.size() <= 2);
  } catch (const std::exception& e) {
    NLOG_INFO("test failed: " + std::string(e.what()));
    test.stopTest(false);
  }
}

void test_storage() {
  test_writeStorageInvalidArgument();
  test_writeStorage();
  test_writeStorageCursor();
  test_readStorageObjects();
  test_deleteStorageObjects();
  test_writeStorage_invalidJson();
  test_writeStorageMultiple();
  test_writeStorage_emptyCollection();
  test_writeStorage_emptyKey();
  test_writeStorage_publicRead();
  test_readStorage_nonExistent();
  test_readStorage_multiple();
  test_deleteStorage_multiple();
  test_listStorage_withLimit();
}

} // namespace Test
} // namespace Nakama
