//
// Created by zhengnan on 2016/6/24.
//

#include "MyUtil.h"

jobject getContext(JNIEnv *env) {
    //activity 线程
    jclass clz_actThread = env->FindClass("android/app/ActivityThread");
    jmethodID mid_getCurThread = env->GetStaticMethodID(clz_actThread, "currentActivityThread",
                                                        "()Landroid/app/ActivityThread;");
    jobject actThread = env->CallStaticObjectMethod(clz_actThread, mid_getCurThread);
    //getApplication
    jmethodID mid_getAppCation = env->GetMethodID(clz_actThread, "getApplication",
                                                  "()Landroid/app/Application;");
    jobject application = env->CallObjectMethod(actThread, mid_getAppCation);

    //getApplicationContext
    jclass clz_Application = env->GetObjectClass(application);
    jmethodID mid_getContext = env->GetMethodID(clz_Application, "getApplicationContext",
                                                "()Landroid/content/Context;");
    jobject context = env->CallObjectMethod(application, mid_getContext);

    //del
    env->DeleteLocalRef(clz_actThread);
    env->DeleteLocalRef(actThread);
    env->DeleteLocalRef(application);
    env->DeleteLocalRef(clz_Application);
    return context;
}


jobject getCommonSp(JNIEnv *env, jobject ctxObj) {

    jclass cls_PreferenceManager = env->FindClass(
            "android/preference/PreferenceManager");
    jmethodID mid_gedf =
            env->GetStaticMethodID(cls_PreferenceManager,
                                   "getDefaultSharedPreferences",
                                   "(Landroid/content/Context;)Landroid/content/SharedPreferences;");
    jobject sp = env->CallStaticObjectMethod(cls_PreferenceManager, mid_gedf,
                                             ctxObj);
    env->DeleteLocalRef(cls_PreferenceManager);
    return sp;
}


string jstringTostring(JNIEnv *env, jstring jstr);

void read_map(const char *name, unsigned long *start, unsigned long *end) {
    int pid = getpid();
    char mapname[STRLEN_MAX];
    memset(mapname, 0, STRLEN_MAX);
    snprintf(mapname, STRLEN_MAX, "/proc/%d/maps", pid);
    FILE *file = fopen(mapname, "r");
    *start = 0;
    if (file) {
        while (1) {
            unsigned int atleast = 32;
            char line[STRLEN_MAX];
            char startbuf[9];
            char endbuf[9];
            memset(line, 0, STRLEN_MAX);
            char *linestr = fgets(line, STRLEN_MAX, file);
            if (!linestr) {
                break;
            }
            //  LOGD("%s" ,linestr );
            if (strlen(line) > atleast && strstr(line, name)) {
                LOGD("%s", linestr);
                memset(startbuf, 0, sizeof(startbuf));
                memset(endbuf, 0, sizeof(endbuf));
                memcpy(startbuf, line, 8);
                memcpy(endbuf, &line[8 + 1], 8);
                if (*start == 0) {
                    *start = strtoul(startbuf, NULL, 16);
                    *end = strtoul(endbuf, NULL, 16);
                } else {
                    *end = strtoul(endbuf, NULL, 16);
                }
            }
        }
        fclose(file);
    }

}


void copy_file(const char *src, const char *dest) {
    if (access(src, R_OK) != 0) {

        LOGD("src can not be read");
        return;
    }
    /*    if(access(dest,W_OK)!=0)
        {
            LOGD("src can not be read");
            return;
        }
    */
    FILE *pFile = fopen(src, "rb"); //获取文件的指针
    if (pFile == NULL) {
        LOGD("copy_file source file is not exist");
        return;
    }
    char *pBuf;  //定义文件指针
    fseek(pFile, 0, SEEK_END); //把指针移动到文件的结尾 ，获取文件长度
    int len = ftell(pFile); //获取文件长度
    pBuf = new char[len]; //定义数组长度
    fseek(pFile, 0, SEEK_SET); //把指针移动到文件的结尾 ，获取文件长度
    fread(pBuf, 1, len, pFile); //读文件
    fclose(pFile); // 关闭文件
    pFile = fopen(dest, "wb"); //获取文件的指针
    fseek(pFile, 0, SEEK_SET);
    fwrite(pBuf, 1, len, pFile);
    fclose(pFile);
    free(pBuf);

}

