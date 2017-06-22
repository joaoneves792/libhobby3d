#
# In order for this to compile you need to run "make java" first
# Make sure that you are using javac < 8  and set the paths in the ./High3d/Makefile are correct as Android doesnt work with java8 compiled code
# You need to set the path to the GLM headers in LOCAL_C_INCLUDES
# Also this script assumes you are using the following repos to provide libpng and libjpeg for android
# https://github.com/folecr/jpeg8d.git
# https://github.com/julienr/libpng-android.git
# You also need to set "APP_STL := stlport_static" in your Application.mk
#


LOCAL_PATH := $(call my-dir)

include $(CLEAR_CARS)

MY_LOW_SRC_FILES  :=./Low3d/MS3DFileIO.cpp ./Low3d/optimizations.cpp ./Low3d/Shader.cpp ./Low3d/MS3DFile.cpp ./Low3d/Textures.cpp
MY_HIGH_SRC_FILES :=./High3d/ms3d.cpp ./High3d/Tex.cpp ./High3d/shader.cpp ./High3d/GLM.cpp ./High3d/Lights.cpp ./High3d/Shadows.cpp ./High3d/Text.cpp ./High3d/Body.cpp

LOCAL_MODULE := jhobby3d
LOCAL_MODULE_FILENAME := libjhobby3dAndroid
LOCAL_CPP_EXTENSION :=.cxx .cpp
LOCAL_C_INCLUDES :=$(LOCAL_PATH)/Low3d/ /home/joao/workspace/prototypes/GLTESTANDROID/app/src/main/jni/ # <-- Change here the path to the glm headers
LOCAL_CFLAGS := -DGLES
LOCAL_LDLIBS := -lGLESv2 -lEGL
LOCAL_STATIC_LIBRARIES += cocos_jpeg_static libpng
LOCAL_SRC_FILES := High3d/hobby3d_java_wrap.cxx $(MY_LOW_SRC_FILES) $(MY_HIGH_SRC_FILES)


include $(BUILD_SHARED_LIBRARY)
