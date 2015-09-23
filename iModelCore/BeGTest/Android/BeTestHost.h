/*--------------------------------------------------------------------------------------+
|
|     $Source: Android/BeTestHost.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#include <Bentley/BeTest.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeFileName.h>
#include <Logging/bentleylogging.h>
#include <stdio.h>

#include <jni.h>

/*---------------------------------------------------------------------------------**//**
* This class knows how data files are linked into the Product/BeGTest directory structure.
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeTestHost : RefCounted<BeTest::Host>
    {
private:
    JNIEnv*    m_env;
    jobject    m_obj;

    BeTestHost ();
    void _GetDocumentsRoot (BeFileName& path) override;
    void _GetDgnPlatformAssetsDirectory (BeFileName& path) override;
    void _GetOutputRoot (BeFileName& path) override;
    void _GetTempDir (BeFileName& path) override;
    void _GetFrameworkSqlangFiles (BeFileName& path) override;
    void*  _InvokeP (char const* function, void* args) override;
   
public:
    //  Called by {GenJUnitTestCase.jni.cpp}Java_com_bentley_unittest___REPO___jniTest* functions
    void         SetJniEnvAndObject (JNIEnv*  env, jobject  obj);

    //  Called by {begtest_static_initialize.cpp}begtest_initialize
    static RefCountedPtr<BeTestHost> Create ();
    };
