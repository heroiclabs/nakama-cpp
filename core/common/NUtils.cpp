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

#include "nakama-cpp/NUtils.h"
#include <chrono>

namespace Nakama {

using namespace std::chrono;

NTimestamp getUnixTimestampMs() {
  milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

  return ms.count();
}

} // namespace Nakama
