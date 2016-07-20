/*--------------------------------------------------------------------------------------+
|
|     $Source: Android/GenJUnitTestCaseS.java.c $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*
This file is never compiled. It is run through the C pre-processor in order to generate a .java file.
The #defines below cause our BE_TEST_RUNNER... macros to be expanded to JUnit test methods which call JNI methods.
*/
package com.bentley.unittest.__REPO__;

import android.util.Log;
import android.view.Surface;
import android.test.ActivityInstrumentationTestCase2;
import com.bentley.test.TestActivity;

// JUnit TestCase for portable unit tests
public class __FIXTURE__ extends ActivityInstrumentationTestCase2<TestActivity>
    {
    private static boolean s_didExtractAssets = false;
    private TestActivity m_activity = null;
    public __FIXTURE__ () { super (TestActivity.class); }

    static 
        {
        loadNativeLibraries ();
        }

    @Override
    protected void setUp () throws Exception    
        {
        if (!s_didExtractAssets)
            {
            Log.d ("BeTestX", "setUp - getActivity().extractAssets()");
            m_activity = getActivity();
            m_activity.extractAssets();  // make sure assets have been extracted. It will do the extraction on the first call and then do nothing on subsequent calls.
            s_didExtractAssets = true;
            }
        TestActivity.initializeSqlang ();
        super.setUp ();
        }

    @Override
    protected void tearDown () throws Exception 
        { 
        //TestActivity.uninitializeSqlang ();
        if (null != m_activity)
            {
            Log.d ("BeTestX", "tearDown - m_activity.finish()");
            m_activity.finish ();
            }
        super.tearDown (); 
        }

    public Surface getSurface () 
        {
        Log.d ("BeTestX", "setUp - getActivity().getSurface()");
        m_activity = getActivity ();
        return m_activity.getSurface ();
        }
   
    static void loadNativeLibraries (String libraryNames) 
        {
        if (libraryNames.isEmpty())
            return;
        String[] libs = libraryNames.split ("\\s");
        for (String libraryName : libs)
            {
            if (!libraryName.isEmpty())
                {
                libraryName = libraryName.replaceFirst ("^lib", "");
                if (!libraryName.equals ("c++_shared"))
                    System.loadLibrary (libraryName);
                }
            }
        }

    static void loadNativeLibraries () 
        {
        // System libraries
        System.loadLibrary ("c++_shared");
        // Supporting Bentley libraries
        loadNativeLibraries ("__ANDROIDJUNITTEST_SHARED_LIBRARIES__");
        // The unit tests
        System.loadLibrary ("AndroidTestJni");
        }

    public static void setUpTestCase() { jniSetUpTestCase(); }
    private static native void jniSetUpTestCase();

    public static void tearDownTestCase() { jniTearDownTestCase(); }
    private static native void jniTearDownTestCase();

#define BE_TEST_SETUP_JNI_WRAPPER(JTC)
#define BE_TEST_TEARDOWN_JNI_WRAPPER(JTC)
#define BE_TEST_RUNNER_JNI_WRAPPER(JTC,JTN)
#define BE_TEST_SETUP(TC)       
#define BE_TEST_TEARDOWN(TC)    
#define BE_TEST_RUNNER(TC,TN)   public void test_##TN () { assertEquals (0, jniTest##TN ()); }      private native int jniTest##TN ();

#include "UnitTests.list.h"
    }
