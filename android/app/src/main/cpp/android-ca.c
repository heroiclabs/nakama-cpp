/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <jni.h>
#include <string.h>
#include "nakama-cpp/log/NLogger.h"

void std::string getCaPath(JNIEnv *env, object instance)
{
  jmethodID getCAPath = (*env)->GetStaticMethodID(env, g_ctx.jniHelperClz, "getCAPath", "()Ljava/lang/String;");
  if (!getCAPath) {
    NLOG(NLogLevel::Error, "Failed to retrieve getCAPath() method.");
    return;
  }

  jstring caPathResult = (*env)->CallStaticObjectMethod(env, g_ctx.jniHelperClz, getCAPath);
  const char *caPath = (*env)->GetStringUTFChars(env, caPathResult, NULL);
  if (!caPath) {
    NLOG(NLogLevel::Error, "Failed to retrieve CA path.");
    return;
  }

  std::string pathString(caPath)

  (*env)->ReleaseStringUTFChars(env, buildVersion, version);
  (*env)->DeleteLocalRef(env, buildVersion);

  return pathString;
}
