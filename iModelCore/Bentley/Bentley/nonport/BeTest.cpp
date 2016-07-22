/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
    #include <windows.h>
#elif defined (BENTLEY_WINRT)
    #include <wrl.h>
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
#define BETEST_NO_INCLUDE_GTEST
#include <Bentley/BeTest.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>

USING_NAMESPACE_BENTLEY

static BeMutex s_bentleyCS;
static intptr_t                                 s_mainThreadId;                     // MT: set only during initialization
static BeAssertFunctions::T_BeAssertHandler*    s_assertHandler;                    // MT: s_bentleyCS
static bool                                     s_assertHandlerCanBeChanged=true;   // MT: s_bentleyCS
static bvector<BeTest::T_BeAssertListener*>     s_assertListeners;                  // MT: s_bentleyCS
static bool                                     s_failOnAssert[(int)BeAssertFunctions::AssertType::TypeCount]; // MT: Problem! If one thread sets this, it will affect assertions that fail on other threads. *** WIP_MT make this thread-local?
static bool                                     s_runningUnderGtest;                // indicates that we are running under gtest. MT: set only during initialization 
static bool                                     s_hadAssert;
static bool                                     s_hadAssertOnAnotherThread;         // MT: s_bentleyCS
static Utf8String                               s_currentTestCaseName;              // MT: set only when we run a test, which is always in the main thread
static Utf8String                               s_currentTestName;                  //  "       "
static RefCountedPtr<BeTest::Host>              s_host;                             // MT: set only during initialization. Used on multiple threads. Must be thread-safe internally.
bool BeTest::s_loop = true;

static std::map<Utf8String, BeTest::TestCaseInfo*>* s_testCases;                        // MT: s_bentleyCS

