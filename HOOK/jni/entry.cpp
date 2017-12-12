//
// Created by zhengnan on 2016/1/7.
//
#include <jni.h>
#include <android/log.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ptrace.h>

#define LOGV(...)  ((void)__android_log_print(ANDROID_LOG_VERBOSE, "ArtHook_native", __VA_ARGS__))


int registerNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *gMethods,
                          int numMethods) {
    jclass clazz;
    clazz = env->FindClass(className);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return JNI_FALSE;
    }
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

jboolean _munprotect(JNIEnv *env, jclass _cls, jlong addr, jlong len) {
    int pagesize = sysconf(_SC_PAGESIZE);
    int alignment = (addr % pagesize);

    int i = mprotect((void *) (addr - alignment), (size_t) (len + alignment),
                     PROT_READ | PROT_WRITE | PROT_EXEC);
    if (i == -1) {
        LOGV("mprotect failed: %s (%d)", strerror(errno), errno);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

void _memcpy(JNIEnv *env, jclass _cls, jlong src, jlong dest, jint length) {
    char *srcPnt = (char *) src;
    char *destPnt = (char *) dest;
    for (int i = 0; i < length; ++i) {
        destPnt[i] = srcPnt[i];
    }
}

void _memput(JNIEnv *env, jclass _cls, jbyteArray src, jlong dest) {
    jbyte *srcPnt = env->GetByteArrayElements(src, 0);
    jsize length = env->GetArrayLength(src);
    unsigned char *destPnt = (unsigned char *) dest;
    for (int i = 0; i < length; ++i) {
        destPnt[i] = srcPnt[i];
    }
    env->ReleaseByteArrayElements(src, srcPnt, 0);
}

jbyteArray _memget(JNIEnv *env, jclass _cls, jlong src, jint length) {
    jbyteArray dest = env->NewByteArray(length);
    if (dest == NULL) {
        return NULL;
    }
    unsigned char *destPnt = (unsigned char *) env->GetByteArrayElements(dest, 0);
    unsigned char *srcPnt = (unsigned char *) src;
    for (int i = 0; i < length; ++i) {
        destPnt[i] = srcPnt[i];
    }
    env->ReleaseByteArrayElements(dest, (jbyte *) destPnt, 0);
    return dest;
}

jlong _mmap(JNIEnv *env, jclass _cls, jint length) {
    void *space = mmap(0, length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS,-1, 0);
    if (space == MAP_FAILED) {
        LOGV("mmap failed: %s (%d)", strerror(errno), errno);
        return 0;
    }
    return (jlong) space;
}

jboolean _munmap(JNIEnv *env, jclass _cls, jlong addr, jint length) {
    int r = munmap((void *) addr, length);
    if (r == -1) {
        LOGV("munmap failed: %s (%d)", strerror(errno), errno);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

void _ptrace(JNIEnv *env, jclass _cls, jint pid) {
    ptrace(PTRACE_ATTACH, (pid_t) pid, 0, 0);
}

int registerNatives(JNIEnv *env) {
    JNINativeMethod invoker_native_methods[] = {
            {"munprotect", "(JJ)Z",  (void *) _munprotect},
            {"memcpy",     "(JJI)V", (void *) _memcpy},
            {"memput",     "([BJ)V", (void *) _memput},
            {"memget",     "(JI)[B", (void *) _memget},
            {"mmap",       "(I)J",   (void *) _mmap},
            {"munmap",     "(JI)Z",  (void *) _munmap},
            {"ptrace",     "(I)Z",   (void *) _ptrace}

    };

    int retCode = registerNativeMethods(env, "de/larma/arthook/Native", invoker_native_methods,
                                        sizeof(invoker_native_methods) /
                                        sizeof(invoker_native_methods[0]));
    return retCode;
}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;
    LOGV("JNI_OnLoad..");
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }

    if (!registerNatives(env)) { //注册
        LOGV("registerNatives failed..");
        return -1;
    }
    //成功
    result = JNI_VERSION_1_4;

    LOGV("JNI_OnLoad..success");
    return result;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    LOGV("JNI_OnUnload");
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return;
    }

}
