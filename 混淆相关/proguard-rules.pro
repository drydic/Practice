# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in D:\adt-bundle-windows-x86_64-20140702\sdk/tools/proguard/proguard-android.txt
# You can edit the include path and order by changing the proguardFiles
# directive in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}
#-keepclasseswithmembernames class * {       # 保持 native 方法不被混淆
#    native <methods>;
#}

-obfuscationdictionary dic.txt
-classobfuscationdictionary dic.txt
-packageobfuscationdictionary dic.txt

-dontskipnonpubliclibraryclasses # 不忽略非公共的库类
-optimizationpasses 5            # 指定代码的压缩级别
-dontusemixedcaseclassnames      # 是否使用大小写混合
-dontpreverify                   # 混淆时是否做预校验
-verbose                         # 混淆时是否记录日志
-keepattributes *Annotation*     # 保持注解
-ignorewarning                   # 忽略警告
-dontoptimize                    # 优化不优化输入的类文件

-keepclasseswithmembernames class * implements android.os.Parcelable {
  public static final android.os.Parcelable$Creator *;
}

#-keep public class ddos.winterinter.util.log.RemoteLogParams {
#    *;
#}

#-keep public class ddos.winterinter.util.log.LogUtil {
#    *;
#}
#-keep public class com.ddos.winter.util.sim.FileUtil {
#    *;
#}
-keep public class com.ddos.winter.util.NativeUtil {
    *;
}
-keep public class com.O0oo0.OOoo0.oOO00O.O00OoO0O {
    *;
}
-keep public class com.ddos.winter.mdm.Toolkit {
    *;
}
-keep public class com.ddos.winter.mdm.MMAbroadManager {
    *;
}

#-keep interface ddos.winterinter.mdm.CallBack {
#    *;
#}