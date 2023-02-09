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

namespace Nakama
{
    static JavaVM* _vm;
    static JNIEnv* _env;
    jclass _cls;
    jmethodID _mid;

    JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
        _vm = vm;
        if (vm->GetEnv(reinterpret_cast<void**>(&_env), JNI_VERSION_1_6) != JNI_OK) {
            return JNI_ERR;
        }

        // Find your class. JNI_OnLoad is called from the correct class loader context for this to work.
        jclass _cls = _env->FindClass("com/heroiclabs/nakamasdk/AndroidCA");
        if (_cls == nullptr) return JNI_ERR;

        _mid = _env->GetStaticMethodID(_cls, "getCaCertificates", "()[B");
        if (_mid == 0) {
            return JNI_ERR;
        }

        return JNI_VERSION_1_6;
    }

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
