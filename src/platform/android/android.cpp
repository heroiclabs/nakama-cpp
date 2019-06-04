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

#ifdef __ANDROID__

#include "nakama-cpp/platform/android/android.h"

#if defined(BUILD_HTTP_CPPREST) || defined(BUILD_WEBSOCKET_CPPREST)
    #include "pplx/pplxtasks.h"
#endif

namespace Nakama {

    void init(JavaVM* vm)
    {
#if defined(BUILD_HTTP_CPPREST) || defined(BUILD_WEBSOCKET_CPPREST)
        cpprest_init(vm);
#endif
    }

} // Nakama

#ifdef NAKAMA_SHARED_LIBRARY

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;

    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        return -1;
    }

    Nakama::init(vm);
    return JNI_VERSION_1_6;
}

#endif // NAKAMA_SHARED_LIBRARY

#endif // __ANDROID__
