LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDLIBS    := -llog
LOCAL_LDLIBS += -landroid

LOCAL_MODULE := hookV26009

LOCAL_SRC_FILES := \
	Android.mk \
	hook.cpp \
	Application.mk \

LOCAL_SHARED_LIBRARIES :=TKHooklib

include $(BUILD_SHARED_LIBRARY)
include $(LOCAL_PATH)/prebuilt/Android.mk

