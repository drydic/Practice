#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "TKHooklib.h"
#include "MyUtil.h"
#include <vector>
#include <dirent.h>
#include <dlfcn.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <stl/_string_fwd.h>
#include <stl/_string.h>

using namespace std;


char package_name[128];
JNIEnv *env = NULL;

#define LOG_TAG "Lmt"


#define   LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args)


#define reg(my_funs, fun) \
temp =(unsigned char *)my_funs; \
offset =  (unsigned int) (&(my_funs->fun)) -(unsigned int) my_funs; \
temp = temp +offset; \
temp_mid = (unsigned int *)temp; \
*temp_mid =(unsigned int) my_##fun

class MethodItem {
public:
    MethodItem() {}

    unsigned int m_id;
    string m_name;

};

using namespace std;

void hookInit(JNIEnv *env);

struct JNINativeInterface *myFunctions;
static std::vector<MethodItem *> methond_ids;

static jobject
CallObjectMethod_internal(JNIEnv *a, jobject b, jmethodID c, MethodItem *mi, int type,
                          va_list argptr);

#define  HOOK_SDK
JNINativeInterface *old_env;


static void *do_patch(HookStruct *entity, void *new_func, const char *FunctionName) {
    strcpy(entity->FunctionName, FunctionName);
    entity->NewFunc = (void *) new_func;
    if (TK_HookExportFunction(entity)) {
        return (void *) -1;
    }
    return (void *) 0;
}


#define USE_D_DATA


const char *files[] =
        {
                "base.apk",
                "sdk_prefs",
                "mgAS.dat",
                "mgSS.dat",
                "mgid.dat",
                "libmiguED.so",
                "libmgRun",
//                "libmg20pbase.so",
#ifdef USE_D_DATA
                "mg20dss.dat",
#endif
                "_BN202.cod",
                "_BN202.dat",
//                "Charge20.xml",
//                "mg20irid.dat",
                NULL
        };
const char *files_replace[] =
        {
                "sdk_prefs",
                "mgAS.dat",
                "mgSS.dat",
                "mgid.dat",
                "libmiguED.so",
                "libmgRun",
//                "libmg20pbase.so",
#ifdef USE_D_DATA
                "mg20dss.dat",
#endif
                "_BN202.cod",
                "_BN202.dat",
                NULL
        };

char *getFileName(const char *fileName) {
    // check the parameter !
    if (NULL == fileName) {
        LOGI(" dir_name is null ! ");
        return NULL;
    }

    char path[256];
    sprintf(path, "/data/data/%s/files/refee/assets", package_name);
    // check if dir_name is a valid dir
    struct stat s;
    lstat(path, &s);
    if (!S_ISDIR(s.st_mode)) {
        LOGI(" dir_name is not a valid directory ! ");
        return NULL;
    }

    struct dirent *file;    // return value for readdir()
    DIR *dir;                   // return value for opendir()
    dir = opendir(path);
    if (NULL == dir) {
        LOGI(" Can not open dir ! ");
        return NULL;
    }
//    LOGI(" Successfully opened the dir ! ");

    /* read all the files in the dir ~ */
    while ((file = readdir(dir)) != NULL) {
        if (strstr(file->d_name, fileName) != NULL) {
            LOGI("fileName : %s", file->d_name);
            return file->d_name;
        }
    }
    LOGI("not find file");
    return NULL;
}


vector<int> fds;

int is_exist(int data) {
    vector<int>::iterator ite = find(fds.begin(), fds.end(), data);
    if (ite != fds.end()) {
        return 1;
    }
    return 0;
}

typedef ssize_t (*proto_write)(int fd, const void *buf, size_t count);

proto_write old_write;

ssize_t my_write(int fd, const void *buf, size_t count) {


    LOGD("buf size (%d)", sizeof(buf));
    if (is_exist(fd)) {
        LOGD("fd %d is in the vector do not write \n", fd);
        return count;
    } else {
        LOGD("fd %d  is in not the vector write with old\n", fd);
        return old_write(fd, buf, count);
    }
}

