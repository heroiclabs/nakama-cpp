#
# Copyright 2019 The Nakama Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

LIB_PATH := $(LOCAL_PATH)/../libs/android/$(TARGET_ARCH_ABI)

include $(CLEAR_VARS)
LOCAL_MODULE := protobuf
LOCAL_MODULE_FILENAME := libprotobuf
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := grpc
LOCAL_MODULE_FILENAME := libgrpc
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := grpc++
LOCAL_MODULE_FILENAME := libgrpc++
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := gpr
LOCAL_MODULE_FILENAME := libgpr
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := address_sorting
LOCAL_MODULE_FILENAME := libaddress_sorting
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cares
LOCAL_MODULE_FILENAME := libcares
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := zlib
LOCAL_MODULE_FILENAME := libz
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ssl
LOCAL_MODULE_FILENAME := libssl
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := crypto
LOCAL_MODULE_FILENAME := libcrypto
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ixwebsocket
LOCAL_MODULE_FILENAME := libixwebsocket
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := nakama-cpp
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE)
LOCAL_SRC_FILES := $(LIB_PATH)/$(LOCAL_MODULE_FILENAME).a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_EXPORT_LDLIBS := -llog
LOCAL_EXPORT_CFLAGS += -DNLOGS_ENABLED
# order is important!
LOCAL_STATIC_LIBRARIES := grpc++ grpc protobuf cares address_sorting gpr zlib ssl crypto ixwebsocket
include $(PREBUILT_STATIC_LIBRARY)
