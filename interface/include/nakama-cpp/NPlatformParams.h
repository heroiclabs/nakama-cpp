/*
 * Copyright 2022 The Nakama Authors
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

#ifdef __ANDROID__
#include "jni.h"
#endif

NAKAMA_NAMESPACE_BEGIN

#ifdef __ANDROID__
struct NPlatformParameters {
    JavaVM* javaVM;
    JNIEnv* jniEnv;
};
#else
#define DEFAULT_PLATFORM_PARAMS
struct NPlatformParameters {};
#endif

NAKAMA_NAMESPACE_END
