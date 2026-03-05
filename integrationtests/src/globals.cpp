/*
 * Copyright 2026 The Nakama Authors
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

#include "globals.h"

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

namespace Nakama {
namespace Test {
std::atomic<uint32_t> g_runTestsCount{0};
std::atomic<uint32_t> g_failedTestsCount{0};
std::mutex g_failedTestNamesMutex;
std::vector<std::string> g_failedTestNames;
} // namespace Test
} // namespace Nakama