typedef int (*proto_close)(int fd);

proto_close old_close;

int my_close(int fd) {
    vector<int>::iterator ite = find(fds.begin(), fds.end(), fd);
    if (ite != fds.end()) {
        fds.erase(ite);
        LOGD("fd %d is  in the vector close it !!\n", fd);
    } else {
        LOGD("fd %d is not in the vector close it  !!\n", fd);
    }
    return old_close(fd);
}


typedef int (*proto_open)(const char *pathname, int oflag, mode_t mode);

proto_open old_open;

int my_open(const char *pathname, int oflag, mode_t mode) {
    char *pos1;
    char *pos2;
    int ret;
    char new_path[256];

    LOGD("open pathName %s", pathname);

    pos1 = strstr(pathname, "stat");
    if (pos1 != NULL) {
        return old_open(pathname, oflag, mode);
    }


    for (int i = 0; files_replace[i] != NULL; i++) {
        const char *pos = strstr(pathname, files_replace[i]);
        pos1 = strstr(pathname, "/tmp");
        if (pos != NULL && pos1 == NULL) {
            if (strcmp("mgAS.dat", files_replace[i]) == 0 ||
                strcmp(files_replace[i], "mgSS.dat") == 0 ||
                strcmp(files_replace[i], "mgid.dat") == 0) {
                sprintf(new_path, "/data/data/%s/files/%s", package_name,
                        files_replace[i]);
                LOGI("open file use new path %s", new_path);
                return old_open(new_path, oflag, mode);
            }
            if (strcmp(files_replace[i], "libmiguED.so") == 0) {
                sprintf(new_path, "/data/data/%s/files/edfile/armeabi/%s",
                        package_name, files_replace[i]);
                LOGI("open file use new path %s", new_path);
                return old_open(new_path, oflag, mode);
            }


            char *filename = getFileName(files_replace[i]);
            if (filename != NULL) {
                sprintf(new_path, "/data/data/%s/files/%s", package_name, filename);
                LOGI("open file use new path %s", new_path);
                return old_open(new_path, oflag, mode);
            }

            ret = old_open(pathname, oflag, mode);
            return ret;
        }
    }


    pos1 = strstr(pathname, "cmdline");
    if (pos1 != NULL) {
        sprintf(new_path, "/data/data/%s/files/new/%s", package_name, "cmdline");
        ret = old_open(new_path, oflag, mode);
        LOGI("file use new path %s", new_path);
    } else {

        ret = old_open(pathname, oflag, mode);
        LOGI("open file open (%s)  oflag (%x)   mode(%x) fd (%d)", pathname, oflag, mode, ret);
    }


    return ret;
}


typedef FILE *(*proto_fopen)(const char *, const char *);

proto_fopen old_fopen;

FILE *my_fopen(const char *path, const char *mode) {

    char *pos1 = strstr(path, "stat");
    if (pos1 != NULL) {
        return old_fopen(path, mode);
    }

    LOGE("my_fopen (%s)    mode(%s)", path, mode);

//    if (strcmp(mode, "w") == 0) {
//        return old_fopen(path, mode);
//    }

    char new_path[256];
    for (int i = 0; files[i] != NULL; i++) {
        const char *pos = strstr(path, files[i]);
        const char *pos1 = strstr(path, "/tmp");
        if (pos != NULL && pos1 == NULL) {


            if ((strcmp("mgAS.dat", files[i]) == 0 || strcmp(files[i], "mgSS.dat") == 0 ||
                 strcmp(files[i], "mgid.dat") == 0) &&
                (strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0)) {

                sprintf(new_path, "/data/data/%s/files/%s", package_name, files[i]);
                LOGE("my_fopen file use new path %s", new_path);
                return old_fopen(new_path, mode);
            }


            if ((strcmp(files[i], "libmiguED.so") == 0) &&
                (strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0)) {
                sprintf(new_path, "/data/data/%s/files/edfile/armeabi/%s",
                        package_name, files[i]);
                LOGE("fopen file use new path %s", new_path);
                return old_fopen(new_path, mode);
            }


            char *filename = getFileName(files[i]);
            if ((filename != NULL) &&
                (strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0)) {
                sprintf(new_path, "/data/data/%s/files/%s", package_name, filename);
                LOGE("fopen file use new path %s", new_path);
                return old_fopen(new_path, mode);
            }

//            sprintf(new_path, "/data/data/%s/files/%s", package_name, files[i]);
            return old_fopen(path, mode);
        }
    }
    return old_fopen(path, mode);
}

