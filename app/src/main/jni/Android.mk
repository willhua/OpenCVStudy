LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


OpenCV_INSTALL_MODULES := on
OpenCV_CAMERA_MODULES := off

OPENCV_LIB_TYPE := STATIC

ifeq ("$(wildcard $(OPENCV_MK_PATH))","")
include ..\..\..\..\native\jni\OpenCV.mk
else
include $(OPENCV_MK_PATH)
endif

LOCAL_MODULE := OpenCV
#LOCAL_ARM_NEON := true

LOCAL_SRC_FILES := JniEnvInit.cpp JniUtils.cpp OpenCVMethod.cpp  JniTest.cpp  LyhDehazor.cpp Dehazor.cpp FastDehazor.cpp FastDehazorCV.cpp

LOCAL_LDLIBS +=  -lm -llog -ljnigraphics

include $(BUILD_SHARED_LIBRARY)