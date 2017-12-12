#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "TKHooklib.h"
#include "MyUtil.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

using namespace std;


char package_name[128];

bool isTest = true;

char replacePackageName[128];

char matchstr[128];

int verticalLineCount = 0;

#define LOG_TAG "hook"


#define   LOGI(...)  __android_log_print(ANDROID_LOG_INFO,"sprintf",__VA_ARGS__)

#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args)


static void *do_patch(HookStruct *entity, void *new_func, const char *FunctionName) {
    strcpy(entity->FunctionName, FunctionName);
    entity->NewFunc = (void *) new_func;
    if (TK_HookExportFunction(entity)) {
        return (void *) -1;
    }
    return (void *) 0;
}

// 从str1中查找str2的个数，并返回
void findChildCnt(char *str1) {

    verticalLineCount = 0;
    while ((str1 = strstr(str1, "|")) != NULL) // 如果查找到,则执行循环，否则为空退出循环
    {
        verticalLineCount++; // 统计次数
        str1 += 1; // 加上偏移量，即移除str2
        if (verticalLineCount > 17) {
            break;
        }
    }
}


typedef char *(*proto_strcat)(char *dest, const char *src);

proto_strcat old_strcat;


char *my_strcat(char *dest, const char *src) {
    if (isTest)LOGD("strcat : dest->%s  src->%s", dest, src);
    if (strstr(dest, matchstr) != NULL && strcmp(src, package_name) == 0) {
        return old_strcat(dest, replacePackageName);
    }
    findChildCnt(dest);
    if (verticalLineCount == 17 && strcmp("1", src) == 0) {
        if (isTest)LOGD("strcat : dest->%s  src->%s", dest, src);
        return old_strcat(dest, "0");//这个1|0 应该是区分是否改包（猜测）
    }

    return old_strcat(dest, src);
}


typedef char *(*proto_strncpy)(char *dest, const char *src, size_t n);

proto_strncpy old_strncpy;

char *my_strncpy(char *dest, const char *src, size_t n) {
    if (isTest)LOGD("strncpy : dest->%s  src->%s  n->%d", dest, src, n);
    return old_strncpy(dest, src, n);
}


typedef char *(*proto_strcpy)(char *dest, const char *src);

proto_strcpy old_strcpy;

char *my_strcpy(char *dest, const char *src) {
    if (isTest)LOGD("strcpy : dest->%s  src->%s", dest, src);

    return old_strcpy(dest, src);
}


typedef int (*proto_sprintf)(char *buffer, const char *format, void *arg1, void *arg2, void *arg3,
                             void *arg4, void *arg5, void *arg6, void *arg7, void *arg8);

proto_sprintf old_sprintf;

//int n = 0;

