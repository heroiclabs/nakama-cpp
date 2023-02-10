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

extern "C"
{
    JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
        _vm = vm;

        NLOG_INFO("JNI ON LOAD CALLED FOR ANDROID CA");

        if (vm->GetEnv(reinterpret_cast<void**>(&_env), JNI_VERSION_1_6) != JNI_OK) {
            return JNI_ERR;
        }

        return JNI_VERSION_1_6;
    }
}

namespace Nakama
{
    CACertificateData getCaCertificates()
    {
        CACertificateData certData;

        // Attach the current thread to the JVM.
        _vm->AttachCurrentThread(&_env, NULL);

        // Find the class. JNI_OnLoad is called from with the application-level class loader. This allows this to work.
        jclass cls = _env->FindClass("com/heroiclabs/nakamasdk/AndroidCA");
        if (cls == NULL) {
            NLOG_ERROR("Failed to find class com/heroiclabs/nakamasdk/AndroidCA");
            _vm->DetachCurrentThread();
            return certData;
        }

        jmethodID mid = _env->GetStaticMethodID(cls, "getCaCertificates", "()[B");
        if (mid == NULL) {
            NLOG_ERROR("Failed to find method getCaCertificates in class com/heroiclabs/nakamasdk/AndroidCA");
            _vm->DetachCurrentThread();
            return certData;
        }

        jbyteArray certificatesArray = (jbyteArray)_env->CallStaticObjectMethod(cls, mid);
        if (certificatesArray == NULL) {
            NLOG_ERROR("Failed to call method getCaCertificates in class com/heroiclabs/nakamasdk/AndroidCA");
            _vm->DetachCurrentThread();
            return certData;
        }

        jsize certificatesArrayLength = _env->GetArrayLength(certificatesArray);
        jbyte* certificates = _env->GetByteArrayElements(certificatesArray, NULL);
        if (certificates == NULL) {
            NLOG_ERROR("Failed to get elements of certificatesArray");
            _env->ReleaseByteArrayElements(certificatesArray, certificates, JNI_ABORT);
            _vm->DetachCurrentThread();
            return certData;
        }

        std::unique_ptr<unsigned char[]> certificatesCharArray(new unsigned char[certificatesArrayLength]);
        memcpy(certificatesCharArray.get(), certificates, certificatesArrayLength);
        _env->ReleaseByteArrayElements(certificatesArray, certificates, JNI_ABORT);

        // Detach the current thread from the JVM.
        _vm->DetachCurrentThread();

        certData.data = std::move(certificatesCharArray);
        certData.len = static_cast<int>(certificatesArrayLength);

        return certData;
    }
};