typedef void *(*proto_mmap)(void *start, size_t length, int prot, int flags, int fd, off_t offset);

proto_mmap old_mmap;

void *my_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset) {
    LOGE("====mmap     fd(%d)", fd);
    return old_mmap(start, length, prot, flags, fd, offset);
}


#define hook_fun(ret, funname, ...) \
typedef ret (*proto_##funname)(__VA_ARGS__); \
proto_##funname old_##funname; \
ret my_##funname(__VA_ARGS__)

hook_fun(void *, dlopen, const char *pathname, int mode) {

    LOGD("hook_fun pathName %s", pathname);
    char *pos1 = strstr(pathname, "libmg20pbase.so");
    char new_path[256];
    if (pos1 != NULL) {
        sprintf(new_path, "/data/data/%s/files/refee/assets/%s", package_name, "libmg20pbase.so");
        void *ret = old_dlopen(pathname, mode);
        LOGE("dlopen file use new path %s", new_path);
        return ret;
    }

    return old_dlopen(pathname, mode);
}


typedef char *(*proto_strcpy)(char *dest, const char *src);

proto_strcpy old_strcpy;

bool isHook = false;

char *my_strcpy(char *dest, const char *src) {
    LOGI("strcpy : dest->%s  src->%s", dest, src);

    return old_strcpy(dest, src);
}

typedef char *(*proto_strcat)(char *dest, const char *src);

proto_strcat old_strcat;


const char *replacePackageName = "com.joym.armorhero.lt";

const char *matchstr = "|com.joym.armorhero.lt|";


char *my_strcat(char *dest, const char *src) {
//    LOGI("strcat : dest->%s  src->%s", dest, src);
    if (strstr(dest, matchstr) != NULL && strcmp(src, package_name) == 0) {
        return old_strcat(dest, replacePackageName);
    }
    return old_strcat(dest, src);
}


typedef char *(*proto_strncpy)(char *dest, const char *src, size_t n);

proto_strncpy old_strncpy;

char *my_strncpy(char *dest, const char *src, size_t n) {
    LOGI("strncpy : dest->%s  src->%s  n->%d", dest, src, n);
    return old_strncpy(dest, src, n);
}

typedef int (*proto_sprintf)(char *buffer, const char *format, char *arg1, char *arg2, char *arg3,
                             char *arg4, char *arg5);

proto_sprintf old_sprintf;

bool isLoad = false;


int substring(char *str, const char *str1) {
    int x = 0;
    char *p = NULL;//任意附个初始值
    do {
        p = strstr(str, str1);//1.p指针指向strstr的返回值。3.再一次循环到 这里函数的参数发生变化，p重新指向strstr返回值，如此循环。
        if (p != NULL) {
            str = p + 1;//2.str同样指向strstr返回值p的下一个地址。
            x = x + 1;
        }
    } while (p != NULL);
    return x;
}

int my_sprintf(char *buffer, const char *format, char *arg1, char *arg2, char *arg3,
               char *arg4, char *arg5) {


    if (!isLoad) {
        isLoad = true;
        LOGI("sprintf : buffer->%s  format->%s arg1->%s arg2->%s", buffer, format, arg1, arg2);
        if (strstr(arg2, "libmg20pbase") != NULL) {

            return old_sprintf(buffer, "/data/data/%s/lib/%s", package_name, arg2, arg3,
                               arg4, arg5);
        }
    }

    return old_sprintf(buffer, format, arg1, arg2, arg3, arg4, arg5);
}


typedef AAsset *(*proto_AAssetManager_open)(AAssetManager *mgr, const char *filename, int mode);

proto_AAssetManager_open old_AAssetManager_open;

AAsset *my_AAssetManager_open(AAssetManager *mgr, const char *filename, int mode) {

    LOGI("AAssetManager_open : filename->%s  mode->%d", filename, mode);

    return old_AAssetManager_open(mgr, filename, mode);
}

typedef AAssetDir *(*proto_AAssetManager_openDir)(AAssetManager *mgr, const char *dirName);

proto_AAssetManager_openDir old_AAssetManager_openDir;

AAssetDir *my_AAssetManager_openDir(AAssetManager *mgr, const char *dirName) {

    LOGI("AAssetManager_openDir : dirName->%s", dirName);

    return old_AAssetManager_openDir(mgr, dirName);
}


typedef const char *(*proto_AAssetDir_getNextFileName)(AAssetDir *assetDir);

proto_AAssetDir_getNextFileName old_AAssetDir_getNextFileName;


const char *name = (const char *) NULL;

const char *my_AAssetDir_getNextFileName(AAssetDir *assetDir) {

    return old_AAssetDir_getNextFileName(assetDir);

}


typedef AAssetManager *(*proto_AAssetManager_fromJava)(JNIEnv *env, jobject assetManager);

proto_AAssetManager_fromJava old_AAssetManager_fromJava;
jobject newAsset = NULL;

AAssetManager *my_AAssetManager_fromJava(JNIEnv *env, jobject assetManager) {


    LOGI("AAssetManager_fromJava");
    if (newAsset != NULL) {
        return old_AAssetManager_fromJava(env, newAsset);
    }
    jclass assetManagerClazz = env->FindClass("android/content/res/AssetManager");

    jmethodID constructMethod = env->GetMethodID(assetManagerClazz, "<init>", "()V");

    jobject newAssetManager = env->NewObject(assetManagerClazz, constructMethod);

    newAsset = env->NewGlobalRef(newAssetManager);

    jmethodID addAssetPathMethod = env->GetMethodID(assetManagerClazz, "addAssetPath",
                                                    "(Ljava/lang/String;)I");

    jstring path = env->NewStringUTF("/sdcard/test.apk");
    env->CallIntMethod(newAsset, addAssetPathMethod, path);

    env->DeleteLocalRef(assetManagerClazz);
    env->DeleteLocalRef(newAssetManager);
    env->DeleteLocalRef(path);

    return old_AAssetManager_fromJava(env, newAsset);

}


#define define_hook(fun) \
 if(do_patch(&entity, (void*)my_##fun, #fun) == (void*)-1){ \
        LOGD("Hook %s failed", #fun); \
        return ; \
    } \
    old_##fun = (proto_##fun)(entity.OldFunc); \
    LOGD("Hook %s OK",#fun)


#define define_hook_inline(fun) \
    ret = TK_InlineHookFunction((void *)fun,(void *) my_##fun, &OldFunc); \
    old_##fun = (proto_##fun)(OldFunc); \
    LOGD("Hook %s %dOK",#fun, ret)


void hook_libc() {
    HookStruct entity;
    strcpy(entity.SOName, "libc.so");
    int ret;
    void *OldFunc;
//    define_hook(open);
//    define_hook(fopen);
//    define_hook(write);
//    define_hook(dlopen);

//    define_hook(strcpy);
    define_hook(strcat);
//    define_hook(strncpy);
//    define_hook(sprintf);
//    define_hook_inline(AAssetManager_openDir);
//    define_hook_inline(AAssetManager_open);
//    define_hook_inline(AAssetDir_getNextFileName);
    define_hook_inline(AAssetManager_fromJava);

}


void get_package_name() {
    int pid = getpid();
    char buff[128];
    sprintf(buff, "/proc/%d/cmdline", pid);
    int fd = open(buff, O_RDONLY);
    LOGD("get_package_name filename(%s) fd(%d)", buff, fd);
    read(fd, package_name, 128);
    LOGD("get_package_name(%s)", package_name);
    close(fd);
}

jmethodID my_GetMethodID(JNIEnv *a, jclass b, const char *c, const char *d) {
    jmethodID ret = old_env->GetMethodID(a, b, c, d);
    LOGD(" GetMethodID :methodID(%p) env(%p) class (%p)   name (%s) args (%s)", ret, a, b, c, d);
    if (strstr(c, "getPackageName") != NULL) {
        LOGD("getPackageName finded !!!!!!! %p", ret);
        MethodItem *mi = new MethodItem();
        mi->m_id = (unsigned int) ret;
        mi->m_name = "getPackageName";
        methond_ids.push_back(mi);
    }
    return ret;
}

jobject my_CallObjectMethodV(JNIEnv *a, jobject b, jmethodID c, va_list argptr) {
    LOGD(" CallObjectMethodV :evn(%p) jobject (%p)   jmethodID (%p)", a, b, c);
#ifdef HOOK_SDK
    for (int i = 0; i < methond_ids.size(); i++) {
        MethodItem *mi = methond_ids[i];
        if (mi->m_id == (unsigned int) c) {
            jobject ret = CallObjectMethod_internal(a, b, c, mi, 0, argptr);
            std::vector<MethodItem *>::iterator it = methond_ids.begin() + i;
            methond_ids.erase(it);
            delete mi;
            return ret;
        }
    }
#endif
    jobject ret = old_env->CallObjectMethodV(a, b, c, argptr);
    return ret;
}


//jobject my_CallObjectMethod(JNIEnv *a, jobject b, jmethodID c, ...) {
//    LOGD(" CallObjectMethod :evn(%p) jobject (%p)   jmethodID (%p)", a, b, c);
//#ifdef HOOK_SDK
//    for (int i = 0; i < methond_ids.size(); i++) {
//        MethodItem *mi = methond_ids[i];
//        if (mi->m_id == (unsigned int) c) {
//            LOGD(" CallObjectMethod  getMyApplication  called !!!!");
//            va_list argptr;
//            va_start(argptr, c);
//            jobject ret = CallObjectMethod_internal(a, b, c, mi, 1, argptr);
//
//            std::vector<MethodItem *>::iterator it = methond_ids.begin() + i;
//            methond_ids.erase(it);
//            delete mi;
//            va_end(argptr);
//            return ret;
//        }
//    }
//#endif
//
//    va_list argptr;
//    va_start(argptr, c);
//    jobject ret = old_env->CallObjectMethod(a, b, c, argptr);
//    va_end(argptr);
//    return ret;
//}


static jobject
CallObjectMethod_internal(JNIEnv *a, jobject b, jmethodID c, MethodItem *mi, int type,
                          va_list argptr) {

    if (mi->m_name == "getPackageName") {
        jobject arg = NULL;
        if (type == 0) {
            arg = old_env->CallObjectMethodV(a, b, c, argptr);
        } else if (type == 1) {
            arg = old_env->CallObjectMethod(a, b, c);
        }
        jobject ret = getMyPackageName(a, (jstring) arg);
        return ret;
    }

    return NULL;
}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    get_package_name();
    fds.clear();
    hook_libc();

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
//    hookInit(env);//hook jni的基本方法

    LOGI("hookV26009 success");

    return JNI_VERSION_1_4;
}

void hookInit(JNIEnv *env) {
    old_env = (JNINativeInterface *) env->functions;
    myFunctions = (JNINativeInterface *) malloc(sizeof(JNINativeInterface));
    unsigned int length =
            (unsigned int) (&(env->functions->GetObjectRefType)) - (unsigned int) env->functions;
    memcpy((void *) myFunctions, (void *) env->functions, length + 4);
    unsigned char *temp;
    unsigned int offset;
    unsigned int *temp_mid;


    reg(myFunctions, GetMethodID);
    reg(myFunctions, CallObjectMethodV);
//    reg(myFunctions, CallObjectMethod);


    env->functions = myFunctions;
}
