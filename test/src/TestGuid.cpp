/*
 * Copyright 2023 The Nakama Authors
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

#include <string>
#include <sstream>
#include <random>
#include <iomanip>
#include "TestGuid.h"

namespace Nakama::Test {
    // unlikely to meet the uniqueness + unpredictability requirements of a real GUID generator,
    // but more portable than any other library and good enough for a test suite.
    std::string TestGuid::newGuid() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis(0, std::numeric_limits<uint64_t>::max());

        // Generate 128 bits of random data
        uint64_t data1 = dis(gen);
        uint64_t data2 = dis(gen);

        // Format the data as a GUID
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(16) << data1 << std::setw(16) << data2;
        std::string guid = ss.str();

        // Insert hyphens at the appropriate positions
        guid.insert(20, "-");
        guid.insert(16, "-");
        guid.insert(12, "-");
        guid.insert(8, "-");

        return guid;
    }
}
