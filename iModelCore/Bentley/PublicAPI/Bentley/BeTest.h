/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeTest.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/// @cond BENTLEY_SDK_Internal

#include "Bentley.h"
#include "bvector.h"
#include "bset.h"
#include "BeFile.h"
#include "WString.h"
#include "BeFileName.h"
#include "BeFileListIterator.h"
#include "BeStringUtilities.h"
#include "BeAssert.h"
#include "RefCounted.h"
#include <Bentley/bvector.h>
#include <Logging/bentleylogging.h>
#include <string>

#ifdef BETEST_GENERATE_UNITTESTS_LIST_H
    #undef USE_GTEST
#endif

#define BE_TEST_BREAK_IN_DEBUGGER BeTest::BreakInDebugger();

#if defined (USE_GTEST)

    #if !defined (BETEST_NO_INCLUDE_GTEST)
        #if (_MSC_VER >= 1600) // 1600 => VC10
            #define GTEST_USE_OWN_TR1_TUPLE 0
        #endif

        #ifndef UNICODE
            #define UNICODE 1
            #define _UNICODE 1
        #endif
        #include <gtest/gtest.h>
        #include <gmock/gmock.h>

    #endif



#else

    #include <math.h>

    namespace testing
        {
        class Test
            {
            void    RunTest();
          protected:
            virtual void SetUp() {;}
            virtual void TearDown() {;}
            virtual void TestBody() {;}
          public:
            static void SetUpTestCase() {;}
            static void TearDownTestCase() {;}
            Test() {;}
            virtual ~Test() {;}
            BENTLEYDLL_EXPORT void Run();
            virtual BentleyApi::CharCP  GetTestCaseNameA () const = 0;
            virtual BentleyApi::CharCP  GetTestNameA () const = 0;
            };
        }

    #define BETEST_TEST_CLASS_NAME(testCaseName, testName) TEST_##testCaseName##_##testName
    #define BETEST_TEST_RUNNER_FUNC(testCaseName, testName) run_TEST_##testCaseName##_##testName

    #define DEFINE_BETEST_INTERNAL(superTestName, testCaseName, testName)                   \
        struct BETEST_TEST_CLASS_NAME(testCaseName,testName) : superTestName                \
        {                                                                                   \
            BentleyApi::CharCP  GetTestCaseNameA () const { return #testCaseName; }         \
            BentleyApi::CharCP  GetTestNameA ()     const { return #testName; }             \
            virtual void TestBody () override;                                              \
            static BeTest::TestCaseInfo* s_superClassTestCaseInfo;                          \
        };                                                                                  \
    BeTest::TestCaseInfo* BETEST_TEST_CLASS_NAME(testCaseName,testName)::s_superClassTestCaseInfo = BeTest::RegisterTestCase(#testCaseName, & superTestName :: SetUpTestCase, & superTestName :: TearDownTestCase);\
    extern "C" EXPORT_ATTRIBUTE int BETEST_TEST_RUNNER_FUNC(testCaseName,testName) ()       \
        {                                                                                   \
        size_t e = BeTest::GetErrorCount();                                                 \
        if (BeTest::ShouldRunTest (#testCaseName "." #testName))                            \
            {                                                                               \
            BeTest::SetNameOfCurrentTestInternal(#testCaseName,#testName);                  \
            BETEST_TEST_CLASS_NAME(testCaseName,testName) t;                                \
            t.Run();                                                                        \
            }                                                                               \
        return BeTest::GetErrorCount() > e;                                                 \
        }                                                                                   \
    void BETEST_TEST_CLASS_NAME(testCaseName,testName) :: TestBody ()


    #if !defined(BETEST_GENERATE_UNITTESTS_LIST_H)
        #undef  TEST_F
        #define TEST_F(testCaseName, testName) DEFINE_BETEST_INTERNAL(testCaseName,testCaseName,testName)

        #undef  TEST
        #define TEST(testCaseName, testName) DEFINE_BETEST_INTERNAL(testing::Test,testCaseName,testName)
    
    #endif

    #define RUN_ALL_TESTS BeTest::RunAllTests
    
#endif

BEGIN_BENTLEY_NAMESPACE

/*=================================================================================**//**
* Portable unit test utility functions, not specific to any one test harness.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BeTest
{
    static bool s_loop;
///@name Platform-specific up-calls
///@{

//! Interface to be adopted by a unit test runner host
struct Host : IRefCounted
    {
protected:
    Host() {;}  // hide constructor of ref-counted class

    // Up-calls
    virtual void*  _InvokeP (char const* function, void* args) = 0;

    // Data directories
    virtual void _GetDocumentsRoot (BeFileName& path) = 0;
    virtual void _GetDgnPlatformAssetsDirectory (BeFileName& path) = 0;
    virtual void _GetOutputRoot (BeFileName& path) = 0;
    virtual void _GetTempDir (BeFileName& path) = 0;
    virtual void _GetFrameworkSqlangFiles(BeFileName& path) = 0;

public:
    //! Call a function in the native test harness
    //! @param[in] function ID of a function call that should be executed by the native test harness
    //! @param[in] args     The arguments for the function call
    //! @return a pointer value
    BENTLEYDLL_EXPORT void*  InvokeP (char const* function, void* args = NULL);

    //! Where a unit test can find BIM and other data files
    BENTLEYDLL_EXPORT void GetDocumentsRoot (BeFileName& path);
    //! Where a unit test can find static resources such as fonts, schemas, and MUIs
    BENTLEYDLL_EXPORT void GetDgnPlatformAssetsDirectory (BeFileName& path);
    //! Where a unit test can write temporary data files
    BENTLEYDLL_EXPORT void GetOutputRoot (BeFileName& path);
    //! Where a unit test can purely temporary files. 
    //! @Note: Do not plan to store much in the temp directory. Files are deleted automatically when this directory gets too large. This directory is deleted entirely when the test finishes.
    //! @Note: Use GetOutputRoot for large files or files that should stick around for a long time.
    BENTLEYDLL_EXPORT void GetTempDir (BeFileName& path);

    //! Get the full filename of the "framework" db3 file
    BENTLEYDLL_EXPORT void GetFrameworkSqlangFiles(BeFileName& path);
    };

//! Query if BeTest is running
BENTLEYDLL_EXPORT static bool IsInitialized();

//! Get the test harness host
BENTLEYDLL_EXPORT static Host&  GetHost ();

///@}

//! Call this once from the main of the test runner program. Initialize does the following:
//! -- sets a BeAssert handler (which cannot be overridden).
//! -- sets platform-specific fault handling
//! -- sets the default IGetDataRoots helper (which can be overridden -- see SetIGetDataRoots)
//! @param[in] host     The test runner host
BENTLEYDLL_EXPORT static void Initialize (Host& host);

///@name TestCase setup/teardown
///@{

typedef void(*T_SetUpFunc)(void);
typedef void(*T_TearDownFunc)(void);

struct TestCaseInfo
    {
    int m_count;
    T_SetUpFunc m_setUp;
    T_TearDownFunc m_tearDown;
    };

///@}

struct IFailureHandler 
    {
    virtual void _OnAssertionFailure (WCharCP msg) THROW_SPECIFIER(CharCP) = 0;
    virtual void _OnUnexpectedResult (WCharCP msg) THROW_SPECIFIER(CharCP) = 0;
    virtual void _OnFailureHandled() = 0;
    };

// Call this to set the failure handler
BENTLEYDLL_EXPORT static void SetIFailureHandler (IFailureHandler&);

// Call this only from BeGTestExe.cpp!
BENTLEYDLL_EXPORT static void SetRunningUnderGtest ();

//! Call this after all tests have run.
BENTLEYDLL_EXPORT static void Uninitialize ();

///@name Assertion failures
///@{

//! Control how assertion failures of various types are handled.
//! @param atype    type of assertion failure
//! @param doFail   if true, the current test fails; otherwise, the assertion failure is merely logged
//! @todo This is not currently thread-safe. 1. This function may be called only on the main thread. It is ignored on other threads. 2. Changing the assertion-failure policy will affect how assertion failures are handled on all threads.
//! @remarks Even if SetFailOnAssert is set to true, an assertion failure in a worker thread never triggers a failure in that thread, and it does not stop the calling code from running.
//! Instead, the failure is logged and deferred. The failure is triggered later in the main thread, usually when the test finishes.
BENTLEYDLL_EXPORT static void SetFailOnAssert (bool doFail, BeAssertFunctions::AssertType atype = BeAssertFunctions::AssertType::All);

//! Returns a flag indicating the there was an assertion failure in the current test.
BENTLEYDLL_EXPORT static bool GetAssertionFailed();

//! Signature of an assertion failure listener. There can be many listeners. A listener does not control how assertion failure is handled by BeTest. Normally, a listener will log a message.
typedef void T_BeAssertListener (WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType atype);

//! Register an assertion failure listener
BENTLEYDLL_EXPORT static void SetBeAssertListener (T_BeAssertListener*);

///@}

///@name Information about the currently running test 
///@{

//! Get name of currently running test. 
//! @return the name of a test or "" if no test is running
static Utf8CP GetNameOfCurrentTest()
    {
#if defined (USE_GTEST)
    ::testing::TestInfo const* tinfo = ::testing::UnitTest::GetInstance()->current_test_info();
    if (nullptr == tinfo)
        return "";
    return tinfo->name();
#else
    return GetNameOfCurrentTestInternal();
#endif
    }

//! Get name of the "test case" of the currently running test. 
//! @return the name of a test case or "" if no test is running
static Utf8CP GetNameOfCurrentTestCase()
    {
#if defined (USE_GTEST)
    ::testing::TestInfo const* tinfo = ::testing::UnitTest::GetInstance()->current_test_info();
    if (nullptr == tinfo)
        return "";
    return tinfo->test_case_name();
#else
    return GetNameOfCurrentTestCaseInternal();
#endif
    }


BENTLEYDLL_EXPORT static void   SetNameOfCurrentTestInternal(Utf8CP tc, Utf8CP tn);
BENTLEYDLL_EXPORT static Utf8CP GetNameOfCurrentTestInternal();
BENTLEYDLL_EXPORT static Utf8CP GetNameOfCurrentTestCaseInternal();


//! Get name of currently running test case

///@}

///@name Ignore list parsing utilities
///@{

//! Each line of the input file is expected to be either a \#comment or a test name, optionally followed by a \#comment
//! The format of the returned string is: test_name1:test_name2:...
BENTLEYDLL_EXPORT static void ReadTestList (Utf8String& contents, FILE* fp);

//! Get the entries all files in the specified directory tree
BENTLEYDLL_EXPORT static void LoadTestList (Utf8String& contents, BeFileName const& ignoreDir);

//! Get the entries all files in the "Ignore" directory tree as tests to be ignored and files in the "Filter" directory as tests to be run
BENTLEYDLL_EXPORT static void LoadFilters (Utf8String& toignore, Utf8String& torun);

///@}

//! Break in the debugger. On Android, this does not actually break in the debugger. Instead, it prints a logging message as a prompt and then spins in a tight loop, waiting for you to break into the process by running ndk-gdb.
BENTLEYDLL_EXPORT static void BreakInDebugger (CharCP msg1="BeTest", CharCP msg2="Break!");

enum LogPriority
    {
    PRIORITY_FATAL      = 0,      /// Used for fatal errors that will terminate the application
    PRIORITY_ERROR      = (-1),   /// Used for general errors
    PRIORITY_WARNING    = (-2), /// Used for general warnings
    PRIORITY_INFO       = (-3),    /// Used for general informatin
    PRIORITY_DEBUG      = (-4),   /// Used for debugging information
    PRIORITY_TRACE      = (-5)    /// Used for function calls
    };

//! Display a logging message.
//! @param category The category or namespace of the message
//! @param message  Message to display
//! @param priority The priority of the message or severity of the error
BENTLEYDLL_EXPORT static void Log (Utf8CP category, LogPriority priority, Utf8CP message);

#if !defined (USE_GTEST)

///@name Test case setup/teardown
///@{
BENTLEYDLL_EXPORT static TestCaseInfo* RegisterTestCase(Utf8CP, T_SetUpFunc, T_TearDownFunc);
BENTLEYDLL_EXPORT static void SetUpTestCase(Utf8CP);
BENTLEYDLL_EXPORT static void TearDownTestCase(Utf8CP);
///@}


    // NB: the macros expand to 
    //  if(test) ; else ExpectedResult(...) 
    // to mimic the gtest implementation. Note that the error handling is the in else clause and has no trailing ;
    // The main reason for doing it this way is that whatever comes after the test macro must be evaluated only if the expression is false. For example,
    // In the test
    //  ASSERT_TRUE(expression) << generateErrorMessage()
    // The call to generateErrorMessage() should only be made if expression is false.
    // We have the error handling in the else clause because tests do things like this:
    // if (entry.GetId() == mat3->GetMaterialId())
    //      EXPECT_STREQ(mat3->GetMaterialName().c_str(), entry.GetName());
    // else
    //      FAIL() << "This material should not exisit";
    // We don't want the test's own else to become attached to the EXPECT_STREQ's internal if-test.
    // Also, note that, when evaluating a test macro, don't assume that in checking for failure we can invert the sense of the original test.
    // For example, TEST_EQ(expr1, expr2)
    // must NOT be simplified to : if ((expr1) != (expr2))
    // That is because expr1 and expr2 might be objects, and we don't know that their class defines the != operator as well as the == operator.
    // Note that the ExpectedResult's destructor is where the error is reported. That gives it time to accumulate the << messages that follow the macro.
    #define BE_TEST_EXPECTED_RESULT_EQ(expected,actual,fatal)       if ((expected) == (actual))          {} else BeTest::ExpectedResult (false, #expected, #actual,      __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_NE(val1,val2,fatal)             if ((val1) != (val2))                {} else BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_STREQ(val1,val2,fatal)          if (BeTest::EqStr(val1,val2,false))  {} else BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_STRCASEEQ(val1,val2,fatal)      if (BeTest::EqStr(val1,val2,true))   {} else BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_STRNE(val1,val2,fatal)          if (!BeTest::EqStr(val1,val2,false)) {} else BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_TRUE(expression,fatal)          if (expression)                      {} else BeTest::ExpectedResult (false, "TRUE",    #expression,  __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_FALSE(expression,fatal)         if (!(expression))                   {} else BeTest::ExpectedResult (false, "FALSE",   #expression,  __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_LE(val1,val2,fatal)             if ((val1) <= (val2))                {} else BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_LT(val1,val2,fatal)             if ((val1) <  (val2))                {} else BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_GE(val1,val2,fatal)             if ((val1) >= (val2))                {} else BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_GT(val1,val2,fatal)             if ((val1) >  (val2))                {} else BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_NEAR(val1, val2, tol,fatal)     if (BeTest::EqTol(val1,val2,tol))    {} else BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_NEAR_(val1, val2, fatal)        if (BeTest::EqNear(val1,val2))       {} else BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_FAIL()                                                                  BeTest::ExpectedResult (false, "SUCCESS", "FAIL",       __FILE__ , __LINE__,true)

    // These macro names match those defined in <gtest.h>
    #define ASSERT_EQ(expected,actual)   BE_TEST_EXPECTED_RESULT_EQ(expected,actual,true)
    #define EXPECT_EQ(expected,actual)   BE_TEST_EXPECTED_RESULT_EQ(expected,actual,false)
    #define ASSERT_TRUE(expression)      BE_TEST_EXPECTED_RESULT_TRUE(expression,true)
    #define EXPECT_TRUE(expression)      BE_TEST_EXPECTED_RESULT_TRUE(expression,false)
    #define ASSERT_FALSE(expression)     BE_TEST_EXPECTED_RESULT_FALSE(expression,true)
    #define EXPECT_FALSE(expression)     BE_TEST_EXPECTED_RESULT_FALSE(expression,false)
    #define ASSERT_NE(val1,val2)         BE_TEST_EXPECTED_RESULT_NE(val1,val2,true)
    #define EXPECT_NE(val1,val2)         BE_TEST_EXPECTED_RESULT_NE(val1,val2,false)
    #define ASSERT_LE(val1,val2)         BE_TEST_EXPECTED_RESULT_LE(val1,val2,true)
    #define EXPECT_LE(val1,val2)         BE_TEST_EXPECTED_RESULT_LE(val1,val2,false)
    #define ASSERT_LT(val1,val2)         BE_TEST_EXPECTED_RESULT_LT(val1,val2,true)
    #define EXPECT_LT(val1,val2)         BE_TEST_EXPECTED_RESULT_LT(val1,val2,false)
    #define ASSERT_GE(val1,val2)         BE_TEST_EXPECTED_RESULT_GE(val1,val2,true)
    #define EXPECT_GE(val1,val2)         BE_TEST_EXPECTED_RESULT_GE(val1,val2,false)
    #define ASSERT_GT(val1,val2)         BE_TEST_EXPECTED_RESULT_GT(val1,val2,true)
    #define EXPECT_GT(val1,val2)         BE_TEST_EXPECTED_RESULT_GT(val1,val2,false)
    #define ASSERT_STREQ(val1,val2)      BE_TEST_EXPECTED_RESULT_STREQ(val1,val2,true)
    #define EXPECT_STREQ(val1,val2)      BE_TEST_EXPECTED_RESULT_STREQ(val1,val2,false)
    #define ASSERT_STRCASEEQ(val1,val2)  BE_TEST_EXPECTED_RESULT_STRCASEEQ(val1,val2,true)
    #define EXPECT_STRCASEEQ(val1,val2)  BE_TEST_EXPECTED_RESULT_STRCASEEQ(val1,val2,false)
    #define ASSERT_STRNE(val1,val2)      BE_TEST_EXPECTED_RESULT_STRNE(val1,val2,true)
    #define EXPECT_STRNE(val1,val2)      BE_TEST_EXPECTED_RESULT_STRNE(val1,val2,false)
    #define ASSERT_NEAR(val1, val2, tol) BE_TEST_EXPECTED_RESULT_NEAR(val1, val2, tol,true)
    #define EXPECT_NEAR(val1, val2, tol) BE_TEST_EXPECTED_RESULT_NEAR(val1, val2, tol,false)
    #define EXPECT_DOUBLE_EQ(v1,v2)      BE_TEST_EXPECTED_RESULT_NEAR_(v1,v2,false)
    #define ASSERT_DOUBLE_EQ(v1,v2)      BE_TEST_EXPECTED_RESULT_NEAR_(v1,v2,true)
    #define FAIL()                       BE_TEST_EXPECTED_RESULT_FAIL()
    #define SUCCEED()

    ///@name Test utilities
    ///@{

    //! Captures the result of testing a condition.
    struct ExpectedResult
        {
        bool m_abortImmediately;
        Utf8String m_message;
        BENTLEYDLL_EXPORT ExpectedResult (bool isAsExpected,                                                            CharCP actualExpression, CharCP expectedExpression, CharCP fileName, size_t  lineNum, bool abortImmediately);
        BENTLEYDLL_EXPORT ExpectedResult (bool isAsExpected, CharCP actualValue, CharCP expectedValue, bool expectedEq, CharCP actualExpression, CharCP expectedExpression, CharCP fileName, size_t  lineNum, bool abortImmediately);
        BENTLEYDLL_EXPORT ~ExpectedResult() THROW_SPECIFIER(CharCP);
        BENTLEYDLL_EXPORT ExpectedResult& operator<< (WCharCP msg);
#if defined(__clang__) && defined(__APPLE__)
        BENTLEYDLL_EXPORT ExpectedResult& operator<< (size_t val);
#endif
        BENTLEYDLL_EXPORT ExpectedResult& operator<< (int32_t val);
        BENTLEYDLL_EXPORT ExpectedResult& operator<< (uint32_t val);
        BENTLEYDLL_EXPORT ExpectedResult& operator<< (int64_t val);
        BENTLEYDLL_EXPORT ExpectedResult& operator<< (uint64_t val);
        BENTLEYDLL_EXPORT ExpectedResult& operator<< (double val);
        BENTLEYDLL_EXPORT ExpectedResult& operator<< (CharCP msg);
                          ExpectedResult& operator<< (std::string const& str) {return this->operator<<(str.c_str());}
        BENTLEYDLL_EXPORT ExpectedResult& operator<< (Utf8String const& msg);
        BENTLEYDLL_EXPORT ExpectedResult& operator<< (WString const& msg);
        };

    //! Compare two floating point numbers for near equality
    BENTLEYDLL_EXPORT static bool EqNear (double v1, double v2);
    //! Compare two floating point numbers for equality <= tolerance
    BENTLEYDLL_EXPORT static bool EqTol (double v1, double v2, double tol);
    //! Compare two integers for equality <= tolerance
    BENTLEYDLL_EXPORT static bool EqTol (int    v1, int    v2, int    tol);
    //! Compare two Utf-8 strings
    BENTLEYDLL_EXPORT static bool EqStr (Utf8CP s1, Utf8CP s2, bool ignoreCase);
    //! Compare two wchar_t strings
    BENTLEYDLL_EXPORT static bool EqStr (WCharCP s1, WCharCP s2, bool ignoreCase);
    //! Call this before running tests
    BENTLEYDLL_EXPORT static void ClearErrorCount();
    //! Call this when a test fails
    BENTLEYDLL_EXPORT static void IncrementErrorCount();
    //! Call this to detect if tests have failed
    BENTLEYDLL_EXPORT static size_t GetErrorCount();
    //! Call this get the number of errors in total
    BENTLEYDLL_EXPORT static void RecordFailedTest (testing::Test const&);
    //! Report failed tests
    BENTLEYDLL_EXPORT static bset<Utf8String> const& GetFailedTests();
    //! Set BeTest to throw an (unhandled) exception when a test fails
    BENTLEYDLL_EXPORT static void SetBreakOnFailure (bool);
    //! Check if BeTest should throw an (unhandled) exception when a test fails
    BENTLEYDLL_EXPORT static bool GetBreakOnFailure ();
    //! Specify which tests to run and which to ignore.
    BENTLEYDLL_EXPORT static void SetRunFilters (bvector<Utf8String> const& toignore, bvector<Utf8String> const& torun);
    //! Should this test be run?
    BENTLEYDLL_EXPORT static bool ShouldRunTest (CharCP fullTestName);

    private:
        static bvector<Utf8String>         s_runList;
        static bvector<Utf8String>         s_ignoreList;
        static bset<Utf8String>            s_failedTests;
        static size_t                      s_errorCount;
        static bool                        s_breakOnFailure;

#else

#endif //!defined (USE_GTEST)

};

//Performance Result Recording to a CSV file

// Add following line to your Performance test
// LOGTODB(TEST_DETAILS, m_InsertTime, "Sql Insert time", 1000);
// It automatically adds Test case name and Test name.You will need to provide time in seconds and then some additional details and number of operations.Last two arguments are optional.
// All entries go into PerformanceResult.csv which is located in your Performance test runners' run\Output\\PerformanceTestResults folder.

#define PERFORMANCELOG (*NativeLogging::LoggingManager::GetLogger (L"Performance"))
#define LOGTODB PerformanceResultRecorder::writeResults
#define TEST_FIXTURE_NAME BeTest::GetNameOfCurrentTestCase()
#define TEST_NAME BeTest::GetNameOfCurrentTest()
#define TEST_DETAILS TEST_FIXTURE_NAME, TEST_NAME


//=======================================================================================
// @bsiclass                                                Majd.Uddin     05/2015
//=======================================================================================
struct PerformanceResultRecorder
{

public:
    PerformanceResultRecorder();
    BENTLEYDLL_EXPORT static void writeResults(Utf8String testcaseName, Utf8String testName, double timeInSeconds, Utf8String testDescription = "", int opCount = -1);

};

#define EXPECT_CONTAINS(container, value)                                       \
    EXPECT_FALSE (std::find (container.begin (), container.end (), value) == container.end ())

#define EXPECT_NCONTAIN(container, value)                                       \
    EXPECT_TRUE (std::find (container.begin (), container.end (), value) == container.end ())

#define EXPECT_BETWEEN(smallerValue, value, biggerValue)                        \
    EXPECT_LE (smallerValue, value);                                            \
    EXPECT_GE (biggerValue, value)

END_BENTLEY_NAMESPACE

/// @endcond BENTLEY_SDK_Internal
