/*--------------------------------------------------------------------------------------+
|
|     $Source: Android/GenJUnitTestCase.jni.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/* 
Example of how to create JNI wrappers for our unit tests.
change "MYPACKAGE" and "__REPO__" below to match the actual JUnit test package and class name that you are building.
*/

#include <jni.h>
#include <android/log.h>
#include <Bentley/BeTest.h>
#include <BeTestAndroid/BeTestHost.h>

extern "C" int begtest_initialize ();
extern "C" void begtest_uninitialize ();

// *_JNI_WRAPPER defines
#undef  BE_TEST_SETUP_JNI_WRAPPER
#define BE_TEST_SETUP_JNI_WRAPPER(JTC)
#undef  BE_TEST_TEARDOWN_JNI_WRAPPER
#define BE_TEST_TEARDOWN_JNI_WRAPPER(JTC)
#undef  BE_TEST_RUNNER_JNI_WRAPPER
#define BE_TEST_RUNNER_JNI_WRAPPER(JTC,JTN)

// declare the actual test case setup and teardown functions
#undef  BE_TEST_SETUP
#define BE_TEST_SETUP(TC)                       extern "C" void setUpTestCase_##TC ();
#undef  BE_TEST_TEARDOWN
#define BE_TEST_TEARDOWN(TC)                    extern "C" void tearDownTestCase_##TC ();

// declare the actual  test runner functions
#undef  BE_TEST_RUNNER
#define BE_TEST_RUNNER(TC,TN)                   extern "C" int  run_TEST_##TC##_##TN ();
#include "__REPO__.list.h"

// define jni functions that call test case setup and teardown functions
#undef  BE_TEST_SETUP_JNI_WRAPPER               
#undef  BE_TEST_SETUP
#define BE_TEST_SETUP_JNI_WRAPPER(JTC)          extern "C" void Java_com_bentley_unittest___REPO___##JTC##_jniSetUpTestCase (JNIEnv*,jclass) {
#define BE_TEST_SETUP(TC)                       if (begtest_initialize () == 0) setUpTestCase_##TC (); }

#undef  BE_TEST_TEARDOWN_JNI_WRAPPER
#undef  BE_TEST_TEARDOWN
#define BE_TEST_TEARDOWN_JNI_WRAPPER(JTC)       extern "C" void Java_com_bentley_unittest___REPO___##JTC##_jniTearDownTestCase (JNIEnv*,jclass) {
#define BE_TEST_TEARDOWN(TC)                    tearDownTestCase_##TC (); begtest_uninitialize (); }

// define jni functions that call the test runner functions
#undef  BE_TEST_RUNNER_JNI_WRAPPER
#undef  BE_TEST_RUNNER
#define BE_TEST_RUNNER_JNI_WRAPPER(JTC,JTN)     extern "C" jint Java_com_bentley_unittest___REPO___##JTC##_jniTest##JTN (JNIEnv* jni, jobject jobj) { \
                                                    if (begtest_initialize() != 0) return 1; \
                                                    ((BeTestHost&) BeTest::GetHost ()).SetJniEnvAndObject (jni, jobj); 
#define BE_TEST_RUNNER(TC,TN)                       try { return run_TEST_##TC##_##TN(); } catch (...) {return 1;} \
                                                    begtest_uninitialize(); \
                                                }
#include "__REPO__.list.h"
