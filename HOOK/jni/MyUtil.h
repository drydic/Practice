//
// Created by zhengnan on 2016/6/24.

//
#ifndef REFEE_UTIL_H

#define REFEE_UTIL_H

#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <android/log.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <android/log.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#define STRLEN_MAX 256
#define LOG_TAG "jniHook"
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args)

using namespace std;
jobject getContext(JNIEnv * env);
void read_map(const char * name , unsigned long  *start ,unsigned long * end);
void copy_file(const char * src , const  char * dest);
std::string get_jobject_name(JNIEnv* env,jobject j,jclass (*proto_GetObjectClass)(JNIEnv*, jobject));
string jstringTostring(JNIEnv* env, jstring jstr);
jobject getSp(JNIEnv * env, jobject ctxObj);
jobject getCommonSp(JNIEnv * env, jobject ctxObj);
jstring getMyPackageName(JNIEnv * env,jstring befPname);
jobject  getMyApplication(JNIEnv* env,jstring packageName, jint flags);
//只拦截applicationinfo
jobject  getMyApplication4onlyPath(JNIEnv* env,jstring packageName, jint flags);
string jstringTostring2(JNINativeInterface* env1,JNIEnv* env,jstring jstr);
#endif //REFEE_UTIL_H
