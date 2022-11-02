/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
    #include <windows.h>

#elif defined (BENTLEY_WINRT)
    #include <windows.h>
    //#include <wrl.h>
    #include <setjmp.h>
    static jmp_buf s_test_failure_jmp_buf;
    static jmp_buf* s_test_failure_jmp_buf_to_use;

#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        #include <pthread.h>
    #endif
    #if defined (ANDROID)
        #include <android/log.h>
    #endif
#else
    #error unknown compiler
#endif

#include <regex>
#include <exception>
#include <signal.h>
#include <map>
#define BETEST_NO_INCLUDE_GTEST // there's no point in including gtest.h, since Bentley.dll does not link with gtest
#include <Bentley/BeTest.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/DateTime.h>
#include <Bentley/Logging.h>

#if !defined (USE_GTEST)
    #if defined(BENTLEYCONFIG_OS_LINUX) || defined(BENTLEYCONFIG_OS_APPLE_MACOS) || (defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT))
        #error USE_GTEST is not defined, but this platform uses gtest!
    #endif
#else
    #if defined(BENTLEYCONFIG_OS_ANDROID) || defined(BENTLEYCONFIG_OS_APPLE_IOS) || defined(BENTLEYCONFIG_OS_WINRT)
        #error USE_GTEST is defined, but this platform does not use gtest!
    #endif
#endif

USING_NAMESPACE_BENTLEY

static BeMutex s_bentleyCS;
static intptr_t                                 s_mainThreadId;                     // MT: set only during initialization
static BeAssertFunctions::T_BeAssertHandler*    s_assertHandler;                    // MT: s_bentleyCS
static bool                                     s_assertHandlerCanBeChanged=true;   // MT: s_bentleyCS
static bvector<BeTest::T_BeAssertListener*>     s_assertListeners;                  // MT: s_bentleyCS
static bool                                     s_failOnAssert[(int)BeAssertFunctions::AssertType::TypeCount]; // MT: Problem! If one thread sets this, it will affect assertions that fail on other threads. *** WIP_MT make this thread-local?
static bool                                     s_failOnInvalidParameterAssert = true;
static bool                                     s_runningUnderGtest;                // indicates that we are running under gtest. MT: set only during initialization
static bool                                     s_hadAssert;
static bool                                     s_runtimeOverride = false;
static RefCountedPtr<BeTest::Host>              s_host;                             // MT: set only during initialization. Used on multiple threads. Must be thread-safe internally.
#if defined (__unix__)
bool BeTest::s_loop = true;
#endif
#if defined (USE_GTEST)
static BeTest::T_AssertionFailureHandler        s_assertionFailureHandler;
#else
static bool                                     s_hadAssertOnAnotherThread;         // MT: s_bentleyCS
static Utf8String                               s_currentTestCaseName;              // MT: set only when we run a test, which is always in the main thread
static Utf8String                               s_currentTestName;                  //  "       "
#endif

#ifdef XX_BETEST_HAS_SEH

#define BE_TEST_SEH_TRY                 __try{
#define BE_TEST_SEH_EXCEPT(CLEANUP)          } __except(ExpFilter(GetExceptionInformation(), GetExceptionCode()))  {    CLEANUP   }

// Exception filter (used to show exception call-stack)
static LONG WINAPI ExpFilter(EXCEPTION_POINTERS* pExp, DWORD dwExpCode)
    {
    if (dwExpCode != EXCEPTION_BREAKPOINT)
        {
        #if defined (WIP_BETEST)
        StackWalker sw;
        sw.ShowCallstack(GetCurrentThread(), pExp->ContextRecord);
        #else
        LOG_ERROR ("--- EXCEPTION ---\n");
        #endif
        return EXCEPTION_EXECUTE_HANDLER;
        }
    else
        return EXCEPTION_CONTINUE_SEARCH;
    }

#else

