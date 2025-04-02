LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := packet_hook
LOCAL_SRC_FILES := main.cpp utils/armhook.cpp utils/addresses.cpp 

LOCAL_LDLIBS := -llog -landroid

LOCAL_CFLAGS := -Wall -Wextra -O2

include $(BUILD_SHARED_LIBRARY)
