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
#include <memory.h>
#include <string.h>
#include "nakama-cpp/log/NLogger.h"
#include "AndroidCA.h"

/*
TODO add support for users utilizing this library from a NativeActivity.
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

static JavaVM* _vm;
static jclass _cls;
static jmethodID _mid;
static JNIEnv* _env; // use a single continuous native thread for our cert grabbing (TODO maybe we could cache the cert data)

extern "C"
{
    // Invoked when this library is loaded using System.loadLibrary() in the JVM.
    // There is a 1:1 relationship between JNIEnv and a native thread.
    // In this callback, we get the current JNIEnv (load_env) because it has a classloader that can find AndroidCA.
    // But in getCACertificiates we create use a new native thread so that it can run in isolation (the ART manages the load_env, so we need our own.)
    // Keep in mind that getCACertificates only has the "system" classloader, it would not be able to find the AndroidCA class, that's why we cache the class and method ids in the OnLoad callback.
    JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
        _vm = vm;

        JNIEnv* load_env;

        if (vm->GetEnv(reinterpret_cast<void**>(&load_env), JNI_VERSION_1_6) != JNI_OK) {
            return JNI_ERR;
        }

        // Find the class. JNI_OnLoad is called from with the application-level rather than just system level class loader. This allows this to work.
        _cls = load_env->FindClass("com/heroiclabs/nakamasdk/AndroidCA");
        if (_cls == NULL) {
            NLOG_ERROR("Failed to find class com/heroiclabs/nakamasdk/AndroidCA");
            return JNI_ERR;
        }

        // promote the jclass to a global object. otherwise we can't share it between native threads (remember, JNI represents a native thread)
        // because the underlying Java object representing the class gets GC'd.
        _cls = (jclass) load_env->NewGlobalRef(_cls);

        _mid = load_env->GetStaticMethodID(_cls, "getCaCertificates", "()[B");
        if (_mid == NULL) {
            NLOG_ERROR("Failed to find method getCaCertificates in class com/heroiclabs/nakamasdk/AndroidCA");
            return JNI_ERR;
        }

        int result = _vm->AttachCurrentThread(&_env, NULL);
        // Attach the current thread to the JVM.
        if (result != JNI_OK) {
            NLOG_ERROR("Thread attach failed: " + std::to_string(result));
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


        NLOG_DEBUG("Calling static method...");

        jbyteArray certificatesArray = (jbyteArray)_env->CallStaticObjectMethod(_cls, _mid);
        if (certificatesArray == NULL) {
            NLOG_ERROR("Failed to call method getCaCertificates in class com/heroiclabs/nakamasdk/AndroidCA");
            return certData;
        }


        jsize certificatesArrayLength = _env->GetArrayLength(certificatesArray);
        jbyte* certificates = _env->GetByteArrayElements(certificatesArray, NULL);
        if (certificates == NULL) {
            NLOG_ERROR("Failed to get elements of certificatesArray");
            _env->ReleaseByteArrayElements(certificatesArray, certificates, JNI_ABORT);
            return certData;
        }

        NLOG_DEBUG("Getting certificates char array");

        std::unique_ptr<unsigned char[]> certificatesCharArray(new unsigned char[certificatesArrayLength]);
        memcpy(certificatesCharArray.get(), certificates, certificatesArrayLength);
        _env->ReleaseByteArrayElements(certificatesArray, certificates, JNI_ABORT);


        certData.data = std::move(certificatesCharArray);
        certData.len = static_cast<int>(certificatesArrayLength);

        NLOG_DEBUG("returning cert data");

        return certData;
    }
};
