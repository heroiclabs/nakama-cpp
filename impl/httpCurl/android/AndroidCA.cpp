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
    CACertificateData getCaCertificates(JNIEnv *env)
    {
        CACertificateData certData;

        jclass cls = env->FindClass("com/nakamasdk/AndroidCA");
        jmethodID mid = env->GetStaticMethodID(cls, "getCaCertificates", "()[B");
        if (mid == 0) {
            NLOG(NLogLevel::Error, "No getCaCertificates method found.");
            return certData;
        }

        jbyteArray certificatesArray = (jbyteArray)env->CallStaticObjectMethod(cls, mid);
        jsize certificatesArrayLength = env->GetArrayLength(certificatesArray);
        jbyte* certificates = env->GetByteArrayElements(certificatesArray, NULL);

        std::unique_ptr<unsigned char[]> certificatesCharArray(new unsigned char[certificatesArrayLength]);
        memcpy(certificatesCharArray.get(), certificates, certificatesArrayLength);
        env->ReleaseByteArrayElements(certificatesArray, certificates, JNI_ABORT);

        certData.data = std::move(certificatesCharArray);
        certData.len = static_cast<int>(certificatesArrayLength);
        return certData;
    }
};
