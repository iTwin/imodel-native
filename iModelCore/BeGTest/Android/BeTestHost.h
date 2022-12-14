/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeFileName.h>
#include <Bentley/Logging.h>
#include <stdio.h>

#include <jni.h>

/*---------------------------------------------------------------------------------**//**
* This class knows how data files are linked into the Product/BeGTest directory structure.
* @bsimethod
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
    void*  _InvokeP (char const* function, void* args) override;

public:
    //  Called by {GenJUnitTestCase.jni.cpp}Java_com_bentley_unittest___REPO___jniTest* functions
    void         SetJniEnvAndObject (JNIEnv*  env, jobject  obj);

    //  Called by {begtest_static_initialize.cpp}begtest_initialize
    static RefCountedPtr<BeTestHost> Create ();
    };
