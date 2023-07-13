#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_bonedust_standalone_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Project Standalone!";
    return env->NewStringUTF(hello.c_str());
}