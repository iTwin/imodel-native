/*--------------------------------------------------------------------------------------+
|
|     $Source: Android/GenJUnitTestCase.jni.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/* 
Example of how to create JNI wrappers for our unit tests.
change "MYPACKAGE" and "__REPO__" below to match the actual JUnit test package and class name that you are building.
*/

#include <jni.h>
#include <android/log.h>
#include <Bentley/BeTest.h>
#include <BeTestHost.h>

extern "C" int begtest_initialize ();
extern "C" void begtest_uninitialize ();

#undef  BE_TEST_SETUP_JNI_WRAPPER               
#undef  BE_TEST_SETUP
#define BE_TEST_SETUP_JNI_WRAPPER(JTC)          extern "C" EXPORT_ATTRIBUTE void Java_com_bentley_unittest___REPO___##JTC##_jniSetUpTestCase (JNIEnv*,jclass) {
#define BE_TEST_SETUP(TC)                       if (begtest_initialize () == 0) BeTest::SetUpTestCase(#TC); }

#undef  BE_TEST_TEARDOWN_JNI_WRAPPER
#undef  BE_TEST_TEARDOWN
#define BE_TEST_TEARDOWN_JNI_WRAPPER(JTC)       extern "C" EXPORT_ATTRIBUTE void Java_com_bentley_unittest___REPO___##JTC##_jniTearDownTestCase (JNIEnv*,jclass) {
#define BE_TEST_TEARDOWN(TC)                    BeTest::TearDownTestCase(#TC); begtest_uninitialize (); }

#undef  BE_TEST_RUNNER_JNI_WRAPPER
#undef  BE_TEST_RUNNER
#define BE_TEST_RUNNER_JNI_WRAPPER(JTC,JTN)     extern "C" EXPORT_ATTRIBUTE jint Java_com_bentley_unittest___REPO___##JTC##_jniTest##JTN (JNIEnv* jni, jobject jobj) { \
                                                    ((BeTestHost&) BeTest::GetHost ()).SetJniEnvAndObject (jni, jobj); 
#define BE_TEST_RUNNER(TC,TN)                       extern int BETEST_TEST_RUNNER_FUNC(TC,TN) (); try { return BETEST_TEST_RUNNER_FUNC(TC,TN) (); } catch (...) {return 1;} }
#include "__REPO__.list.h"