string get_jobject_name(JNIEnv *env, jobject j, jclass (*proto_GetObjectClass)(JNIEnv *, jobject)) {
    jclass obj_class = proto_GetObjectClass(env, j);
    jmethodID mid_getClass = env->GetMethodID(obj_class, "getClass", "()Ljava/lang/Class;");
    jobject obj_Nameclass = env->CallObjectMethod(j, mid_getClass);
    jmethodID mid_getName = env->GetMethodID(proto_GetObjectClass(env, obj_Nameclass), "getName",
                                             "()Ljava/lang/String;");
    jstring retObj = (jstring) env->CallObjectMethod(obj_Nameclass, mid_getName);
    string ret = jstringTostring(env, retObj);
    return ret;
}


string jstringTostring(JNIEnv *env, jstring jstr) {
    if (jstr == NULL)return "null";

    char *rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("utf-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);

    if (env->ExceptionCheck() == JNI_TRUE) {
        env->ExceptionClear();
        return "";
    }
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    string stemp;
    if (rtn != NULL) stemp = string(rtn);
    if (rtn != NULL) {
        free(rtn);
        rtn = NULL;
    }
    env->DeleteLocalRef(clsstring);
    env->DeleteLocalRef(strencode);
    env->DeleteLocalRef(barr);
    return stemp;
}

string jstringTostring2(JNINativeInterface *env1, JNIEnv *env, jstring jstr) {
    if (jstr == NULL)return "null";

    char *rtn = NULL;
    jclass clsstring = env1->FindClass(env, "java/lang/String");
    jstring strencode = env1->NewStringUTF(env, "utf-8");
    jmethodID mid = env1->GetMethodID(env, clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env1->CallObjectMethod(env, jstr, mid, strencode);
    jsize alen = env1->GetArrayLength(env, barr);
    jbyte *ba = env1->GetByteArrayElements(env, barr, JNI_FALSE);

    if (env1->ExceptionCheck(env) == JNI_TRUE) {
        env1->ExceptionClear(env);
        return "";
    }
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env1->ReleaseByteArrayElements(env, barr, ba, 0);
    string stemp;
    if (rtn != NULL) stemp = string(rtn);
    if (rtn != NULL) {
        free(rtn);
        rtn = NULL;
    }
    env1->DeleteLocalRef(env, clsstring);
    env1->DeleteLocalRef(env, strencode);
    env1->DeleteLocalRef(env, barr);
    return stemp;
}

jobject getSp(JNIEnv *env, jobject ctxObj) {
    jclass clz_ctx = env->GetObjectClass(ctxObj);
    jmethodID mid_getSharedPreferences = env->GetMethodID(clz_ctx, "getSharedPreferences",
                                                          "(Ljava/lang/String;I)Landroid/content/SharedPreferences;");
    jstring spName = env->NewStringUTF("bwcLog");
    jobject sp = env->CallObjectMethod(ctxObj, mid_getSharedPreferences, spName, (jint) 2);
    env->DeleteLocalRef(clz_ctx);
    env->DeleteLocalRef(spName);
    return sp;
}

string spGet(JNIEnv *env, jobject sp, const char *key, const char *def) {
    if (sp == NULL)return def;
    jclass cls_SharedPreferences = env->FindClass(
            "android/content/SharedPreferences");
    jmethodID mid_getString = env->GetMethodID(cls_SharedPreferences,
                                               "getString",
                                               "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    jstring jstrKey = env->NewStringUTF(key);
    jstring jstrDef = env->NewStringUTF(def);
    jstring str = (jstring) env->CallObjectMethod(sp, mid_getString, jstrKey,
                                                  jstrDef);
    if (env->ExceptionCheck() == JNI_TRUE) {
        env->ExceptionClear();
        return "null";
    }

    env->DeleteLocalRef(cls_SharedPreferences);
    env->DeleteLocalRef(jstrKey);
    env->DeleteLocalRef(jstrDef);
    string ret = jstringTostring(env, str);
    env->DeleteLocalRef(str);
    return ret;
}

//ignore_
jstring getMyPackageName(JNIEnv *env, jstring befPname) {
    string str_befPname = jstringTostring(env, befPname);

    LOGE("getMyPackageName:::%s ", str_befPname.c_str());
    //trace contains native check
    jclass clz_throwable = env->FindClass("java/lang/Throwable");
    jmethodID mid_init = env->GetMethodID(clz_throwable, "<init>", "()V");
    jobject obj_throwable = env->NewObject(clz_throwable, mid_init);

    jclass clz_Log = env->FindClass("android/util/Log");
    jmethodID mid_getTrace = env->GetStaticMethodID(clz_Log, "getStackTraceString",
                                                    "(Ljava/lang/Throwable;)Ljava/lang/String;");
    jstring trace = (jstring) env->CallStaticObjectMethod(clz_Log, mid_getTrace, obj_throwable);
    string traceStr = jstringTostring(env, trace);
    //del
    env->DeleteLocalRef(clz_throwable);
    env->DeleteLocalRef(obj_throwable);
    env->DeleteLocalRef(clz_Log);
    env->DeleteLocalRef(trace);
    //get
    if (traceStr.find("nativeLoad") != string::npos) {
        jobject ctx = getContext(env);
        jobject sp = getSp(env, ctx);
        string pname = spGet(env, sp, "transfer_pname", str_befPname.c_str());
        env->DeleteLocalRef(ctx);
        env->DeleteLocalRef(sp);
        LOGE("nativeLoad , 传入：%s,返回：%s", str_befPname.c_str(), pname.c_str());
//        return  env->NewStringUTF(pname.c_str());
        return env->NewStringUTF("com.joym.armorhero.lt");
    }
    return befPname;
}

jobject getMyApplication(JNIEnv *env, jstring packageName, jint flags) {
    string str_packageName = jstringTostring(env, packageName);
    LOGE("getMyApplication:::%s,%d", str_packageName.c_str(), flags);
    jobject ctx = getContext(env);
    jobject sp = getSp(env, ctx);
    string pname = spGet(env, sp, "transfer_pname", "");
    if (pname == str_packageName) {
        string pkgPath = spGet(env, sp, "transfer_pkgPath", "");
        //modify
        //ApplicationInfo ai =  ctx.getApplicationInfo();
        jclass clz_ctx = env->GetObjectClass(ctx);
        jmethodID mid_getApplicationInfo = env->GetMethodID(clz_ctx, "getApplicationInfo",
                                                            "()Landroid/content/pm/ApplicationInfo;");
        jobject obj_appInfo = env->CallObjectMethod(ctx, mid_getApplicationInfo);
        //ApplicationInfo applicationInfo = new ApplicationInfo(ai);
        jclass clz_applicationInf = env->GetObjectClass(obj_appInfo);
        jmethodID mid_init = env->GetMethodID(clz_applicationInf, "<init>",
                                              "(Landroid/content/pm/ApplicationInfo;)V");
        jobject obj_applicationInfo = env->NewObject(clz_applicationInf, mid_init, obj_appInfo);
        // applicationInfo.sourceDir = model.getPkPath();
        jfieldID fid_sDir = env->GetFieldID(clz_applicationInf, "sourceDir", "Ljava/lang/String;");
        jstring sdir = env->NewStringUTF(pkgPath.c_str());
        env->SetObjectField(obj_applicationInfo, fid_sDir, sdir);

        //del
        env->DeleteLocalRef(clz_ctx);
        env->DeleteLocalRef(obj_appInfo);
        env->DeleteLocalRef(clz_applicationInf);
        env->DeleteLocalRef(sdir);
        return obj_applicationInfo;
    }
    //origin
    jclass clz_ctx = env->GetObjectClass(ctx);

    //ctx.getPackageManager()
    jmethodID mid_getPm = env->GetMethodID(clz_ctx, "getPackageManager",
                                           "()Landroid/content/pm/PackageManager;");
    jobject obj_pm = env->CallObjectMethod(ctx, mid_getPm);
    //getApplicationInfo(packageName, flags)
    jclass clz_pm = env->GetObjectClass(obj_pm);
    jmethodID mid_getApinfo = env->GetMethodID(clz_pm, "getApplicationInfo",
                                               "(Ljava/lang/String;I)Landroid/content/pm/ApplicationInfo;");
    jobject obj_ret = env->CallObjectMethod(obj_pm, mid_getApinfo, packageName,
                                            flags * -1);//暗示hook处放行

    //del
    env->DeleteLocalRef(clz_ctx);
    env->DeleteLocalRef(obj_pm);
    env->DeleteLocalRef(clz_pm);
    return obj_ret;
}


jobject getMyApplication4onlyPath(JNIEnv *env, jstring packageName, jint flags) {
    jobject ctx = getContext(env);
    LOGE("ctx: %x", ctx);
    jobject sp = getCommonSp(env, ctx);
    string pkgPath = spGet(env, sp, "transfer_pkgPath", "");
    LOGE("new PackagePath :%s", pkgPath.c_str());
    if (pkgPath != "") {
        //modify
        //ApplicationInfo ai =  ctx.getApplicationInfo();
        jclass clz_ctx = env->GetObjectClass(ctx);
        jmethodID mid_getApplicationInfo = env->GetMethodID(clz_ctx, "getApplicationInfo",
                                                            "()Landroid/content/pm/ApplicationInfo;");
        jobject obj_appInfo = env->CallObjectMethod(ctx, mid_getApplicationInfo);
        //ApplicationInfo applicationInfo = new ApplicationInfo(ai);
        jclass clz_applicationInf = env->GetObjectClass(obj_appInfo);
        jmethodID mid_init = env->GetMethodID(clz_applicationInf, "<init>",
                                              "(Landroid/content/pm/ApplicationInfo;)V");
        jobject obj_applicationInfo = env->NewObject(clz_applicationInf, mid_init, obj_appInfo);
        // applicationInfo.sourceDir = model.getPkPath();
        jfieldID fid_sDir = env->GetFieldID(clz_applicationInf, "sourceDir", "Ljava/lang/String;");
        jstring sdir = env->NewStringUTF(pkgPath.c_str());
        env->SetObjectField(obj_applicationInfo, fid_sDir, sdir);

        //del
        env->DeleteLocalRef(clz_ctx);
        env->DeleteLocalRef(obj_appInfo);
        env->DeleteLocalRef(clz_applicationInf);
        env->DeleteLocalRef(sdir);
        return obj_applicationInfo;
    }

    //origin
    jclass clz_ctx = env->GetObjectClass(ctx);

    //ctx.getPackageManager()
    jmethodID mid_getPm = env->GetMethodID(clz_ctx, "getPackageManager",
                                           "()Landroid/content/pm/PackageManager;");
    jobject obj_pm = env->CallObjectMethod(ctx, mid_getPm);
    //getApplicationInfo(packageName, flags)
    jclass clz_pm = env->GetObjectClass(obj_pm);
    jmethodID mid_getApinfo = env->GetMethodID(clz_pm, "getApplicationInfo",
                                               "(Ljava/lang/String;I)Landroid/content/pm/ApplicationInfo;");
    jobject obj_ret = env->CallObjectMethod(obj_pm, mid_getApinfo, packageName,
                                            flags * -1);//暗示hook处放行

    //del
    env->DeleteLocalRef(clz_ctx);
    env->DeleteLocalRef(obj_pm);
    env->DeleteLocalRef(clz_pm);
    return obj_ret;

}