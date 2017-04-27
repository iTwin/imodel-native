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
jobject
Java_com_bentley_loadprojects_ConnectJNI_GetProjects (
        JNIEnv *env,
        jobject /* this */) {

    std::map<wstring, wstring> list;
    ConnectInterfaceNative connectInterface;
    int count = connectInterface.GetProjects(&list);

    jclass hashMapClass= env->FindClass("java/util/HashMap");
    jmethodID hashMapInit = env->GetMethodID( hashMapClass, "<init>", "(I)V");
    jobject hashMapObj = env->NewObject( hashMapClass, hashMapInit, list.size());
    jmethodID hashMapOut = env->GetMethodID( hashMapClass, "put",
                                            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");


    int i = 0;
    for (std::map<wstring, wstring> ::iterator theIterator = list.begin(); theIterator != list.end();theIterator++) {
            std::wstring id = theIterator->first;
            Utf8String utf8StringId;
            BeStringUtilities::WCharToUtf8(utf8StringId, id.c_str());
            jstring jstrId = env->NewStringUTF(utf8StringId.c_str());

            std::wstring name = theIterator->second;
            Utf8String utf8StringName;
            BeStringUtilities::WCharToUtf8(utf8StringName, name.c_str());
            jstring jstrName = env->NewStringUTF(utf8StringName.c_str());

            env->CallObjectMethod(hashMapObj, hashMapOut, jstrId, jstrName);

            env->DeleteLocalRef(jstrId);
            env->DeleteLocalRef(jstrName);
            i++;
        }

    return hashMapObj;
}

extern "C"
jobject
Java_com_bentley_loadprojects_ConnectJNI_GetProjectProperties (
        JNIEnv *env,
        jobject /* this */,
        jstring projectIdJ) {

    std::wstring projectIdW = Java_To_WStr(env, projectIdJ);

    std::map<int, wstring> propertyMap;
    ConnectInterfaceNative connectInterface;
    int count = connectInterface.GetProjectPropertyMapV4(projectIdW, &propertyMap);

    jclass hashMapClass = env->FindClass("java/util/HashMap");
    jmethodID hashMapInit = env->GetMethodID(hashMapClass, "<init>", "(I)V");
    jobject hashMapObj = env->NewObject(hashMapClass, hashMapInit, propertyMap.size());
    jmethodID hashMapOut = env->GetMethodID(hashMapClass, "put",
                                            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    int i = 0;
    for (std::map<int, wstring> ::iterator theIterator = propertyMap.begin(); theIterator != propertyMap.end();theIterator++) {
        int id = theIterator->first;
        jclass cls = env->FindClass("java/lang/Integer");
        jmethodID midInit = env->GetMethodID(cls, "<init>", "(I)V");
        if (NULL == midInit) return NULL;
        jobject jIntId = env->NewObject(cls, midInit, id);

        /*jint jintId = id;
        char strId[64] = "\0";
        snprintf(strId, 64, "%d", id);
        jstring jstrId = env->NewStringUTF(strId);
        */

        std::wstring val = theIterator->second;
        Utf8String utf8StringVal;
        BeStringUtilities::WCharToUtf8(utf8StringVal, val.c_str());
        jstring jstrName = env->NewStringUTF(utf8StringVal.c_str());

        env->CallObjectMethod(hashMapObj, hashMapOut, jIntId, jstrName);

        env->DeleteLocalRef(jIntId);
        env->DeleteLocalRef(jstrName);
        i++;
    }

    return hashMapObj;
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
