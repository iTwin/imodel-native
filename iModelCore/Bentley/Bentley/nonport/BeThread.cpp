/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeThread.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #include <windows.h>
    #include <objbase.h>
    #include <process.h>
    #if defined (BENTLEY_WINRT)
        #include <processthreadsapi.h>
    #endif
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        #include <pthread.h>
    #endif
    #include <errno.h>
    #include <unistd.h>
    #include <sys/param.h>
    #include <sys/mman.h>
#else
    #error unknown compiler
#endif

#include "../BentleyInternal.h"

#include <stdlib.h>
#include <stdio.h>
#include <Bentley/BeDebugLog.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeAssert.h>
#include <Bentley/WString.h>

USING_NAMESPACE_BENTLEY

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    static DWORD         toKey (void* k)        {return (DWORD)         (intptr_t)k;}
    static void*         toPtr (DWORD k)        {return (void*)         (intptr_t)k;}
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        static pthread_key_t toKey (void* k)        {return (pthread_key_t) (intptr_t)k;}
        static void*         toPtr (pthread_key_t k){return (void*)         (intptr_t)k;}
    #else
        static void*         toPtr (void* k)        {return k;}
    #endif
#else
#error unknown runtime
#endif

BeCriticalSection* BeCriticalSection::s_systemCS;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeThreadLocalStorage::BeThreadLocalStorage ()
    {
#if defined (BENTLEY_WIN32)
    DWORD key = TlsAlloc ();
#elif defined (BENTLEY_WINRT)
    // note: FLS acts the same as TLS if you don't create fibers
    DWORD key = FlsAlloc (NULL);
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_key_t key;
        pthread_key_create (&key, NULL);
    #else
        void* key = NULL;
    #endif
#else
#error unknown runtime
#endif
    m_key = toPtr (key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeThreadLocalStorage::~BeThreadLocalStorage ()
    {
#if defined (BENTLEY_WIN32)
    TlsFree (toKey (m_key));
#elif defined (BENTLEY_WINRT)
    // note: FLS acts the same as TLS if you don't create fibers
    FlsFree (toKey (m_key));
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_key_delete (toKey(m_key));
    #endif
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThreadLocalStorage::SetValueAsPointer (void* v)
    {
#if defined (BENTLEY_WIN32)
    TlsSetValue (toKey(m_key), v);
#elif defined (BENTLEY_WINRT)
    // note: FLS acts the same as TLS if you don't create fibers
    FlsSetValue (toKey (m_key), v);
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_setspecific (toKey(m_key), v);
    #else
        m_value = v;
    #endif
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void* BeThreadLocalStorage::GetValueAsPointer ()
    {
#if defined (BENTLEY_WIN32)
    return TlsGetValue (toKey(m_key));
#elif defined (BENTLEY_WINRT)
    // note: FLS acts the same as TLS if you don't create fibers
    return FlsGetValue (toKey (m_key));
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        return pthread_getspecific (toKey(m_key));
    #else
        return m_value;
    #endif
#else
#error unknown runtime
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
BeConditionVariable::BeConditionVariable(BeCriticalSection* csect) : m_ownCSect(false), m_csect (csect)
    {
    memset (m_conditionVariable, 0, sizeof (m_conditionVariable));
    if (NULL == m_csect)
        {
        m_csect = new BeCriticalSection();
        m_ownCSect = true;
        }

#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)
    BeAssert (sizeof (m_conditionVariable) >= sizeof (CONDITION_VARIABLE));
    InitializeConditionVariable((CONDITION_VARIABLE*)&m_conditionVariable[0]);
    m_isValid = true;
#elif defined (__unix__) || defined (__linux)
    #if defined (BETHREAD_USE_PTHREAD)
        BeAssert (sizeof (m_conditionVariable) >= sizeof (pthread_cond_t));

        //  WIP May want support for failed pthread_cond_init; if so, we should have a create 
        //  method that returns NULL instead of putting the pthread_cond_init call in the constructor
        m_isValid = pthread_cond_init ((pthread_cond_t*)&m_conditionVariable[0], NULL) == 0;
    #else
        #error BeConditionVariable not implemented for this platform
    #endif
#endif
    BeAssert (m_isValid);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2012
//--------------+------------------------------------------------------------------------
bool BeConditionVariable::GetIsValid() const
    {
    return m_csect->GetIsValid() && m_isValid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
BeConditionVariable::~BeConditionVariable()
    {
    if (m_ownCSect)
        delete m_csect;

#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)
    //  It looks like there is nothing to do to clean up a condition variable
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_cond_destroy ((pthread_cond_t*)&m_conditionVariable[0]);
    #endif
#endif
    }

#if defined (__unix__)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2012
//--------------+------------------------------------------------------------------------
static void addToTimeSpec (struct timespec& ts, uint64_t millis)
    {
    int64_t     oneBillion = 1000000000;
    int64_t     fullTime = ts.tv_sec * oneBillion + ts.tv_nsec;
    fullTime += millis * 1000000;
    ts.tv_sec = (int32_t)(fullTime/oneBillion);
    ts.tv_nsec = (int32_t)(fullTime % oneBillion);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2012
//--------------+------------------------------------------------------------------------
void BeConditionVariable::InfiniteWait()
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    CRITICAL_SECTION*   nativeCriticalSection = (CRITICAL_SECTION*) m_csect->GetNativeCriticalSection ();
    SleepConditionVariableCS ((CONDITION_VARIABLE*) &m_conditionVariable[0], nativeCriticalSection, INFINITE);
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_mutex_t*    nativeCriticalSection = (pthread_mutex_t*) m_csect->GetNativeCriticalSection ();
        int e = pthread_cond_wait ((pthread_cond_t*) &m_conditionVariable[0], nativeCriticalSection);
        if (EPERM == e || EINVAL == e)
            { BeAssert(false); }
    #endif
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2012
//--------------+------------------------------------------------------------------------
void BeConditionVariable::RelativeWait(bool&timedOut, uint32_t timeoutMillis)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    CRITICAL_SECTION*   nativeCriticalSection = (CRITICAL_SECTION*) m_csect->GetNativeCriticalSection ();
    BOOL    status          = SleepConditionVariableCS ((CONDITION_VARIABLE*) &m_conditionVariable[0], nativeCriticalSection, timeoutMillis);
    if (false == status && ERROR_TIMEOUT == GetLastError ())
        timedOut = true;
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_mutex_t*    nativeCriticalSection = (pthread_mutex_t*) m_csect->GetNativeCriticalSection ();
        pthread_cond_t* nativeCondVariable = (pthread_cond_t*) GetNativeConditionVariable ();
        struct timespec     timeoutSpec;

        #if !defined (__APPLE__)
            clock_gettime (CLOCK_REALTIME, &timeoutSpec);
            addToTimeSpec (timeoutSpec, timeoutMillis);
            int e = pthread_cond_timedwait (nativeCondVariable, nativeCriticalSection, &timeoutSpec);
        #else
            //  iOS does not support clock_gettime (CLOCK_REALTIME, &timeoutSpec).
            memset (&timeoutSpec, 0, sizeof (timeoutSpec));
            addToTimeSpec (timeoutSpec, timeoutMillis);
            int e = pthread_cond_timedwait_relative_np (nativeCondVariable, nativeCriticalSection, &timeoutSpec);
        #endif

        BeAssert (EPERM != e && EINVAL != e);
        timedOut = ETIMEDOUT == e;
    #endif
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2012
//--------------+------------------------------------------------------------------------
bool            BeConditionVariable::ProtectedWaitOnCondition(IConditionVariablePredicate* condition, uint32_t timeoutMillis)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    BeAssert (m_csect->HasEntered ());
    BeAssert (((CRITICAL_SECTION*) m_csect)->RecursionCount == 1);
#endif

    bool    conditionSatisfied = NULL == condition ? false : condition->_TestCondition (*this);
    bool    timedOut           = timeoutMillis == 0;

    while (!conditionSatisfied && (Infinite == timeoutMillis || !timedOut))
        {
        if (Infinite == timeoutMillis)
            {
            InfiniteWait ();
            }
        else   //  Caller wants a timeout
            {
            uint32_t    startTicks = BeTimeUtilities::QueryMillisecondsCounterUInt32();

            RelativeWait(timedOut, timeoutMillis);

            uint32_t    elapsedTicks = BeTimeUtilities::QueryMillisecondsCounterUInt32() - startTicks;
            timeoutMillis = elapsedTicks > timeoutMillis ? 0 : (timeoutMillis - elapsedTicks);
            }

        conditionSatisfied = NULL == condition ? true : condition->_TestCondition (*this);
        }

    return conditionSatisfied;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
bool            BeConditionVariable::WaitOnCondition(IConditionVariablePredicate* condition, uint32_t timeoutMillis)
    {
    BeCriticalSectionHolder holder (*m_csect);
    return ProtectedWaitOnCondition (condition, timeoutMillis);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
void BeConditionVariable::Wake (bool wakeAll)
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)
    if (wakeAll)
        WakeAllConditionVariable ((CONDITION_VARIABLE*)&m_conditionVariable[0]);
    else
        WakeConditionVariable  ((CONDITION_VARIABLE*)&m_conditionVariable[0]);
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        if (wakeAll)
            pthread_cond_broadcast((pthread_cond_t*)&m_conditionVariable[0]);
        else
            pthread_cond_signal((pthread_cond_t*)&m_conditionVariable[0]);
    #endif
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeCriticalSection::BeCriticalSection ()
    {
    memset (m_osCSect, 0, sizeof(m_osCSect));

#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    m_isValid = InitializeCriticalSectionEx ((CRITICAL_SECTION*)&m_osCSect, 0, 0) != 0;

#elif defined (__unix__)
    
    #if defined (BETHREAD_USE_PTHREAD)
        BeAssert (sizeof (m_osCSect) >= sizeof (pthread_mutex_t));
        pthread_mutexattr_t    attr;
        pthread_mutexattr_init (&attr);
        pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
        m_isValid = pthread_mutex_init ((pthread_mutex_t*)&m_osCSect, &attr) == 0;
        pthread_mutexattr_destroy (&attr);
    #else
        #error BeCriticalSection not implemented for this platform
    #endif

#else
#error unknown runtime
#endif
    BeAssert (m_isValid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeCriticalSection::~BeCriticalSection ()
    {
    m_isValid = false;

#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    DeleteCriticalSection ((CRITICAL_SECTION*)&m_osCSect);

#elif defined (__unix__)

    #if defined (BETHREAD_USE_PTHREAD)
        pthread_mutex_destroy ((pthread_mutex_t*)&m_osCSect);
    #endif

#else
#error unknown runtime
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BeCriticalSection::Enter ()
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    EnterCriticalSection ((CRITICAL_SECTION*)&m_osCSect);

#elif defined (__unix__)

    #if defined (BETHREAD_USE_PTHREAD)
        int e = pthread_mutex_lock ((pthread_mutex_t*)&m_osCSect);
        if (EINVAL == e)
            { BeAssert(false); }
    #endif

#else
#error unknown runtime
#endif
    }

#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeCriticalSection::HasEntered() const
    {
    return (GetCurrentThreadId() == (intptr_t)((CRITICAL_SECTION*)&m_osCSect)->OwningThread);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    BeCriticalSection::Leave ()
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    BeAssert (HasEntered());

    LeaveCriticalSection ((CRITICAL_SECTION*)&m_osCSect);

#elif defined (__unix__)

    #if defined (BETHREAD_USE_PTHREAD)
        int e = pthread_mutex_unlock ((pthread_mutex_t*)&m_osCSect);
        if (EPERM == e || EINVAL == e)
            { BeAssert(false); }
    #endif

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
intptr_t BeThreadUtilities::GetCurrentThreadId() 
    { 
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    return ::GetCurrentThreadId(); 

#elif defined (__unix__)

    #if defined (BETHREAD_USE_PTHREAD)
        //  Don't change this to gettid.  MobileDgnRPC uses 
        //  the value that pthread_create returns.  On Android that
        //  matches what pthread_self returns and does not match what
        //  gettid returns.
        return (intptr_t) pthread_self ();
    #else
        return (intptr_t)-1;
    #endif

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009  
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t BeNumerical::ResetFloatingPointExceptions (uint32_t newFpuMask)
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    _clearfp();
    _fpreset();

    if (0 == newFpuMask)
        newFpuMask = _controlfp (0, 0);

    uint32_t retval =  _controlfp (newFpuMask, MCW_EM);
    return retval;

#elif defined (__unix__)

    /*WIP_NONPORT*/
    return 0;

#else
#error unknown runtime
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThreadUtilities::BeSleep (uint32_t millis)
    {
#if defined (BENTLEY_WIN32)

    ::Sleep (millis);

#elif defined (BENTLEY_WINRT)

//	new System.Threading.ManualResetEvent(false).WaitOne(ms);

    // Thanks to http://blogs.msdn.com/b/shawnhar/archive/2012/03/12/createthread-for-windows-8-metro.aspx for the work-around idea

    static HANDLE s_sleepEvent = nullptr;
    if (s_sleepEvent == nullptr)
        {
        BeSystemCriticalSectionHolder singletonlock;
        if (s_sleepEvent == nullptr)
            {
            s_sleepEvent = CreateEventExW (nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
            if (s_sleepEvent == nullptr)
                return;
            }
        }

    // Emulate sleep by waiting with timeout on an event that is never signalled.
    WaitForSingleObjectEx (s_sleepEvent, millis, false);

#elif defined (__unix__)

    struct timespec sleepSpec;
    sleepSpec.tv_sec = millis/1000;
    sleepSpec.tv_nsec = (millis % 1000) * 1000000;
    nanosleep (&sleepSpec, NULL);

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BeCriticalSection::StartupInitializeSystemCriticalSection ()
    {
    if (NULL != s_systemCS)
        return;
    BeAssert (NULL == s_systemCS);
    s_systemCS = new BeCriticalSection();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeCriticalSection& BeCriticalSection::GetSystemCriticalSection ()
    {
    if (NULL == s_systemCS) // *** NEEDS WORK: This is not thread-safe. On the other hand, I don't want to change every app to call StartupInitializeSystemCriticalSection! My compromise is to change only the apps that might be multi-threaded to call StartupInitializeSystemCriticalSection.
        StartupInitializeSystemCriticalSection();

    BeAssert (NULL != s_systemCS);// "Call BeCriticalSection::StartupInitializeSystemCriticalSection at the start of your program");
    return *s_systemCS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThreadUtilities::SetCurrentThreadName(Utf8CP newName)
    {
#if !defined (NDEBUG)
 #if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    // see http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
    #pragma pack(push,8)
    struct THREADNAME_INFO
        {
        uint32_t m_dwType;
        LPCSTR m_szName;
        uint32_t m_dwThreadID;
        uint32_t m_dwFlags;
        explicit THREADNAME_INFO (Utf8CP newName) {m_dwType = 0x1000; m_szName = newName; m_dwThreadID = -1; m_dwFlags= 0;}
        };
    #pragma pack(pop)

    THREADNAME_INFO threadNameInfo (newName);
    __try {RaiseException (0x406D1388, 0, sizeof(threadNameInfo) / sizeof (ULONG_PTR), (ULONG_PTR*)&threadNameInfo);}
    __except (EXCEPTION_CONTINUE_EXECUTION){}
 #elif defined (__APPLE__)
    pthread_setname_np (newName);
 #endif

#endif // NDEBUG
    }

#if defined (BENTLEY_WINRT)
static T_ThreadStartHandler s_threadStartHandler = NULL;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThreadUtilities::SetThreadStartHandler (T_ThreadStartHandler handler)
    {
    s_threadStartHandler = handler;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t BeThreadUtilities::StartNewThread (int stackSize, T_ThreadStart startAddr, void* arg)
    {
#if defined (__unix__)
    pthread_attr_t  threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setstacksize (&threadAttr, stackSize);

    pthread_t threadHandle;
    uintptr_t retval= pthread_create(&threadHandle, &threadAttr, startAddr, arg);
    if (0 == retval)
        pthread_detach(threadHandle);
    pthread_attr_destroy(&threadAttr);
    return  retval;
#elif defined (BENTLEY_WIN32)
    return _beginthreadex(NULL, (unsigned) stackSize, startAddr, arg, 0, NULL);
#elif defined (BENTLEY_WINRT)
    if (NULL == s_threadStartHandler)
        {
        BeAssert (false);
    return 0;
        }
    return s_threadStartHandler (startAddr, arg);
#else
    #error unknown platform
#endif
    }
