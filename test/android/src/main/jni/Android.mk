LOCAL_MODULE    := native-activity
LOCAL_SRC_FILES := main.cpp
LOCAL_LDLIBS    := -llog -landroid
LOCAL_STATIC_LIBRARIES := android_native_app_glue
$(call import-module,android/native_app_glue)
