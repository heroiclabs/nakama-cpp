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

#include <jni.h>
#include <memory>
#include <string.h>
#include "nakama-cpp/log/NLogger.h"
#include "AndroidCA.h"

/* TODO add support for users utilizing this library from a NativeActivity.
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
jclass classIWant = (jclass)jni->CallObjectMethod(cls, findClass, strClassName);*/

static JavaVM* _vm;
static JNIEnv* _env;
jclass _cls;
jmethodID _mid;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    _vm = vm;
    if (vm->GetEnv(reinterpret_cast<void**>(&_env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // Find the class. JNI_OnLoad is called from with the application-level class loader. This allows this to work.
    jclass _cls = _env->FindClass("com/heroiclabs/nakamasdk/AndroidCA");
    if (_cls == nullptr) return JNI_ERR;

    _mid = _env->GetStaticMethodID(_cls, "getCaCertificates", "()[B");
    if (_mid == 0) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

namespace Nakama
{
    CACertificateData getCaCertificates()
    {
        CACertificateData certData;
        _vm->AttachCurrentThread(&_env, NULL);

        jbyteArray certificatesArray = (jbyteArray)_env->CallStaticObjectMethod(_cls, _mid);
        jsize certificatesArrayLength = _env->GetArrayLength(certificatesArray);
        jbyte* certificates = _env->GetByteArrayElements(certificatesArray, NULL);

        std::unique_ptr<unsigned char[]> certificatesCharArray(new unsigned char[certificatesArrayLength]);
        memcpy(certificatesCharArray.get(), certificates, certificatesArrayLength);
        _env->ReleaseByteArrayElements(certificatesArray, certificates, JNI_ABORT);

        _vm->DetachCurrentThread();
        certData.data = std::move(certificatesCharArray);
        certData.len = static_cast<int>(certificatesArrayLength);
        return certData;
    }
};