/*---------------------------------------------------------------------------------**//**
* Default handler for test test and assertion failures. Handles failures by throwing 
* an exception. This stragey works on all platforms but may not be optimal for some,
* such as gtest.
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeTestThrowFailureHandler : BeTest::IFailureHandler
    {
    private:
    static bool s_disableThrows; 

    void ThrowException (WCharCP msg) THROW_SPECIFIER(CharCP)
        {
        if (!s_runningUnderGtest)
            {
            // Ignore BeAssert failures that occur when we are unwinding an assertion failure or a test failure. 
            // Note that throwing while unwinding an exception will terminate the process!
            if (s_disableThrows)
                return;
            s_disableThrows = true; // I rely on the test runner to call _OnFailureHandled to allow me clear this flag.
            }
        throw "failure";
        }

    public:
    virtual void _OnAssertionFailure (WCharCP msg) THROW_SPECIFIER(CharCP) {ThrowException(msg);}
    virtual void _OnUnexpectedResult (WCharCP msg) THROW_SPECIFIER(CharCP) {ThrowException(msg);}
    virtual void _OnFailureHandled() {s_disableThrows=false;}
    };
bool BeTestThrowFailureHandler::s_disableThrows;
static BeTestThrowFailureHandler                s_defaultFailureHandler;
static BeTest::IFailureHandler*                 s_IFailureHandler = &s_defaultFailureHandler;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::SetIFailureHandler (IFailureHandler& h) {s_IFailureHandler=&h;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::SetRunningUnderGtest () {s_runningUnderGtest=true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      11/2011
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
* @bsimethod                                    sam.wilson                      11/2011
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
* @bsimethod                                    sam.wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::BeAssertFunctions::DefaultAssertionFailureHandler (WCharCP message, WCharCP file, unsigned line)
    {
#if !defined (BENTLEY_WINRT) // *** WIP_WINRT_GETENV
    CharCP env = getenv ("MS_IGNORE_ASSERTS");
    if (NULL != env && (*env == '1' || tolower(*env) == 't'))
        return;
#endif

    BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->fatalv (L"ASSERTION FAILURE: %ls (%ls:%d)\n", message, file, line);

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
* @bsimethod                                    John.Gooding                    05/2008
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
* @bsimethod                                    Chuck.Kirschman                    04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::BeAssertFunctions::PerformBeDataAssert (WCharCP message, WCharCP file, unsigned line)
    {
#if !defined (BENTLEY_WINRT) // *** WIP_WINRT_GETENV
    CharCP env = getenv ("MS_IGNORE_DATA_ASSERTS");
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
* @bsimethod                                    Sam.Wilson                      08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::BeAssertFunctions::SetBeAssertHandler (T_BeAssertHandler* h)
    {
    s_bentleyCS.lock();
    if (s_assertHandlerCanBeChanged)
        s_assertHandler = h;
    s_bentleyCS.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::BeAssertFunctions::SetBeTestAssertHandler (T_BeAssertHandler* h)
    {
    s_bentleyCS.lock();
    s_assertHandlerCanBeChanged = false;
    s_assertHandler = h;
    s_bentleyCS.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::GetAssertionFailed()
    {
    BeMutexHolder lock(s_bentleyCS);
    return s_hadAssert;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeTest::SetBeAssertListener (T_BeAssertListener* l)
    {
    s_bentleyCS.lock();
    s_assertListeners.push_back(l);
    s_bentleyCS.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void beTestAssertionFailureHandler (WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType atype)
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
        BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->infov (L"Ignoring assert %ls at %ls:%d\n", _Message, _File, _Line);
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

    BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->error (assertionFailure.c_str());

    if (BeTest::GetBreakOnFailure())
        {
        BeTest::BreakInDebugger ("BeAssert", "ASSERT!");
        }

    if (!s_runningUnderGtest && BeThreadUtilities::GetCurrentThreadId () != s_mainThreadId)
        {
        BeMutexHolder holder (s_bentleyCS);
        s_hadAssertOnAnotherThread = true;
        return;
        }

    s_IFailureHandler->_OnAssertionFailure (assertionFailure.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
static void onInvalidParameter
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

    BentleyApi::NativeLogging::LoggingManager::GetLogger(L"BeAssert")->error(parameterFailure.c_str());

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
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::Host&  BeTest::GetHost () {return *s_host;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::Initialize (Host& host)
    {
    s_host = &host;

    for (size_t i = 0; i<_countof(s_failOnAssert); ++i)
        s_failOnAssert[i] = true;

    s_failOnAssert[(int)BeAssertFunctions::AssertType::Data] = false;

    s_mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    BeAssertFunctions::SetBeTestAssertHandler(beTestAssertionFailureHandler);   // steals asserthandler and prevents any else from overriding it!

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

    _set_invalid_parameter_handler(onInvalidParameter);

#elif defined (BENTLEY_WINRT)

    //RoInitialize(RO_INIT_MULTITHREADED);

    // We do this to cause asserts to always output to a message box.  Without this the gtest process will abort without giving the developer the opportunity to 
    // attach with a debugger and analyze the situation.
    _set_error_mode(_OUT_TO_STDERR);

    // Suppress exception pop-ups.
    //_set_abort_behavior(0, _CALL_REPORTFAULT);
    //SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

    //_set_invalid_parameter_handler(onInvalidParameter);
#elif defined (__unix__)

    // no special set-up required

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::Uninitialize ()
    {
    s_host = NULL;

//#if defined (BENTLEY_WINRT)
//RoUninitialize();
//#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
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
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::LoadTestList (Utf8String& filters, BeFileName const& ignoreDir)
    {
    BeFileListIterator it (ignoreDir, true);
    BeFileName fn;
    while (it.GetNextFileName (fn) == SUCCESS)
        {
        Utf8String afn (fn);
        FILE* fp = fopen (afn.c_str(), "r");
        ReadTestList (filters, fp);
        fclose (fp);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
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
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static bool doesFilterApply (bvector<Utf8String> const& filters, CharCP testFullName)
    {
    FOR_EACH(Utf8String const& filter, filters)
        {
        std::regex re (filter.c_str());
        if (std::regex_match (testFullName, testFullName+strlen(testFullName), re))
            return true;
        } 
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::ShouldRunTest (CharCP fullTestName)
    {
    CharCP dot = strchr (fullTestName, '.');
    if (dot != nullptr && 0==strncmp (dot+1, "DISABLED_", 9))
        return false;

    bool doRun = s_runList.empty() || doesFilterApply (s_runList, fullTestName);

    bool doIgnore = !s_ignoreList.empty() && doesFilterApply (s_ignoreList, fullTestName);

    return doRun && !doIgnore;
    }

struct FromFilterToRegex
{
void operator () (Utf8String& str)
    {
    str.ReplaceAll ("\r", "");
    str.ReplaceAll (".", "\\.");
    str.ReplaceAll ("*", ".*");
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::SetRunFilters (bvector<Utf8String> const& toignore, bvector<Utf8String> const& torun)
    {
    s_ignoreList = toignore;
    s_runList = torun;
    std::for_each (s_ignoreList.begin(), s_ignoreList.end(), FromFilterToRegex());
    std::for_each (s_runList.begin(), s_runList.end(), FromFilterToRegex());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::EqNear (double v1, double v2) {return fabs(v1-v2) <= BeNumerical::ComputeComparisonTolerance(v1,v2);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::EqTol (double v1, double v2, double tol) {return fabs(v1-v2) <= tol;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::EqTol (int    v1, int    v2, int    tol) {return  abs(v1-v2) <= tol;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2015
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
* @bsimethod                                    Grigas.Petraitis                01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::EqStr (WCharCP s1, WCharCP s2, bool ignoreCase)
    {
    if (ignoreCase)
        return 0 == BeStringUtilities::Wcsicmp (s1, s2);
    else
        return 0 == wcscmp (s1, s2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult::ExpectedResult (bool isAsExpected, CharCP actualExpression, CharCP expectedExpression, CharCP fileName, size_t  lineNum, bool abortImmediately)
    {
    m_abortImmediately = false;
    if (isAsExpected)
        return;

    char lnStr [32];
    sprintf (lnStr, ":%d", (int)lineNum);

    m_message = "";
    m_message.append (actualExpression);
    m_message.append (" **!!** ");
    m_message.append (expectedExpression);
    m_message.append (" @ "); m_message.append (fileName); m_message.append (lnStr);
    m_abortImmediately = abortImmediately;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult::ExpectedResult (bool isAsExpected, CharCP actualValue, CharCP expectedValue, bool expectedEq, CharCP actualExpression, CharCP expectedExpression, CharCP fileName, size_t  lineNum, bool abortImmediately)
    {
    m_abortImmediately = false;
    if (isAsExpected)
        return;

    char lnStr [32];
    sprintf (lnStr, ":%d", (int)lineNum);

    m_message = "";
    m_message.append ("("); m_message.append (actualExpression); m_message.append ("="); m_message.append(actualValue); m_message.append(")");
    m_message.append (expectedEq? " was expected to equal ": " was not expected to equal ");
    m_message.append ("("); m_message.append (expectedExpression); m_message.append ("="); m_message.append(expectedValue); m_message.append(")"); 
    m_message.append (" @ "); m_message.append (fileName); m_message.append (lnStr);
    m_abortImmediately = abortImmediately;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult::~ExpectedResult() THROW_SPECIFIER(CharCP)
    {
    if (m_message.empty())
        return;
    BeTest::IncrementErrorCount ();
    BentleyApi::NativeLogging::LoggingManager::GetLogger(L"TestRunner")->fatalv ("%s\n", m_message.c_str());
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
* @bsimethod                                    Sam.Wilson                      11/2011
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
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (int64_t val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    sprintf (details, "%" PRId64, val);
    m_message.append (details);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (int32_t val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    sprintf (details, "%d", (int)val);  // the case is there to work around int/long ambiguity on Mac
    m_message.append (details);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (uint32_t val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    sprintf (details, "%u", (unsigned int)val);     // the cast is there to work around int/long ambiguity on Mac
    m_message.append (details);
    return *this;
    }

#if defined(__clang__) && defined(__APPLE__)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (size_t val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    sprintf (details, "%u", (unsigned int)val);     // the cast is there to work around int/long ambiguity on Mac
    m_message.append (details);
    return *this;
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (uint64_t val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    sprintf (details, "%" PRIu64, val);
    m_message.append (details);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (double val)
    {
    if (m_message.empty())
        return *this;
    char details[32];
    sprintf (details, "%lf", val);
    m_message.append (details);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (CharCP msg)
    {
    if (m_message.empty())
        return *this;
    m_message.append (msg);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (Utf8String const& msg)
    {
    if (m_message.empty())
        return *this;
    m_message.append (msg);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult& BeTest::ExpectedResult::operator<< (WString const& msg)
    {
    if (m_message.empty())
        return *this;
    m_message.append (Utf8String(msg));
    return *this;
    }

#if defined (EXPERIMENT_COMMENT_OFF)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::PrimitiveValueUnion::PrimitiveValueUnion ( bool  v):   m_ivalue(v), m_type(TYPE_BOOL)    {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion ( int8_t v):   m_ivalue(v), m_type(INTEGRAL)    {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion (uint8_t v):   m_ivalue(v), m_type(UINTEGRAL)   {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion ( int16_t v):   m_ivalue(v), m_type(INTEGRAL)    {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion (uint16_t v):   m_ivalue(v), m_type(UINTEGRAL)   {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion ( int32_t v):   m_ivalue(v), m_type(INTEGRAL)    {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion (uint32_t v):   m_ivalue(v), m_type(UINTEGRAL)   {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion ( int64_t v):   m_ivalue(v), m_type(INTEGRAL)    {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion (uint64_t v):   m_ivalue(v), m_type(UINTEGRAL)   {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion (double v):   m_dvalue(v), m_type(DOUBLE) {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion (void const* v):m_pvalue((void*)v), m_type(VOIDSTAR) {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion (CharCP v):   m_pvalue((void*)v), m_type(CHARCP) {;}
BeTest::PrimitiveValueUnion::PrimitiveValueUnion (WCharCP v):  m_pvalue((void*)v), m_type(WCHARCP) {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::PrimitiveValueUnion BeTest::PrimitiveValueUnion::PromoteTo (Type t) const
    {
    if (m_type == t)
        return *this;

    switch (m_type)
        {
        case INTEGRAL: case UINTEGRAL:
            switch (t)
                {
                case DOUBLE:    return PrimitiveValueUnion ((double)m_ivalue);
                case VOIDSTAR:  return PrimitiveValueUnion ((void*) m_ivalue);
                }
            break;    
        }
    BeAssert (false && "Invalid conversion");
    return PrimitiveValueUnion (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeTest::PrimitiveValueUnion::operator==(PrimitiveValueUnion const& rhs) const
    {
    if (m_type == DOUBLE || rhs.m_type == DOUBLE)
        return PromoteTo(DOUBLE).m_dvalue == rhs.PromoteTo(DOUBLE).m_dvalue;

    if (m_type == VOIDSTAR || rhs.m_type == VOIDSTAR)
        return PromoteTo(VOIDSTAR).m_pvalue == rhs.PromoteTo(VOIDSTAR).m_pvalue;

    switch (m_type)
        {
        case CHARCP: return 0==strcmp((CharCP)m_pvalue, (CharCP)rhs.m_pvalue); 
        case WCHARCP: return 0==wcscmp((WCharCP)m_pvalue, (WCharCP)rhs.m_pvalue); 
        }

    return m_ivalue == rhs.m_ivalue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeTest::PrimitiveValueUnion::ToString () const
    {
    char buf[64];
    switch (m_type)
        {
        case CHARCP:    return (CharCP) m_pvalue;
        case WCHARCP:   return Utf8String ((WCharCP)m_pvalue);
        case DOUBLE:    sprintf (buf, "%lf", m_dvalue); return buf;
        case VOIDSTAR:  sprintf (buf, "%p",  m_pvalue); return buf;
        case TYPE_BOOL: return m_ivalue? "true": "false";
        }
    sprintf (buf, (m_type == UINTEGRAL)? "%x": "%d", m_ivalue);
    return buf;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::ExpectedResult BeTest::CheckResultEQ (PrimitiveValueUnion const& a, PrimitiveValueUnion const& x, bool expectEq, CharCP aexp, CharCP xexp, CharCP file, uint32_t ln, bool fatal)
    {
    if ((a == x) == expectEq)
        return ExpectedResult (true,"","",file,ln,fatal);
    return ExpectedResult (false, a.ToString().c_str(), x.ToString().c_str(), expectEq, aexp, xexp, file, ln, fatal);
    }
#endif //defined (EXPERIMENT_COMMENT_OFF)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String>             BeTest::s_runList;
bvector<Utf8String>             BeTest::s_ignoreList;
bset<Utf8String>                BeTest::s_failedTests;
size_t                          BeTest::s_errorCount;
bool                            BeTest::s_breakOnFailure;

void    BeTest::ClearErrorCount()                   {s_failedTests.clear(); s_errorCount=0;}
size_t  BeTest::GetErrorCount()                     {return s_errorCount;}
void    BeTest::IncrementErrorCount()               {++s_errorCount;}
void    BeTest::SetBreakOnFailure(bool b)           {s_breakOnFailure=b;}
bool    BeTest::GetBreakOnFailure()                 {return s_breakOnFailure;}

static void incrementErrorCountAndEnableThrows()    {BeTest::IncrementErrorCount(); s_IFailureHandler->_OnFailureHandled();}

static void rethrowAssertFromOtherTreads () 
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
    BentleyApi::NativeLogging::LoggingManager::GetLogger(L"TestRunner")->fatalv ("%s FAILED\n", tname.c_str());
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
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void testing::Test::RunTest()
    {
    try {
        TestBody();
        rethrowAssertFromOtherTreads ();
        } 
    catch(...) 
        {
        incrementErrorCountAndEnableThrows();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void testing::Test::Run()
    {
    BentleyApi::NativeLogging::LoggingManager::GetLogger(L"TestRunner")->infov ("%s.%s", GetTestCaseNameA(), GetTestNameA());
    
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
            rethrowAssertFromOtherTreads ();
            }
        catch(...)
            {
            incrementErrorCountAndEnableThrows();
            }
        }
    BE_TEST_SEH_EXCEPT( incrementErrorCountAndEnableThrows(); )

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
            rethrowAssertFromOtherTreads ();
            } 
        catch(...) 
            {
            incrementErrorCountAndEnableThrows();
            }
        }
    BE_TEST_SEH_EXCEPT( incrementErrorCountAndEnableThrows(); )
    
    s_IFailureHandler->_OnFailureHandled();

    if (BeTest::GetErrorCount() > e)
        {
        BeTest::RecordFailedTest (*this);
        }

    s_currentTestCaseName.clear();
    s_currentTestName.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeTest::TestCaseInfo* BeTest::RegisterTestCase(Utf8CP tcname, T_SetUpFunc s, T_TearDownFunc t)
    {
    BeMutexHolder lock(s_bentleyCS);

    if (nullptr == s_testCases)
        s_testCases = new std::map<Utf8String, BeTest::TestCaseInfo*>();

    auto& tci = (*s_testCases)[tcname];
    if (nullptr == tci)
        {
        tci = new TestCaseInfo;
        tci->m_setUp = s;
        tci->m_tearDown = t;
        tci->m_isSetUp = false;
        }

    return tci;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::SetUpTestCase(Utf8CP tcname)
    {
    BeMutexHolder lock(s_bentleyCS);

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
    if (!tci->m_isSetUp)
        {
        tci->m_isSetUp = true;
        tci->m_setUp();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTest::TearDownTestCase(Utf8CP tcname)
    {
    BeMutexHolder lock(s_bentleyCS);
    
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
    if (!tci->m_isSetUp)
        {
        BeAssert(false && "attempt to tear down test case that was not set up");
        }
    else
        {
        tci->m_isSetUp = false;
        itci->second->m_tearDown();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void* BeTest::Host::InvokeP (char const* f, void* a)            {return _InvokeP (f, a);}
void  BeTest::Host::GetDocumentsRoot (BeFileName& path)         {_GetDocumentsRoot (path);}
void  BeTest::Host::GetDgnPlatformAssetsDirectory (BeFileName& path) {_GetDgnPlatformAssetsDirectory (path);}
void  BeTest::Host::GetOutputRoot (BeFileName& path)            {_GetOutputRoot (path);}
void  BeTest::Host::GetTempDir (BeFileName& path)               {_GetTempDir (path);}
void  BeTest::Host::GetFrameworkSqlangFiles (BeFileName& path)  {_GetFrameworkSqlangFiles(path);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeTest::Log (Utf8CP category, LogPriority priority, Utf8CP message)
    {
    NativeLogging::LoggingManager::GetLogger(category)->messagev (category, (NativeLogging::SEVERITY)priority, "%s", message);
    }

/*---------------------------------------------------------------------------------**//**
* Writes time to a csv file
*@bsimethod                                            Majd.Uddin          10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceResultRecorder::writeResults(Utf8String testcaseName, Utf8String testName, double timeInSeconds, Utf8String testDescription, int opCount)
{
    FILE* logFile = NULL;

    BeFileName dir;
    BeTest::GetHost().GetOutputRoot(dir);
    dir.AppendToPath(L"PerformanceTestResults");
    if (!dir.DoesPathExist())
        BeFileName::CreateNewDirectory(dir.c_str());

    dir.AppendToPath(L"PerformanceResults.csv");

    bool existingFile = dir.DoesPathExist();

    logFile = fopen(dir.GetNameUtf8().c_str(), "a+");
    PERFORMANCELOG.infov(L"CSV Results filename: %ls\n", dir.GetName());

    if (!existingFile)
        fprintf(logFile, "DateTime, TestCaseName, TestName, ExecutionTime, TestDescription, opCount\n");
    tm t;
    BeTimeUtilities::ConvertUnixMillisToLocalTime(t, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    int year = t.tm_year + 1900; //it is always from year 1900
    int month = t.tm_mon + 1; // it is from Jan
    fprintf(logFile, "%d-%d-%dT%d:%d:%d, %s, %s, %.6lf, \"%s\", %d\n", year, month, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, testcaseName.c_str(), testName.c_str(), timeInSeconds, testDescription.c_str(), opCount);

    fclose(logFile);
}

