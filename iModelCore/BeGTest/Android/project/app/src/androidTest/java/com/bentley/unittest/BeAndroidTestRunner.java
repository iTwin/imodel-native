package com.bentley.unittest;

import java.lang.reflect.Method;

import junit.framework.AssertionFailedError;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestListener;

import android.app.Activity;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.test.AndroidTestRunner;
import android.util.Log;

import com.bentley.test.TestActivity;

public class BeAndroidTestRunner extends AndroidTestRunner implements TestListener
    {
    private Class<? extends TestCase> m_currentClass = null;

    /**
     * Constructor. Attaches itself as a test listener.
     */
    public BeAndroidTestRunner()
        {
        addTestListener(this);
        }

    /**
     * Called before each test run. Makes sure that test fixtures get set up and torn down.
     *
     * @param test Test case to be run.
     */
    @Override
    public void startTest(Test test)
        {
        ActivityInstrumentationTestCase2<TestActivity> testCase = (ActivityInstrumentationTestCase2<TestActivity>) test;
        if (null != testCase)
            {
            TestActivity activity = testCase.getActivity();
            Context context = activity.getApplication().getApplicationContext();
            activity.extractAssets();
            TestActivity.initialize(context);

            if (m_currentClass != testCase.getClass())
                {
                if (null != m_currentClass)
                    tearDownTestCase(m_currentClass);

                m_currentClass = testCase.getClass();
                setUpTestCase(m_currentClass);
                }
            }
        }

    /**
     * Runs all tests and makes sure that the last test fixture gets torn down.
     */
    @Override
    public void runTest()
        {
        super.runTest();

        if (null != m_currentClass)
            {
            tearDownTestCase(m_currentClass);
            m_currentClass = null;
            }
        }

    /**
     * Runs set up function of the specified test fixture (if it has one).
     *
     * @param clazz Java test class.
     */
    private void setUpTestCase(Class<? extends TestCase> clazz)
        {
        try
            {
            Method setUpMethod = clazz.getDeclaredMethod("setUpTestCase", (Class<?>[]) null);
            setUpMethod.invoke(null, (Object[]) null);
            }
        catch (NoSuchMethodException e)
            {
            Log.d(this.getClass().getSimpleName(), clazz.getSimpleName() + " does not have TC_SETUP");
            }
        catch (Exception e)
            {
            e.printStackTrace();
            }
        }

    /**
     * Runs tear down function of the specified test fixture (if it has one).
     *
     * @param clazz Java test class.
     */
    private void tearDownTestCase(Class<? extends TestCase> clazz)
        {
        try
            {
            Method tearDownMethod = clazz.getDeclaredMethod("tearDownTestCase", (Class<?>[]) null);
            tearDownMethod.invoke(null, (Object[]) null);
            }
        catch (NoSuchMethodException e)
            {
            Log.d(this.getClass().getSimpleName(), clazz.getSimpleName() + " does not have TC_TEARDOWN");
            }
        catch (Exception e)
            {
            e.printStackTrace();
            }
        TestActivity.uninitializeSqlang();
        }

    public void addError(Test test, Throwable exception)
        {
        }

    public void addFailure(Test test, AssertionFailedError error)
        {
        }

    public void endTest(Test test)
        {
        }
    }
