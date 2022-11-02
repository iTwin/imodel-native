/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BeTestHost.h"
#include <Bentley/BeJStringUtilities.h>
#include <android/native_window_jni.h>
#include <android/log.h>

#define UTF8CSTR(WSTR) Utf8String(WSTR).c_str()

static BeFileName s_assetsDir;
static BeFileName s_docsDir;
static BeFileName s_tempDir;
static BeFileName s_localStateDir;
static BeFileName s_testOutputRoot;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTestHost::BeTestHost ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTestHost::_GetDocumentsRoot (BeFileName& path)
    {
    path = s_docsDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTestHost::_GetDgnPlatformAssetsDirectory (BeFileName& path)
    {
    //
    path = s_assetsDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTestHost::_GetOutputRoot (BeFileName& path)
    {
    //  The "local state directory" is a writable directory that is private to the activity.
    //  Android does not automatically delete files or clean up this directory.
    //  So, it's a good place to create documents that are manipulated by the tests.
    //  We need to isolate the test scratch documents in their own subdirectory, however, so that
    //  we can clean them up at the start of the run.
    path = s_testOutputRoot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTestHost::_GetTempDir (BeFileName& path)
    {
    path = s_tempDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTestHost::SetJniEnvAndObject (JNIEnv* env, jobject obj) {m_env = env; m_obj = obj;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void* BeTestHost::_InvokeP (char const* function, void* args)
    {
    if (0==strncmp (function, "foo", 3))
        {
        jmethodID   jmid        = m_env->GetMethodID (m_env->GetObjectClass(m_obj), "getName", "()Ljava/lang/String;");
        jstring     jstr        = (jstring) m_env->CallObjectMethod (m_obj, jmid);
        CharCP      jstrbytes   = m_env->GetStringUTFChars (jstr, NULL);
        Utf8String* utf8        = new Utf8String (jstrbytes);
        m_env->ReleaseStringUTFChars (jstr, jstrbytes);
        return utf8;
        }
    else if (0 == strncmp (function, "getSurface", 15))
        {
        jmethodID jmid     = m_env->GetMethodID (m_env->GetObjectClass (m_obj), "getSurface", "()Landroid/view/Surface;");
        jobject   jsurface = (jobject) m_env->CallObjectMethod (m_obj, jmid);
        return jsurface;
        }
    else if (0 == strncmp (function, "getEnv", 6))
        {
        return m_env;
        }
    else if (0 == strncmp (function, "getObj", 6))
        {
        return m_obj;
        }
    else if (0 == strncmp (function, "getClass", 8))
        {
        return m_env->GetObjectClass (m_obj);
        }

    NativeLogging::CategoryLogger("TestRunner").errorv ("Unknown upcall %hs", function);
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<BeTestHost> BeTestHost::Create ()
    {
    return new BeTestHost;
    }

static void recreateDir(BeFileNameCR dirName)
    {
    BeFileName::EmptyAndRemoveDirectory(dirName.c_str());
    BeFileName::CreateNewDirectory(dirName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" void Java_com_bentley_test_TestActivity_initializeJni
(
JNIEnv*         env,
jclass,
jstring         assetsDir,
jstring         docsDir,
jstring         tempDir,
jstring         localStateDir
)
    {
#ifdef DEBUG_BETEST_STARTUP
    fprintf(stderr, "Java_com_bentley_test_TestActivity_initializeJni. Waiting");
    int i = 0;
    static bool s_loop = true;
    while (s_loop)
        {
        //            BeThreadUtilities::BeSleep (1);
        if (i++ % 10000 == 0)
            printf(".");
        }
#endif

    BeJStringUtilities::InitWStringFromJString (env, s_assetsDir, assetsDir);
    BeJStringUtilities::InitWStringFromJString (env, s_docsDir, docsDir);
    BeJStringUtilities::InitWStringFromJString (env, s_tempDir, tempDir);
    BeJStringUtilities::InitWStringFromJString (env, s_localStateDir, localStateDir);

    s_testOutputRoot = s_localStateDir;
    s_testOutputRoot.AppendToPath(L"Output").AppendSeparator();

    static bool s_cleaned;
    if (!s_cleaned)
        {
        // Delete any files that may be hanging around since the last run
        s_cleaned = true;
        BeFileName::EmptyDirectory(s_tempDir);
        recreateDir(s_testOutputRoot);
        }

    // ensure known locations have trailing separators for consistency with DgnClientFx
    s_assetsDir.AppendSeparator();
    s_docsDir.AppendSeparator();
    s_tempDir.AppendSeparator();
    s_localStateDir.AppendSeparator();

    __android_log_print (ANDROID_LOG_INFO, "TestRunner", "initializeJni (assetsDir=%s, docsDir=%s, tempDir=%s, localStateDir=%s)",
                                            UTF8CSTR(s_assetsDir),
                                            UTF8CSTR(s_docsDir),
                                            UTF8CSTR(s_tempDir),
                                            UTF8CSTR(s_localStateDir));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" int begtest_initialize ()
    {
    try {
        BeTest::Initialize (*BeTestHost::Create ());
        }
    catch (...)
        {
        __android_log_print (ANDROID_LOG_FATAL, "TestRunner", "Exception caught during BeTest::Initialize or NativeLogging initialization.");
        return -1;
        }
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" void begtest_uninitialize ()
    {
    BeTest::Uninitialize();
    }
