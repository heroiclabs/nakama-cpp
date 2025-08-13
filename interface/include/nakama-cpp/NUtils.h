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

#pragma once

#include <nakama-cpp/NTypes.h>
#include <nakama-cpp/NExport.h>

NAKAMA_NAMESPACE_BEGIN

    /**
     * Get current UNIX time in milliseconds.
     * 
     * Returns number of milliseconds that have elapsed since 00:00:00 Thursday, 1 January 1970.
     * 
     * @return UNIX time in milliseconds.
     */
    NAKAMA_API NTimestamp getUnixTimestampMs();

    struct disarmed_t{};
    inline constexpr disarmed_t disarmed{};

    // like unique_ptr but for non-pointer types
    template<typename T, int (*d)(T)>
    class Resource {
    public:
      explicit Resource(T v): r(v) {}
      explicit Resource(disarmed_t): r(), armed(false) {}
      ~Resource() { armed ? d(r) : 0; }
      Resource(Resource&) = delete;
      Resource(const Resource&) = delete;
      Resource& operator=(const Resource&) = delete;
      Resource& operator=(Resource&&) = delete;
      Resource(Resource&& r) = default;
      explicit operator bool() { return armed; }

      T& get() { return r; }
      void reset(T v) { armed ? d(r) : 0; armed = true; r = v; }
      void reset(disarmed_t) { armed ? d(r) : 0; armed = false; r = T(); }
      // Call this only after making sure the resource is invalid or you handle destruction manually. This will not call the delete function on the resource!
      void disarm() { armed = false; }
    private:
      T r;
      bool armed = true;
    };
NAKAMA_NAMESPACE_END
