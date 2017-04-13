//
// Created by lisan on 2017/4/13.
//
#include "JniEnvInit.h"

JavaVM *JniEnvInit::gVM = NULL;

jint JNI_OnLoad(JavaVM * vm, void * reversed)
{
    LOG("JniEnvInit JNI_OnLoad");
    JniEnvInit::gVM = vm;
    return JNI_VERSION_1_4;
}


void JNI_OnUnload(JavaVM * vm, void * reversed)
{
    LOG("JniEnvInit JNI_OnUnload");
    JniEnvInit::gVM = NULL;
}