int my_sprintf(char *buffer, const char *format, void *arg1, void *arg2, void *arg3,
               void *arg4, void *arg5, void *arg6, void *arg7, void *arg8) {


//  LOGI("%s", format);
    if (strcmp("%s:%s:%d:%d:%s:%s:", format) == 0) {
        if (isTest)LOGI("%s", format);
        if (isTest)LOGI(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        arg6 = replacePackageName;
    }
//  LOGI(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);


    return old_sprintf(buffer, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}


typedef int (*proto_open)(const char *pathname, int oflag, mode_t mode);

proto_open old_open;

int my_open(const char *pathname, int oflag, mode_t mode) {

    int ret;

    ret = old_open(pathname, oflag, mode);
    LOGI("open open (%s)  oflag (%x)   mode(%x) fd (%d)", pathname, oflag, mode, ret);
    return ret;
}


typedef FILE *(*proto_fopen)(const char *, const char *);

proto_fopen old_fopen;

FILE *my_fopen(const char *path, const char *mode) {

    char *pos1 = strstr(path, "stat");
    if (pos1 != NULL) {
        return old_fopen(path, mode);
    }
//    /data/app/com.huale.ultraman2.doov-1/base.apk

    if (strcmp(path, "/data/app/com.huale.ultraman2.doov-1/base.apk") == 0) {
        LOGI("my_fffff_open (%s)    mode(%s)", "/sdcard/ref.apk", mode);
        return old_fopen("/sdcard/ref.apk", mode);
    }
    LOGI("my_fffff_open (%s)    mode(%s)", path, mode);

    return old_fopen(path, mode);
}


typedef AAssetManager *(*proto_AAssetManager_fromJava)(JNIEnv *env, jobject assetManager);

proto_AAssetManager_fromJava old_AAssetManager_fromJava;
jobject newAsset = NULL;

AAssetManager *my_AAssetManager_fromJava(JNIEnv *env, jobject assetManager) {


    if (isTest)LOGD("AAssetManager_fromJava");
    if (newAsset != NULL) {
        return old_AAssetManager_fromJava(env, newAsset);
    }
    jclass assetManagerClazz = env->FindClass("android/content/res/AssetManager");

    jmethodID constructMethod = env->GetMethodID(assetManagerClazz, "<init>", "()V");

    jobject newAssetManager = env->NewObject(assetManagerClazz, constructMethod);

    newAsset = env->NewGlobalRef(newAssetManager);

    jmethodID addAssetPathMethod = env->GetMethodID(assetManagerClazz, "addAssetPath",
                                                    "(Ljava/lang/String;)I");


    char pathChar[256];
    sprintf(pathChar, "/data/data/%s/files/xi_bwcn/realRef/res/ref.apk", package_name);

//    jstring path = env->NewStringUTF("/sdcard/test.apk");
    jstring path = env->NewStringUTF(pathChar);
    env->CallIntMethod(newAsset, addAssetPathMethod, path);

    env->DeleteLocalRef(assetManagerClazz);
    env->DeleteLocalRef(newAssetManager);
    env->DeleteLocalRef(path);

    return old_AAssetManager_fromJava(env, newAsset);

}


#define define_hook(fun) \
 if(do_patch(&entity, (void*)my_##fun, #fun) == (void*)-1){ \
       if (isTest) LOGD("Hook %s failed", #fun); \
        return ; \
    } \
    old_##fun = (proto_##fun)(entity.OldFunc); \
    if (isTest)LOGD("Hook %s OK",#fun)


#define define_hook_inline(fun) \
    ret = TK_InlineHookFunction((void *)fun,(void *) my_##fun, &OldFunc); \
    old_##fun = (proto_##fun)(OldFunc); \
    if (isTest)LOGD("Hook %s %dOK",#fun, ret)


void hook_libc() {
    HookStruct entity;
    strcpy(entity.SOName, "libc.so");
    int ret;
    void *OldFunc;
    define_hook(strcat);
    define_hook_inline(AAssetManager_fromJava);


//    define_hook(open);
//    define_hook(fopen);
//    define_hook(strcpy);
//    define_hook(strncpy);
    define_hook(sprintf);
}


void get_package_name() {
    int pid = getpid();
    char buff[128];
    sprintf(buff, "/proc/%d/cmdline", pid);
    int fd = open(buff, O_RDONLY);
    if (isTest)LOGD("get_package_name filename(%s) fd(%d)", buff, fd);
    read(fd, package_name, 128);
    if (isTest) LOGD("get_package_name(%s)", package_name);
    close(fd);

    char path[128];
    sprintf(buff, "/data/data/%s/files/refee/cmdline", package_name);

    FILE *pFile = fopen(buff, "r"); //获取文件的指针
    char *pBuf;  //定义文件指针
    fseek(pFile, 0, SEEK_END); //把指针移动到文件的结尾 ，获取文件长度
    int len = ftell(pFile); //获取文件长度
    pBuf = new char[len + 1]; //定义数组长度
    rewind(pFile); //把指针移动到文件开头 因为我们一开始把指针移动到结尾，如果不移动回来 会出错
    fread(pBuf, 1, len, pFile); //读文件
    pBuf[len] = 0; //把读到的文件最后一位 写为0 要不然系统会一直寻找到0后才结束
    fclose(pFile); // 关闭文件
//    replacePackageName = pBuf;
//    sprintf(replacePackageName, "%s",pBuf);
    sprintf(replacePackageName, "%s", pBuf);
    if (isTest) LOGD("replacePackageName(%s)", replacePackageName);


//    char temp[128];

    sprintf(matchstr, "|%s|", replacePackageName);

    if (isTest) LOGD("matchstr(%s)", matchstr);
//    matchstr = temp;

}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    FILE *fp = fopen("/sdcard/hookLog", "r");
    if (fp != NULL) {
        isTest = true;
        fclose(fp);
    } else {
        isTest = false;
    }
    get_package_name();
    hook_libc();

    if (isTest)LOGI("hookV26009 success");

    return JNI_VERSION_1_4;
}


