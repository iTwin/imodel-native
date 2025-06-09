/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
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
#include <Bentley/Logging.h>
#include <string>

#ifdef BETEST_GENERATE_UNITTESTS_LIST_H
    #undef USE_GTEST
#endif

#define BE_TEST_BREAK_IN_DEBUGGER BeTest::BreakInDebugger();

#if defined (USE_GTEST)

    #if !defined (BETEST_NO_INCLUDE_GTEST)
    #ifndef UNICODE
        #define UNICODE 1
        #define _UNICODE 1
    #endif
PUSH_DISABLE_DEPRECATION_WARNINGS
    #include <gtest/gtest.h>
    #include <gmock/gmock.h>
POP_DISABLE_DEPRECATION_WARNINGS
    #endif

#else

    #include <math.h>

    namespace testing
        {
        class Test
            {
          protected:
            virtual void SetUp() {;}
            virtual void TearDown() {;}
            virtual void TestBody() {;}
            virtual void InvokeTestBody() { TestBody(); }
          public:
            BENTLEYDLL_EXPORT void    RunTest();
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
            BentleyApi::CharCP  GetTestCaseNameA () const override { return #testCaseName; }\
            BentleyApi::CharCP  GetTestNameA ()     const override { return #testName; }    \
            virtual void TestBody () override;                                              \
            static BeTest::TestCaseInfo* s_superClassTestCaseInfo;                          \
        };                                                                                  \
    BeTest::TestCaseInfo* BETEST_TEST_CLASS_NAME(testCaseName,testName)::s_superClassTestCaseInfo = BeTest::RegisterTestCase(#testCaseName, & superTestName :: SetUpTestCase, & superTestName :: TearDownTestCase);\
    extern "C" EXPORT_ATTRIBUTE int BETEST_TEST_RUNNER_FUNC(testCaseName,testName) ()       \
        {                                                                                   \
        size_t e = BeTest::GetErrorCount();                                                 \
        BeTest::SetNameOfCurrentTestInternal(#testCaseName,#testName);                      \
        BETEST_TEST_CLASS_NAME(testCaseName,testName) t;                                    \
        t.Run();                                                                            \
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
#if defined (__unix__)
    static bool s_loop;
#endif

#ifdef _WIN32
    static void OnInvalidParameter(const wchar_t * expression, const wchar_t * function, const wchar_t * file, unsigned int line, uintptr_t pReserved);
#endif

static void AssertionFailureHandler (WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType atype);

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
    virtual void _OnAssertionFailure (WCharCP msg) = 0;
    virtual void _OnUnexpectedResult (WCharCP msg) = 0;
    virtual void _OnFailureHandled() = 0;
    };

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

//! Query if the specified kinds of assertion failures are turned on or not.
//! @return true if tests will break when assertion failures are detected or false if not.
BENTLEYDLL_EXPORT static bool GetFailOnAssert (BeAssertFunctions::AssertType atype = BeAssertFunctions::AssertType::All);

//! Disable invalid printf parameter asserts in some tests that produce them to test failures.
//! Should not be used in other cases.
BENTLEYDLL_EXPORT static void SetFailOnInvalidParameterAssert(bool doFail);

//! Returns a flag indicating the there was an assertion failure in the current test.
BENTLEYDLL_EXPORT static bool GetAssertionFailed();

//! Signature of an assertion failure listener. There can be many listeners. A listener does not control how assertion failure is handled by BeTest. Normally, a listener will log a message.
typedef void T_BeAssertListener (WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType atype);

//! Register an assertion failure listener
BENTLEYDLL_EXPORT static void SetBeAssertListener (T_BeAssertListener*);

BENTLEYDLL_EXPORT static void setS_mainThreadId (intptr_t id);


///@}

///@name Information about the currently running test
///@{

//! Get name of currently running test.
//! @return the name of a test or "" if no test is running
static Utf8CP GetNameOfCurrentTest()
    {
#if defined (USE_GTEST)
#if !defined(BETEST_NO_INCLUDE_GTEST)
    ::testing::TestInfo const* tinfo = ::testing::UnitTest::GetInstance()->current_test_info();
    if (nullptr == tinfo)
        return "";
    return tinfo->name();
#else
    BeAssert(false);
    return nullptr;
#endif
#else
    return GetNameOfCurrentTestInternal();
#endif
    }

//! Get name of the "test case" of the currently running test.
//! @return the name of a test case or "" if no test is running
static Utf8CP GetNameOfCurrentTestCase()
    {
#if defined (USE_GTEST)
#if !defined(BETEST_NO_INCLUDE_GTEST)
    ::testing::TestInfo const* tinfo = ::testing::UnitTest::GetInstance()->current_test_info();
    if (nullptr == tinfo)
        return "";
    return tinfo->test_case_name();
#else
    BeAssert(false);
    return nullptr;
#endif
#else
    return GetNameOfCurrentTestCaseInternal();
#endif
    }

BENTLEYDLL_EXPORT static void   SetNameOfCurrentTestInternal(Utf8CP tc, Utf8CP tn);
BENTLEYDLL_EXPORT static Utf8CP GetNameOfCurrentTestInternal();
BENTLEYDLL_EXPORT static Utf8CP GetNameOfCurrentTestCaseInternal();


//! Get name of currently running test case

///@}

//! Break in the debugger. On Android, this does not actually break in the debugger. Instead, it prints a logging message as a prompt and then spins in a tight loop, waiting for you to break into the process by running ndk-gdb.
BENTLEYDLL_EXPORT static void BreakInDebugger (CharCP msg1="BeTest", CharCP msg2="Break!");

enum LogPriority
    {
    PRIORITY_FATAL      = 0,      /// Used for fatal errors that will terminate the application
    PRIORITY_ERROR      = -1,   /// Used for general errors
    PRIORITY_WARNING    = -2, /// Used for general warnings
    PRIORITY_INFO       = -3,    /// Used for general information
    PRIORITY_TRACE      = -4    /// Used for function calls
    };

//! Display a logging message.
//! @param category The category or namespace of the message
//! @param message  Message to display
//! @param priority The priority of the message or severity of the error
BENTLEYDLL_EXPORT static void Log (Utf8CP category, LogPriority priority, Utf8CP message);

#if !defined (USE_GTEST)

private:
BENTLEYDLL_EXPORT static void SetIFailureHandler (IFailureHandler&);

public:
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
    #define BE_TEST_EXPECTED_RESULT_EQ(expected,actual,fatal)       if (!((expected) == (actual)))          BeTest::ExpectedResult (false, #expected, #actual,      __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_NE(val1,val2,fatal)             if (!((val1) != (val2)))                BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_STREQ(val1,val2,fatal)          if (!BeTest::EqStr(val1,val2,false))    BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_STRCASEEQ(val1,val2,fatal)      if (!BeTest::EqStr(val1,val2,true))     BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_STRNE(val1,val2,fatal)          if (BeTest::EqStr(val1,val2,false))     BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_TRUE(expression,fatal)          if (!(expression))                      BeTest::ExpectedResult (false, "TRUE",    #expression,  __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_FALSE(expression,fatal)         if (expression)                         BeTest::ExpectedResult (false, "FALSE",   #expression,  __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_LE(val1,val2,fatal)             if (!((val1) <= (val2)))                BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_LT(val1,val2,fatal)             if (!((val1) <  (val2)))                BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_GE(val1,val2,fatal)             if (!((val1) >= (val2)))                BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_GT(val1,val2,fatal)             if (!((val1) >  (val2)))                BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_NEAR(val1, val2, tol,fatal)     if (!BeTest::EqTol(val1,val2,tol))      BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_NEAR_(val1, val2, fatal)        if (!BeTest::EqNear(val1,val2))         BeTest::ExpectedResult (false, #val1,     #val2,        __FILE__ , __LINE__,fatal)
    #define BE_TEST_EXPECTED_RESULT_FAIL()                                                                  BeTest::ExpectedResult (false, "SUCCESS", "FAIL",       __FILE__ , __LINE__,true)
    #define BE_TEST_EXPECTED_RESULT_ADDFAILURE(file, line)                                                  BeTest::ExpectedResult (false, "SUCCESS", "FAIL",       file , line, false)

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
    #define ADD_FAILURE()                BE_TEST_EXPECTED_RESULT_ADDFAILURE(__FILE__ , __LINE__)
    #define ADD_FAILURE_AT(file,line)    BE_TEST_EXPECTED_RESULT_ADDFAILURE(file, line)
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
        BENTLEYDLL_EXPORT ~ExpectedResult();
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

    private:
        static bset<Utf8String>            s_failedTests;
        static size_t                      s_errorCount;
        static bool                        s_breakOnFailure;

public:
    //! Non-gtest only: Used by custom test runner to record an error.
    //! @note This function is needed only by a test that overrides InvokeTestBody. This function must be called if
    //! the test fails.
    BENTLEYDLL_EXPORT static void IncrementErrorCountAndEnableThrows();

    //! Non-gtest only: Used by custom test runner to propagate an assertion failure
    //! @note This function is needed only by a test that overrides InvokeTestBody. This function must be called if
    //! the test succeeds.
    BENTLEYDLL_EXPORT static void RethrowAssertFromOtherTreads();

#ifdef BENTLEY_WINRT
    //! Non-gtest only / WinRT only: Tell failure-handling code where to resume execution when aborting a failing test.
    //! @note This function is needed only by a test that overrides InvokeTestBody. In that case, the test must do its
    //! own error-checking and reporting. On UWP only, when a test fails and must be terminated, BeTest calls longjmp.
    //! The purpose of this function is to allow the caller to tell BeTest what jmp_buf it should use.
    //! @param jmpbufptr    Must be a pointer to a real jmp_buf that has been set up by the caller with setjmp
    BENTLEYDLL_EXPORT static void SetFailureJmpbuf(void* jmpbufptr);
#endif

#else

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

typedef std::function<void(wchar_t const*)> T_AssertionFailureHandler;

BENTLEYDLL_EXPORT static void SetAssertionFailureHandler(T_AssertionFailureHandler const& f);

#endif //!defined (USE_GTEST)

//! Indicate that the test has failed. This helper function may be called by libraries that do not link with gtest.
BENTLEYDLL_EXPORT static void Fail(WCharCP msg = L"");

};

//Performance Result Recording to a CSV file

//! Add following line to your Performance test
//! LOGTODB(TEST_DETAILS, insertTime, 100, "SQL Insert time", false /*quoteTestDescription*/);
//! It automatically adds Test case name and Test name. You will need to provide time in seconds, number of operations, some additional details and whether the additional details string should be wrapped in quotes or not.Last three arguments are optional.
//! All entries go into PerformanceResult.csv which is located in your Performance test runners' run\Output\\PerformanceTestResults folder.
#define LOGTODB PerformanceResultRecorder::WriteResults
#define LOGPERFDB PerformanceResultRecorder::WriteResultsPerf
#define PERFORMANCELOG (NativeLogging::CategoryLogger("Performance"))
#define TEST_FIXTURE_NAME BeTest::GetNameOfCurrentTestCase()
#define TEST_NAME BeTest::GetNameOfCurrentTest()
#define TEST_DETAILS TEST_FIXTURE_NAME, TEST_NAME


//=======================================================================================
// @bsiclass
//=======================================================================================
struct PerformanceResultRecorder
{
private:
    PerformanceResultRecorder();
    ~PerformanceResultRecorder();

public:
    BENTLEYDLL_EXPORT static void WriteResults(Utf8CP testcaseName, Utf8CP testName, double timeInSeconds, int opCount = -1, Utf8CP testDescription = "", bool quoteTestDecription = false , Utf8String opType ="" , int initialCount=-1);
    BENTLEYDLL_EXPORT static void WriteResultsPerf(Utf8CP testcaseName, Utf8CP testName, double timeInSeconds, Utf8CP info);
    BENTLEYDLL_EXPORT static void WriteResultsPerf(Utf8CP testcaseName, Utf8CP testName, Utf8CP valueDescription, double value, Utf8CP info);
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