#define BE_TEST_SEH_TRY
#define BE_TEST_SEH_EXCEPT(CLEANUP)

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::SetRunningUnderGtest () {s_runningUnderGtest=true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::BreakInDebugger (CharCP msg1, CharCP msg2)
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    WString debugMessage (msg1, BentleyCharEncoding::Utf8);
    debugMessage.append (L":");
    debugMessage.AppendUtf8 (msg2);
    OutputDebugStringW (debugMessage.c_str());

    #if defined (BENTLEY_WIN32)
        DebugBreak();
    #else
        assert (false);
    #endif

#elif defined (__unix__)

    #if !defined (ANDROID)

        fprintf (stderr, "%s:%s. Waiting", msg1,msg2);
        int i=0;
        while (s_loop)
            {
//            BeThreadUtilities::BeSleep (1);
            if (i++ % 10000 == 0)
                printf (".");
            }

    #else

        __android_log_print (ANDROID_LOG_ERROR, msg1, "%s. Waiting", msg2);

        int i=0;
        while (s_loop)
            {
            //BeThreadUtilities::BeSleep (1);
            if (i++ % 10000 == 0)
                printf (".");
            }

    #endif

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static WString getAssertTypeDesc (BeAssertFunctions::AssertType t)
    {
    switch (t)
        {
        case BeAssertFunctions::AssertType::Data:      return L"DATA ASSERTION FAILURE";
        case BeAssertFunctions::AssertType::Sigabrt:   return L"SIGABRT";
        }
    return L"ASSERTION FAILURE";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::BeAssertFunctions::DefaultAssertionFailureHandler (WCharCP message, WCharCP file, unsigned line)
    {
#if !defined (BENTLEY_WINRT) // *** WIP_WINRT_GETENV
PUSH_DISABLE_DEPRECATION_WARNINGS
    CharCP env = getenv ("MS_IGNORE_ASSERTS");
POP_DISABLE_DEPRECATION_WARNINGS
    if (NULL != env && (*env == '1' || tolower(*env) == 't'))
        return;
#endif

    NativeLogging::CategoryLogger("BeAssert").fatalv(L"ASSERTION FAILURE: %ls (%ls:%d)\n", message, file, line);
    if (s_runtimeOverride)
        return;

    #ifndef NDEBUG
        #if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)
            // We used to explicitly ::DebugBreak here, but this causes crashes if you're not actually in a debugger, which is not necessarily desirable for an assert which you may want to try and ignore.
            _wassert (message, file, line);
        #elif defined (ANDROID)
            //  The desired Android behavior is to log the error, then continue
        #else
            assert (false);
        #endif
    #endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::BeAssertFunctions::PerformBeAssert (WCharCP message, WCharCP file, unsigned line)
    {
    s_bentleyCS.lock();
    T_BeAssertHandler* host = s_assertHandler;
    s_hadAssert = true;
    s_bentleyCS.unlock();

    if (NULL != host)
        {
        // Let host handle assertion failure
        host (message, file, line, BeAssertFunctions::AssertType::Normal);
        return;
        }

    DefaultAssertionFailureHandler (message, file, line);
    }

/*---------------------------------------------------------------------------------**//**
* Will assert on developer's box, but not in Filecheck where bad data must be handled.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::BeAssertFunctions::PerformBeDataAssert (WCharCP message, WCharCP file, unsigned line)
    {
#if !defined (BENTLEY_WINRT) // *** WIP_WINRT_GETENV
PUSH_DISABLE_DEPRECATION_WARNINGS
    CharCP env = getenv ("MS_IGNORE_DATA_ASSERTS");
POP_DISABLE_DEPRECATION_WARNINGS
    if (NULL != env && (*env == '1' || tolower(*env) == 't'))
        return;
#endif

    s_bentleyCS.lock();
    T_BeAssertHandler* host = s_assertHandler;
    s_hadAssert = true;
    s_bentleyCS.unlock();

    if (NULL != host)
        {
        host (message, file, line, BeAssertFunctions::AssertType::Data);
        return;
        }

    PerformBeAssert (message, file, line);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::BeAssertFunctions::SetBeAssertHandler (T_BeAssertHandler* h)
    {
    s_bentleyCS.lock();
    if (s_assertHandlerCanBeChanged)
        s_assertHandler = h;
    s_bentleyCS.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::BeAssertFunctions::SetBeTestAssertHandler (T_BeAssertHandler* h)
    {
    s_bentleyCS.lock();
    s_assertHandlerCanBeChanged = false;
    s_assertHandler = h;
    s_bentleyCS.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::GetAssertionFailed()
    {
    BeMutexHolder lock(s_bentleyCS);
    return s_hadAssert;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeTest::SetFailOnAssert (bool doFail, BeAssertFunctions::AssertType atype)
    {
    if (BeThreadUtilities::GetCurrentThreadId () != s_mainThreadId)
        {
        // Not thread-safe!!
        return;
        }

    s_bentleyCS.lock();

    if (atype == BeAssertFunctions::AssertType::All)
        {
        for (int i=0; i<(int)BeAssertFunctions::AssertType::TypeCount; ++i)
            {
            if (i != (int)BeAssertFunctions::AssertType::Data)   // must set data asserts specifically
                s_failOnAssert[i] = doFail;
            }
        }
    else
        {
        s_failOnAssert[(int)atype] = doFail;
        }

    s_bentleyCS.unlock();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void BeTest::setS_mainThreadId (intptr_t id)
    {
    s_mainThreadId = id;
    return void ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::SetFailOnInvalidParameterAssert(bool doFail)
    {
    BeMutexHolder lock(s_bentleyCS);
    s_failOnInvalidParameterAssert = doFail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::GetFailOnAssert (BeAssertFunctions::AssertType atype)
    {
    bool allWillFail = true;

    s_bentleyCS.lock();

    if (atype == BeAssertFunctions::AssertType::All)
        {
        allWillFail = true;
        for (int i=0; i<(int)BeAssertFunctions::AssertType::TypeCount; ++i)
            {
            if (i != (int)BeAssertFunctions::AssertType::Data)   // must set data asserts specifically
                allWillFail &= s_failOnAssert[i];
            }
        }
    else
        {
        allWillFail = s_failOnAssert[(int)atype];
        }

    s_bentleyCS.unlock();

    return allWillFail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeTest::SetBeAssertListener (T_BeAssertListener* l)
    {
    s_bentleyCS.lock();
    s_assertListeners.push_back(l);
    s_bentleyCS.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::Host&  BeTest::GetHost () {return *s_host;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::Initialize (Host& host)
    {
    s_host = &host;

    for (size_t i = 0; i<_countof(s_failOnAssert); ++i)
        s_failOnAssert[i] = true;

    s_failOnAssert[(int)BeAssertFunctions::AssertType::Data] = false;

    s_mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    BeAssertFunctions::SetBeTestAssertHandler(AssertionFailureHandler);   // steals asserthandler and prevents any else from overriding it!

    static bool s_initialized = false;
    if (s_initialized)
        return;
    s_initialized = true;

    BeSystemMutexHolder::StartupInitializeSystemMutex();

#if defined (BENTLEY_WIN32)

    // We do this to cause asserts to always output to a message box.  Without this the gtest process will abort without giving the developer the opportunity to
    // attach with a debugger and analyze the situation.
    _set_error_mode (_OUT_TO_STDERR);

    // Suppress exception pop-ups.
    _set_abort_behavior (0,_CALL_REPORTFAULT);
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);

    _set_invalid_parameter_handler(OnInvalidParameter);

#elif defined (BENTLEY_WINRT)

    // We do this to cause asserts to always output to a message box.  Without this the process will abort without giving the developer the opportunity to
    // attach with a debugger and analyze the situation.
    _set_error_mode(_OUT_TO_STDERR);

#elif defined (__unix__)

    // no special set-up required

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::IsInitialized()
    {
    return s_host.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::Uninitialize ()
    {
    s_host = NULL;

//#if defined (BENTLEY_WINRT)
//RoUninitialize();
//#endif
    }

#if defined(USE_GTEST)

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::ReadTestList (Utf8String& filters, FILE* fp)
    {
    char buf[512];
    while (fgets (buf,sizeof(buf),fp))
        {
        char* p = buf + strspn (buf, " \r\n\t");
        if ('#' == *p || '\0' == *p)
            continue;
        char* end = p + strcspn (p, " \t\r\n#");
        if (end)
            *end = 0;
        if (!filters.empty())
            filters.append (":");
        filters.append (p);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::LoadTestList (Utf8String& filters, BeFileName const& ignoreDir)
    {
    BeFileListIterator it (ignoreDir, true);
    BeFileName fn;
    while (it.GetNextFileName (fn) == SUCCESS)
        {
        Utf8String afn (fn);
        FILE* fp;
        BeFile::Fopen (&fp, afn.c_str(), "r");
        ReadTestList (filters, fp);
        fclose (fp);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::LoadFilters (Utf8String& toignore, Utf8String& torun)
    {
    BeFileName ignoreDir;
    GetHost().GetDgnPlatformAssetsDirectory (ignoreDir);

    ignoreDir.AppendToPath (L"Ignore");
    ignoreDir.AppendToPath (L"*");
    LoadTestList (toignore, ignoreDir);

    BeFileName runDir;
    GetHost().GetDgnPlatformAssetsDirectory (runDir);
    runDir.AppendToPath (L"Run");
    runDir.AppendToPath (L"*");
    LoadTestList (torun, runDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::SetAssertionFailureHandler(T_AssertionFailureHandler const& f)
    {
    s_assertionFailureHandler = f;
    }

#else

static std::map<Utf8String, BeTest::TestCaseInfo*>* s_testCases;                    // MT: s_testCases is populated at code load time (by static constructors), so there is no danger of a race

/*---------------------------------------------------------------------------------**//**
* Default handler for test test and assertion failures. Handles failures by throwing
* an exception. This stragey works on all platforms but may not be optimal for some,
* such as gtest.
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeTestThrowFailureHandler : BeTest::IFailureHandler
    {
    private:
    static bool s_disableThrows;

    void ThrowException (WCharCP msg)
        {
        if (!s_runningUnderGtest)
            {
            // Ignore BeAssert failures that occur when we are unwinding an assertion failure or a test failure.
            // Note that throwing while unwinding an exception will terminate the process!
            if (s_disableThrows)
                return;
            s_disableThrows = true; // I rely on the test runner to call _OnFailureHandled to allow me clear this flag.
            }
#ifdef BENTLEY_WINRT
        if (nullptr != s_test_failure_jmp_buf_to_use)
            longjmp(*s_test_failure_jmp_buf_to_use, 101);
        else
            BeTest::IncrementErrorCountAndEnableThrows();
#else
        throw "failure";
#endif
        }

    public:
    virtual void _OnAssertionFailure (WCharCP msg) {ThrowException(msg);}
    virtual void _OnUnexpectedResult (WCharCP msg) {ThrowException(msg);}
    virtual void _OnFailureHandled() {s_disableThrows=false;}
    };
bool BeTestThrowFailureHandler::s_disableThrows;
static BeTestThrowFailureHandler                s_defaultFailureHandler;
static BeTest::IFailureHandler*                 s_IFailureHandler = &s_defaultFailureHandler;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::SetIFailureHandler (IFailureHandler& h) {s_IFailureHandler=&h;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::EqNear (double v1, double v2) {return fabs(v1-v2) <= BeNumerical::ComputeComparisonTolerance(v1,v2);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::EqTol (double v1, double v2, double tol) {return fabs(v1-v2) <= tol;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::EqTol (int    v1, int    v2, int    tol) {return  abs(v1-v2) <= tol;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::EqStr (Utf8CP s1, Utf8CP s2, bool ignoreCase)
    {
    if (nullptr == s1)              // Amazingly, gtest STREQ supports NULL arguments, so we must emulate that.
        return (nullptr == s2);

    if (nullptr == s2)
        return (nullptr == s1);

    if (ignoreCase)
        return 0 == BeStringUtilities::Stricmp (s1, s2);
    else
        return 0 == strcmp (s1, s2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::EqStr (WCharCP s1, WCharCP s2, bool ignoreCase)
    {
    if (ignoreCase)
        return 0 == BeStringUtilities::Wcsicmp (s1, s2);
    else
        return 0 == wcscmp (s1, s2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult::ExpectedResult (bool isAsExpected, CharCP actualExpression, CharCP expectedExpression, CharCP fileName, size_t  lineNum, bool abortImmediately)
    {
    m_abortImmediately = false;
    if (isAsExpected)
        return;

    char lnStr [32];
    snprintf (lnStr, sizeof(lnStr), ":%d", (int)lineNum);

    m_message = "";
    m_message.append (actualExpression);
    m_message.append (" **!!** ");
    m_message.append (expectedExpression);
    m_message.append (" @ "); m_message.append (fileName); m_message.append (lnStr);
    m_abortImmediately = abortImmediately;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult::ExpectedResult (bool isAsExpected, CharCP actualValue, CharCP expectedValue, bool expectedEq, CharCP actualExpression, CharCP expectedExpression, CharCP fileName, size_t  lineNum, bool abortImmediately)
    {
    m_abortImmediately = false;
    if (isAsExpected)
        return;

    char lnStr [32];
    snprintf (lnStr, sizeof(lnStr), ":%d", (int)lineNum);

    m_message = "";
    m_message.append ("("); m_message.append (actualExpression); m_message.append ("="); m_message.append(actualValue); m_message.append(")");
    m_message.append (expectedEq? " was expected to equal ": " was not expected to equal ");
    m_message.append ("("); m_message.append (expectedExpression); m_message.append ("="); m_message.append(expectedValue); m_message.append(")");
    m_message.append (" @ "); m_message.append (fileName); m_message.append (lnStr);
    m_abortImmediately = abortImmediately;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult::~ExpectedResult()
    {
    if (m_message.empty())
        return;
    BeTest::IncrementErrorCount ();
    NativeLogging::CategoryLogger("TestRunner").fatalv ("%s\n", m_message.c_str());
    if (m_abortImmediately)
        {
        if (BeTest::GetBreakOnFailure())
            {
            BreakInDebugger ("BeTest", "FAIL");
            }
        s_IFailureHandler->_OnUnexpectedResult (WString(m_message.c_str(),true).c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (WCharCP msg)
    {
    if (m_message.empty())
        return *this;

    Utf8String details;
    BeStringUtilities::WCharToUtf8 (details, msg);
    m_message.append (details.c_str());

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (int64_t val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    snprintf (details, sizeof(details), "%" PRId64, val);
    m_message.append (details);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (int32_t val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    snprintf (details, sizeof(details), "%d", (int)val);  // the case is there to work around int/long ambiguity on Mac
    m_message.append (details);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (uint32_t val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    snprintf (details, sizeof(details), "%u", (unsigned int)val);     // the cast is there to work around int/long ambiguity on Mac
    m_message.append (details);
    return *this;
    }

#if defined(__clang__) && defined(__APPLE__)
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (size_t val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    snprintf (details, sizeof(details), "%u", (unsigned int)val);     // the cast is there to work around int/long ambiguity on Mac
    m_message.append (details);
    return *this;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (uint64_t val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    snprintf (details, sizeof(details), "%" PRIu64, val);
    m_message.append (details);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (double val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    snprintf(details, sizeof(details), "%lf", val);
    m_message.append (details);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (CharCP msg)
    {
    if (m_message.empty())
        return *this;
    m_message.append (msg);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (Utf8String const& msg)
    {
    if (m_message.empty())
        return *this;
    m_message.append (msg);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (WString const& msg)
    {
    if (m_message.empty())
        return *this;
    m_message.append (Utf8String(msg));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bset<Utf8String>                BeTest::s_failedTests;
size_t                          BeTest::s_errorCount;
bool                            BeTest::s_breakOnFailure;

void    BeTest::ClearErrorCount()                   {s_failedTests.clear(); s_errorCount=0;}
size_t  BeTest::GetErrorCount()                     {return s_errorCount;}
void    BeTest::IncrementErrorCount()               {++s_errorCount;}
void    BeTest::SetBreakOnFailure(bool b)           {s_breakOnFailure=b;}
bool    BeTest::GetBreakOnFailure()                 {return s_breakOnFailure;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::IncrementErrorCountAndEnableThrows()
    {
    IncrementErrorCount();
    s_IFailureHandler->_OnFailureHandled();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::RethrowAssertFromOtherTreads ()
    {
    BeMutexHolder holder (s_bentleyCS);
    if (s_hadAssertOnAnotherThread)
        s_IFailureHandler->_OnAssertionFailure (L"assert failed in another thread");
    }

void    BeTest::RecordFailedTest (testing::Test const& t)
    {
    Utf8String tname = t.GetTestCaseNameA();
    tname.append (".");
    tname.append (t.GetTestNameA());
    NativeLogging::CategoryLogger("TestRunner").fatalv ("%s FAILED\n", tname.c_str());
    s_failedTests.insert (tname);
    }

bset<Utf8String> const& BeTest::GetFailedTests()
    {
    return s_failedTests;
    }

void   BeTest::SetNameOfCurrentTestInternal(Utf8CP tc, Utf8CP tn)
    {
    s_currentTestCaseName = tc;
    s_currentTestName = tn;
    }

Utf8CP BeTest::GetNameOfCurrentTestCaseInternal()
    {
    return s_currentTestCaseName.c_str();
    }

Utf8CP BeTest::GetNameOfCurrentTestInternal()
    {
    return s_currentTestName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void testing::Test::RunTest()
    {
#ifdef BENTLEY_WINRT
    s_test_failure_jmp_buf_to_use = &s_test_failure_jmp_buf;
    int failure_detected = setjmp(s_test_failure_jmp_buf);
    if (0 != failure_detected)
        {
        BeTest::IncrementErrorCountAndEnableThrows();
        return;
        }

    InvokeTestBody();
    BeTest::RethrowAssertFromOtherTreads ();
#else
    try {
        InvokeTestBody();
        BeTest::RethrowAssertFromOtherTreads ();
        }
    catch(...)
        {
        BeTest::IncrementErrorCountAndEnableThrows();
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void testing::Test::Run()
    {
    NativeLogging::CategoryLogger("TestRunner").infov ("%s.%s", GetTestCaseNameA(), GetTestNameA());

    size_t e = BeTest::GetErrorCount();

    s_bentleyCS.lock();
    s_hadAssert = false;
    s_hadAssertOnAnotherThread = false;
    s_bentleyCS.unlock();

    BeAssert(s_currentTestCaseName.Equals(GetTestCaseNameA()));
    BeAssert(s_currentTestName.Equals(GetTestNameA()));

    BE_TEST_SEH_TRY
        {
        try {
            SetUp();
            BeTest::RethrowAssertFromOtherTreads ();
            }
        catch(...)
            {
            BeTest::IncrementErrorCountAndEnableThrows();
            }
        }
    BE_TEST_SEH_EXCEPT( BeTest::IncrementErrorCountAndEnableThrows(); )

    s_IFailureHandler->_OnFailureHandled();

    if (BeTest::GetErrorCount() > e)
        {
        BeTest::RecordFailedTest (*this);
        // If setup failed, don't call the test.
        return;
        }

    BE_TEST_SEH_TRY
        {
        RunTest();

        s_IFailureHandler->_OnFailureHandled();

        try {
            TearDown();             // always call teardown, even if the test itself failed.
            BeTest::RethrowAssertFromOtherTreads ();
            }
        catch(...)
            {
            BeTest::IncrementErrorCountAndEnableThrows();
            }
        }
    BE_TEST_SEH_EXCEPT( BeTest::IncrementErrorCountAndEnableThrows(); )

    s_IFailureHandler->_OnFailureHandled();

    if (BeTest::GetErrorCount() > e)
        {
        BeTest::RecordFailedTest (*this);
        }

    s_currentTestCaseName.clear();
    s_currentTestName.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::TestCaseInfo* BeTest::RegisterTestCase(Utf8CP tcname, T_SetUpFunc s, T_TearDownFunc t)
    {
    if (nullptr == s_testCases)
        s_testCases = new std::map<Utf8String, BeTest::TestCaseInfo*>();

    auto& tci = (*s_testCases)[tcname];
    if (nullptr == tci)
        {
        tci = new TestCaseInfo;
        tci->m_setUp = s;
        tci->m_tearDown = t;
        tci->m_count = 0;
        }

    return tci;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::SetUpTestCase(Utf8CP tcname)
    {
    if (0 == strcmp("Bspline", tcname))
        printf ("got here\n");
    if (nullptr == s_testCases)
        {
        BeAssert(false && "loading logic should have automatically registered all test cases");
        return;
        }

    auto itci = s_testCases->find(tcname);
    if (s_testCases->end() == itci)
        {
        BeAssert(false && "loading logic should have automatically registered all test cases");
        return;
        }

    auto tci = itci->second;
    if (0 == tci->m_count++)
        {
        tci->m_setUp();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::TearDownTestCase(Utf8CP tcname)
    {
    if (nullptr == s_testCases)
        {
        BeAssert(false && "loading logic should have automatically registered all test cases");
        return;
        }

    auto itci = s_testCases->find(tcname);
    if (s_testCases->end() == itci)
        {
        BeAssert(false && "loading logic should have automatically registered all test cases");
        return;
        }

    auto tci = itci->second;
    if (1 == tci->m_count--)
        {
        itci->second->m_tearDown();
        }
    }

#ifdef BENTLEY_WINRT
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::SetFailureJmpbuf(void* jmpbufptr)
    {
    s_test_failure_jmp_buf_to_use = (jmp_buf*)(jmpbufptr);
    }
#endif

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::AssertionFailureHandler (WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType atype)
    {
    FOR_EACH(BeTest::T_BeAssertListener* listener, s_assertListeners)
        listener (_Message, _File, _Line, atype);

    bool failOnAssert;
        {
        BeMutexHolder holder (s_bentleyCS);
        failOnAssert = s_failOnAssert[(int)atype];
        }

    if (!failOnAssert)
        {
        NativeLogging::CategoryLogger("BeAssert").infov (L"Ignoring assert %ls at %ls:%d\n", _Message, _File, _Line);
        return;
        }

    WString assertionFailure;
    assertionFailure.append (_File);

    wchar_t buf[64];
    BeStringUtilities::Snwprintf (buf, L"(%d): ", _Line);
    assertionFailure.append (buf);

    assertionFailure.append(getAssertTypeDesc(atype));

    BeStringUtilities::Snwprintf(buf, L" (%d): ", atype);
    assertionFailure.append(buf);

    assertionFailure.append(_Message);
    assertionFailure.append(L" ");

    NativeLogging::CategoryLogger("BeAssert").error (assertionFailure.c_str());

#ifdef USE_GTEST
    s_assertionFailureHandler(assertionFailure.c_str());
#else
    if (BeTest::GetBreakOnFailure())
        {
        BeTest::BreakInDebugger ("BeAssert", "ASSERT!");
        }

    if (BeThreadUtilities::GetCurrentThreadId () != s_mainThreadId)
        {
        BeMutexHolder holder (s_bentleyCS);
        s_hadAssertOnAnotherThread = true;
        return;
        }

    s_IFailureHandler->_OnAssertionFailure (assertionFailure.c_str());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (BENTLEY_WIN32)// || defined (BENTLEY_WINRT)
void BeTest::OnInvalidParameter
(
const wchar_t * expression,
const wchar_t * function,
const wchar_t * file,
unsigned int line,
uintptr_t pReserved
)
    {
    WString parameterFailure;
    if (expression != nullptr)
        {
        parameterFailure.assign (expression);
        parameterFailure.append(L": ");
        parameterFailure.append(function);
        parameterFailure.append(L" ");
        parameterFailure.append(file);
        wchar_t buf[64];
        BeStringUtilities::Snwprintf(buf, L" %d", line);
        parameterFailure.append(buf);
        }

    if (parameterFailure.empty())
        parameterFailure = L"Invalid format parameter";

    bool failOnAssert;
        {
        BeMutexHolder holder(s_bentleyCS);
        failOnAssert = s_failOnInvalidParameterAssert;
        }

    if (!failOnAssert)
        {
        NativeLogging::CategoryLogger("BeAssert").infov(L"Ignoring parameter failure: %ls\n", parameterFailure.c_str());
        return;
        }

    NativeLogging::CategoryLogger("BeAssert").error(parameterFailure.c_str());

#ifdef USE_GTEST
    s_assertionFailureHandler(parameterFailure.c_str());
#else
    if (BeTest::GetBreakOnFailure())
        {
        BeTest::BreakInDebugger("BeAssert", "onInvalidParameter");
        }

    if (!s_runningUnderGtest && BeThreadUtilities::GetCurrentThreadId() != s_mainThreadId)
        {
        BeMutexHolder holder(s_bentleyCS);
        s_hadAssertOnAnotherThread = true;
        return;
        }

    s_IFailureHandler->_OnAssertionFailure(parameterFailure.c_str());
#endif
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::Fail(WCharCP msg)
    {
#ifdef USE_GTEST
    s_assertionFailureHandler(msg);
#else
    s_IFailureHandler->_OnUnexpectedResult (msg);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void* BeTest::Host::InvokeP (char const* f, void* a)            {return _InvokeP (f, a);}
void  BeTest::Host::GetDocumentsRoot (BeFileName& path)         {_GetDocumentsRoot (path);}
void  BeTest::Host::GetDgnPlatformAssetsDirectory (BeFileName& path) {_GetDgnPlatformAssetsDirectory (path);}
void  BeTest::Host::GetOutputRoot (BeFileName& path)            {_GetOutputRoot (path);}
void  BeTest::Host::GetTempDir (BeFileName& path)               {_GetTempDir (path);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::Log (Utf8CP category, LogPriority priority, Utf8CP message)
    {
    NativeLogging::CategoryLogger(category).messagev((NativeLogging::SEVERITY)priority, "%s", message);
    }

/*---------------------------------------------------------------------------------**//**
* Writes time to a csv file
*@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceResultRecorder::WriteResults(Utf8CP testcaseName, Utf8CP testName, double timeInSeconds, int opCount, Utf8CP testDescription, bool quoteTestDescription , Utf8String opType , int initialCount)
{
    FILE* logFile = nullptr;

    BeFileName dir;
    BeTest::GetHost().GetOutputRoot(dir);
    dir.AppendToPath(L"PerformanceTestResults");
    if (!dir.DoesPathExist())
        BeFileName::CreateNewDirectory(dir.c_str());

    dir.AppendToPath(L"PerformanceResults.csv");

    bool existingFile = dir.DoesPathExist();

    BeFile::Fopen(&logFile, dir.GetNameUtf8().c_str(), "a+");
    PERFORMANCELOG.infov(L"CSV Results filename: %ls\n", dir.GetName());

    if (!existingFile)
        fprintf(logFile, "DateTime,TestCaseName,TestName,ExecutionTime(s),opCount,TestDescription,OpType,InitialCount\n");

    Utf8CP quotes = quoteTestDescription ? "\"" : "";
    fprintf(logFile, "%s,%s,%s,%.6lf,%d,%s%s%s,%s,%i\n", DateTime::GetCurrentTimeUtc().ToString().c_str(), testcaseName, testName, timeInSeconds, opCount, quotes, testDescription, quotes , opType.c_str(), initialCount);
    fclose(logFile);
}

/*---------------------------------------------------------------------------------**//**
* Writes time to a csv file in newer format for reporting
*@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceResultRecorder::WriteResultsPerf(Utf8CP testcaseName, Utf8CP testName, double timeInSeconds, Utf8CP info)
{
    PerformanceResultRecorder::WriteResultsPerf(testcaseName, testName, "ExecutionTime", timeInSeconds, info);
}

/*---------------------------------------------------------------------------------**//**
* Writes performance test results to a csv file in newer format for reporting
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceResultRecorder::WriteResultsPerf(Utf8CP testcaseName, Utf8CP testName, Utf8CP valueDescription, double value, Utf8CP info)
{
    FILE* logFile = nullptr;

    BeFileName dir;
    BeTest::GetHost().GetOutputRoot(dir);
    dir.AppendToPath(L"PerfTestResults");
    if (!dir.DoesPathExist())
        BeFileName::CreateNewDirectory(dir.c_str());

    dir.AppendToPath(L"PerfResults.csv");

    bool existingFile = dir.DoesPathExist();

    BeFile::Fopen(&logFile, dir.GetNameUtf8().c_str(), "a+");
    PERFORMANCELOG.infov(L"CSV Results filename: %ls\n", dir.GetName());

    if (!existingFile)
        fprintf(logFile, "TestSuite,TestName,ValueDescription,Value,Date,Info\n");

    fprintf(logFile, "%s,%s,%s,%.6lf,%s,%s\n", testcaseName, testName, valueDescription, value, DateTime::GetCurrentTimeUtc().ToString().c_str(), info);
    fclose(logFile);
}

