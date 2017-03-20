#include <jni.h>
#include <string>
#include "ConnectInterfaceNative.h"

static JavaVM*          s_jvm;

std::wstring Java_To_WStr(JNIEnv *env, jstring string) {

    std::wstring value;
    const jchar *raw = env->GetStringChars(string, 0);
    jsize len = env->GetStringLength(string);
    value.assign(raw, raw + len);
    env->ReleaseStringChars(string, raw);
    return value;
}

extern "C"
jstring
Java_com_bentley_loadprojects_ConnectJNI_GetConnectUser(
        JNIEnv *env,
        jobject /* this */) {

    ConnectInterfaceNative connectInterface;
    std::string hello = "Results - " + connectInterface.GetConnectUserName();
    return env->NewStringUTF(hello.c_str());
}

extern "C"
jobjectArray
Java_com_bentley_loadprojects_ConnectJNI_GetProjects (
        JNIEnv *env,
        jobject /* this */) {

    std::list<wstring> list;
    ConnectInterfaceNative connectInterface;
    int count = connectInterface.GetProjects(&list);
    jstring emptyString = env->NewStringUTF("");
    jobjectArray result = (jobjectArray)env->NewObjectArray(count, env->FindClass("java/lang/String"), emptyString);

    int i = 0;
    for (std::list<wstring> ::iterator theIterator = list.begin(); theIterator != list.end();theIterator++) {
            std::wstring name = *theIterator;
            Utf8String utf8String;
            BeStringUtilities::WCharToUtf8(utf8String, name.c_str());
            jstring jstrName = env->NewStringUTF(utf8String.c_str());
            env->SetObjectArrayElement(result, i, jstrName);
            env->DeleteLocalRef(jstrName);
            i++;
        }

    env->DeleteLocalRef(emptyString);
    return result;
}

extern "C"
void
Java_com_bentley_loadprojects_ConnectJNI_Initialize(
        JNIEnv *env,
        jobject /* this */,
        jstring         appDirJ,
        jstring         tempDirJ,
        jstring         externalStorageDirJ,
        jstring         deviceIdJ) {

    if (s_jvm == nullptr)
        env->GetJavaVM (&s_jvm);

    std::wstring appDirW = Java_To_WStr(env, appDirJ);
    std::wstring tempDirW = Java_To_WStr(env, tempDirJ);
    std::wstring externalStorageDirW = Java_To_WStr(env, externalStorageDirJ);
    std::wstring deviceIdW = Java_To_WStr(env, deviceIdJ);

    ConnectInterfaceNative::Initialize(appDirW, tempDirW, externalStorageDirW, deviceIdW, env);
}
