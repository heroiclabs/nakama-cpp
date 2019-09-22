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

#include "nakama-c/NStringDoubleMap.h"
#include "nakama-cpp/NTypes.h"
#include <memory>
#include <unordered_map>

NAKAMA_NAMESPACE_BEGIN

using NStringDoubleMapPtr = std::shared_ptr<NStringDoubleMap>;
static std::unordered_map<::NStringDoubleMap, NStringDoubleMapPtr> g_maps;

::NStringDoubleMap saveNStringDoubleMap(const NStringDoubleMap& map)
{
    NStringDoubleMapPtr map_ptr(new NStringDoubleMap(map));

    g_maps.emplace(map_ptr.get(), map_ptr);

    return map_ptr.get();
}

NStringDoubleMap* findNStringDoubleMap(::NStringDoubleMap map)
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

NStringDoubleMap NStringDoubleMap_create()
{
    return Nakama::saveNStringDoubleMap({});
}

void NStringDoubleMap_setValue(NStringDoubleMap map, const char* key, double value)
{
    auto cppMap = Nakama::findNStringDoubleMap(map);

    (*cppMap)[key] = value;
}

bool NStringDoubleMap_getValue(NStringDoubleMap map, const char* key, double* value)
{
    auto cppMap = Nakama::findNStringDoubleMap(map);

    auto it = cppMap->find(key);
    if (it != cppMap->end())
    {
        *value = it->second;
        return true;
    }

    return false;
}

void NStringDoubleMap_getKeys(NStringDoubleMap map, const char** keysArray)
{
    auto cppMap = Nakama::findNStringDoubleMap(map);
    size_t i = 0;

    for (auto& it : *cppMap)
    {
        keysArray[i++] = it.first.c_str();
    }
}

uint16_t NStringDoubleMap_getSize(NStringDoubleMap map)
{
    return (uint16_t)Nakama::findNStringDoubleMap(map)->size();
}

void NStringDoubleMap_destroy(NStringDoubleMap map)
{
    if (map)
    {
        Nakama::g_maps.erase(map);
    }
}

} // extern "C"
