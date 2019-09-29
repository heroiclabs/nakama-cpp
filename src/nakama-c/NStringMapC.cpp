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

#ifdef BUILD_C_API

#include "nakama-c/NStringMap.h"
#include "nakama-cpp/NTypes.h"
#include <memory>
#include <unordered_map>

NAKAMA_NAMESPACE_BEGIN

using NStringMapPtr = std::shared_ptr<NStringMap>;
static std::unordered_map<::NStringMap, NStringMapPtr> g_maps;

::NStringMap saveNStringMap(const NStringMap& map)
{
    NStringMapPtr map_ptr(new NStringMap(map));

    g_maps.emplace((::NStringMap)map_ptr.get(), map_ptr);

    return (::NStringMap)map_ptr.get();
}

NStringMap* findNStringMap(::NStringMap map)
{
    if (map)
    {
        auto it = g_maps.find(map);
        if (it != g_maps.end())
        {
            return it->second.get();
        }
    }

    return nullptr;
}

NAKAMA_NAMESPACE_END

extern "C" {

NStringMap NStringMap_create()
{
    return Nakama::saveNStringMap({});
}

void NStringMap_setValue(NStringMap map, const char* key, const char* value)
{
    auto cppMap = Nakama::findNStringMap(map);

    (*cppMap)[key] = value;
}

const char* NStringMap_getValue(NStringMap map, const char* key)
{
    auto cppMap = Nakama::findNStringMap(map);

    auto it = cppMap->find(key);
    if (it != cppMap->end())
    {
        return it->second.c_str();
    }

    return nullptr;
}

void NStringMap_getKeys(NStringMap map, const char** keysArray)
{
    auto cppMap = Nakama::findNStringMap(map);
    size_t i = 0;

    for (auto& it : *cppMap)
    {
        keysArray[i++] = it.first.c_str();
    }
}

uint16_t NStringMap_getSize(NStringMap map)
{
    return (uint16_t)Nakama::findNStringMap(map)->size();
}

void NStringMap_destroy(NStringMap map)
{
    if (map)
    {
        Nakama::g_maps.erase(map);
    }
}

} // extern "C"

#endif // BUILD_C_API
