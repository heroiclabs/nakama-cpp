/*
 * Copyright 2023 Heroic Labs
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

#include "AndroidCA.h"
#include <android/log.h>
#include <jni.h>

/*
TODO add support for users utilizing this library from a NativeActivity.

--------------------------------------------------------------------------------------------------------------
EDIT: this may be be the better way now, "JNI_GetCreatedJavaVMs": https://github.com/android/ndk/issues/1320
--------------------------------------------------------------------------------------------------------------

In order to do so, they need to get the class loader from their native activity and pass that to our library.
We would need to provide hooks for it.
also see: https://archive.is/QzA8 (this is old so there may now be an easier way):

JNIEnv *jni;
state->vm->AttachCurrentThread(&jni, NULL);
jclass activityClass = jni->FindClass("android/app/NativeActivity");
jmethodID getClassLoader = jni->GetMethodID(activityClass,"getClassLoader", "()Ljava/lang/ClassLoader;");
jobject cls = jni->CallObjectMethod(state->activity->clazz, getClassLoader);
jclass classLoader = jni->FindClass("java/lang/ClassLoader");
jmethodID findClass = jni->GetMethodID(classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
jstring strClassName = jni->NewStringUTF("com/tewdew/ClassIWant");
jclass classIWant = (jclass)jni->CallObjectMethod(cls, findClass, strClassName);
*/

static Nakama::CACertificateData* _certData = nullptr;

extern "C" {
// Invoked when this library is loaded using System.loadLibrary() in the JVM.
// We don't need to AttachCurrentThread because this C++ thread is self-evidently already loaded.
// Although it is not necessary to cache the certificate data, it is more performant to do this.
// Also it avoids any issues with sharing the JavaVM with other libraries/game engines outside of this JNI onload call.
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  _certData = new Nakama::CACertificateData();

  // we can't use NLogger in this block of code because the library is just getting loaded. NLogger isn't setup yet.
  __android_log_print(ANDROID_LOG_INFO, "nakama", "JNI_OnLoad called.");

  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  // Find the class. JNI_OnLoad is called from with the application-level rather than just system level class loader.
  // This allows this to work.

  jclass cls = env->FindClass("com/heroiclabs/nakama/AndroidCA");
  if (cls == NULL) {
    __android_log_print(ANDROID_LOG_ERROR, "nakama", "Failed to find class com/heroiclabs/nakamasdk/AndroidCA");
    return JNI_ERR;
  }

  jmethodID mid = env->GetStaticMethodID(cls, "getCaCertificates", "()[B");
  if (mid == NULL) {
    __android_log_print(
        ANDROID_LOG_ERROR, "nakama",
        "Failed to find method getCaCertificates in class com/heroiclabs/nakamasdk/AndroidCA");
    return JNI_ERR;
  }

  __android_log_print(ANDROID_LOG_INFO, "nakama", "JNI_OnLoad done being called.");

  jbyteArray certificatesArray = (jbyteArray)env->CallStaticObjectMethod(cls, mid);
  if (certificatesArray == NULL) {
    __android_log_print(
        ANDROID_LOG_ERROR, "nakama",
        "Failed to call method getCaCertificates in class com/heroiclabs/nakamasdk/AndroidCA");
    return JNI_ERR;
  }

  jsize certificatesArrayLength = env->GetArrayLength(certificatesArray);
  jbyte* certificates = env->GetByteArrayElements(certificatesArray, NULL);
  if (certificates == NULL) {
    env->ReleaseByteArrayElements(certificatesArray, certificates, JNI_ABORT);
    return JNI_ERR;
  }

  unsigned char* certificatesCharArray = new unsigned char[certificatesArrayLength];
  memcpy(certificatesCharArray, certificates, certificatesArrayLength);
  env->ReleaseByteArrayElements(certificatesArray, certificates, JNI_ABORT);

  _certData->data = certificatesCharArray;
  _certData->len = static_cast<int>(certificatesArrayLength);
  return JNI_VERSION_1_6;
}

void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
  if (_certData != nullptr) {
    delete (_certData->data);
    delete (_certData);
    _certData = nullptr;
  }
}
}

namespace Nakama {
CACertificateData* getCaCertificates() { return _certData; }
}; // namespace Nakama
