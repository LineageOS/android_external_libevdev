LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    libevdev/libevdev.c \
    libevdev/libevdev-uinput.c \
    libevdev/libevdev-names.c

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include

LOCAL_EXPORT_C_INCLUDE_DIRS = \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/ \
    $(LOCAL_PATH)/libevdev

LOCAL_MODULE := libevdev
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true
LOCAL_CFLAGS := -Wno-error
include $(BUILD_SHARED_LIBRARY)
