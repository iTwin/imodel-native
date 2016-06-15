/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeThread.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
#include <thread>
#include <mutex>
#include <condition_variable>

USING_NAMESPACE_BENTLEY

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    static DWORD         toKey(void* k)        {return (DWORD)         (intptr_t)k;}
    static void*         toPtr(DWORD k)        {return (void*)         (intptr_t)k;}
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        static pthread_key_t toKey(void* k)        {return (pthread_key_t) (intptr_t)k;}
        static void*         toPtr(pthread_key_t k){return (void*)         (intptr_t)k;}
    #else
        static void*         toPtr(void* k)        {return k;}
    #endif
#else
#error unknown runtime
#endif

BeMutex* s_systemCS;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeThreadLocalStorage::BeThreadLocalStorage()
    {
#if defined (BENTLEY_WIN32)
    DWORD key = TlsAlloc();
#elif defined (BENTLEY_WINRT)
    // note: FLS acts the same as TLS if you don't create fibers
    DWORD key = FlsAlloc(NULL);
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_key_t key;
        pthread_key_create(&key, NULL);
    #else
        void* key = NULL;
    #endif
#else
#error unknown runtime
#endif
    m_key = toPtr(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeThreadLocalStorage::~BeThreadLocalStorage()
    {
#if defined (BENTLEY_WIN32)
    TlsFree(toKey(m_key));
#elif defined (BENTLEY_WINRT)
    // note: FLS acts the same as TLS if you don't create fibers
    FlsFree(toKey(m_key));
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_key_delete(toKey(m_key));
    #endif
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThreadLocalStorage::SetValueAsPointer(void* v)
    {
#if defined (BENTLEY_WIN32)
    TlsSetValue(toKey(m_key), v);
#elif defined (BENTLEY_WINRT)
    // note: FLS acts the same as TLS if you don't create fibers
    FlsSetValue(toKey(m_key), v);
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_setspecific(toKey(m_key), v);
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
void* BeThreadLocalStorage::GetValueAsPointer()
    {
#if defined (BENTLEY_WIN32)
    return TlsGetValue(toKey(m_key));
#elif defined (BENTLEY_WINRT)
    // note: FLS acts the same as TLS if you don't create fibers
    return FlsGetValue(toKey(m_key));
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        return pthread_getspecific(toKey(m_key));
    #else
        return m_value;
    #endif
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t BeThreadUtilities::GetHardwareConcurrency()
    {
    return std::thread::hardware_concurrency();
    }

//=======================================================================================
// The classe BeMutex, BeMutexHolder, and BeConditionVariable are each meant to be implemented by
// std::recursive_mutex, std::unique_lock, and std::condition_variable_any respectively. All of this nonsense below 
// is due to the fact that the MSVC compiler in /clr mode doesn't support them. Hence,
// we can't include <mutex>, or <condition_variable> in our header files and we must hide those 
// classes inside this file. The public types must be at least as large as the std types, and then
// we simply forward the methods to them below.
//=======================================================================================
static_assert (sizeof(BeMutex) == sizeof(std::recursive_mutex), "BeMutex wrong size");
static_assert (sizeof(BeMutex) == BEMUTEX_DATA_ARRAY_LENGTH * sizeof(void*), "BEMUTEX_DATA_ARRAY_LENGTH wrong size");
static_assert (sizeof(BeMutexHolder) == sizeof(std::unique_lock<std::recursive_mutex>), "BeMutexHolder wrong size");
static std::recursive_mutex& to_mutex(BeMutex* bmutex){return *(std::recursive_mutex*)bmutex;}
static std::unique_lock<std::recursive_mutex>& to_uniquelock(BeMutexHolder* bholder){return *(std::unique_lock<std::recursive_mutex>*)bholder;}
static std::condition_variable_any& to_cv(BeConditionVariable* bcv){return *(std::condition_variable_any*)bcv;}

BeMutex::BeMutex()      {new (this) std::recursive_mutex();}
BeMutex::~BeMutex()     {to_mutex(this).~recursive_mutex();}
void BeMutex::lock()    {to_mutex(this).lock();}
void BeMutex::unlock()  {to_mutex(this).unlock();}
BeMutexHolder::BeMutexHolder(BeMutex& mutex, Lock lock)
    {
    if (lock == Lock::Yes)
        new (this) std::unique_lock<std::recursive_mutex>(to_mutex(&mutex));
    else
        new (this) std::unique_lock<std::recursive_mutex>(to_mutex(&mutex), std::defer_lock);
    }
BeMutexHolder::~BeMutexHolder() {to_uniquelock(this).~unique_lock();}
void BeMutexHolder::unlock()    {to_uniquelock(this).unlock();}
void BeMutexHolder::lock()      {to_uniquelock(this).lock();}
bool BeMutexHolder::owns_lock() {return to_uniquelock(this).owns_lock();}
BeConditionVariable::BeConditionVariable()  {new (this) std::condition_variable_any();}
BeConditionVariable::~BeConditionVariable() {to_cv(this).~condition_variable_any();}
void BeConditionVariable::InfiniteWait(BeMutexHolder& holder){to_cv(this).wait(to_uniquelock(&holder));}
bool BeConditionVariable::RelativeWait(BeMutexHolder& holder, uint32_t timeoutMillis) {return std::cv_status::timeout == to_cv(this).wait_for (to_uniquelock(&holder), std::chrono::milliseconds(timeoutMillis));}
void BeConditionVariable::notify_all() {to_cv(this).notify_all();}
void BeConditionVariable::notify_one() {to_cv(this).notify_one();}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2012
//--------------+------------------------------------------------------------------------
bool BeConditionVariable::ProtectedWaitOnCondition(BeMutexHolder& holder, IConditionVariablePredicate* condition, uint32_t timeoutMillis)
    {
    static_assert (sizeof(m_osCV) >= sizeof(std::condition_variable_any), "BeConditionVariable wrong size");

    bool    conditionSatisfied = nullptr == condition ? false : condition->_TestCondition(*this);
    bool    timedOut           = timeoutMillis == 0;

    while (!conditionSatisfied && (Infinite == timeoutMillis || !timedOut))
        {
        if (Infinite == timeoutMillis)
            InfiniteWait(holder);
        else  
            {
            uint32_t    startTicks = BeTimeUtilities::QueryMillisecondsCounterUInt32();

            timedOut = RelativeWait(holder, timeoutMillis);

            uint32_t    elapsedTicks = BeTimeUtilities::QueryMillisecondsCounterUInt32() - startTicks;
            timeoutMillis = elapsedTicks > timeoutMillis ? 0 : (timeoutMillis - elapsedTicks);
            }

        conditionSatisfied = nullptr == condition ? true : condition->_TestCondition(*this);
        }

    return conditionSatisfied;
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
        return (intptr_t) pthread_self();
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
uint32_t BeNumerical::ResetFloatingPointExceptions(uint32_t newFpuMask)
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    _clearfp();
    _fpreset();

    if (0 == newFpuMask)
        newFpuMask = _controlfp(0, 0);

    uint32_t retval =  _controlfp(newFpuMask, MCW_EM);
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
void BeThreadUtilities::BeSleep(uint32_t millis)
    {
#if defined (BENTLEY_WIN32)

    ::Sleep(millis);

#elif defined (BENTLEY_WINRT)

    // Thanks to http://blogs.msdn.com/b/shawnhar/archive/2012/03/12/createthread-for-windows-8-metro.aspx for the work-around idea

    static HANDLE s_sleepEvent = nullptr;
    if (s_sleepEvent == nullptr)
        {
        BeSystemMutexHolder singletonlock;
        if (s_sleepEvent == nullptr)
            {
            s_sleepEvent = CreateEventExW (nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
            if (s_sleepEvent == nullptr)
                return;
            }
        }

    // Emulate sleep by waiting with timeout on an event that is never signalled.
    WaitForSingleObjectEx(s_sleepEvent, millis, false);

#elif defined (__unix__)

    struct timespec sleepSpec;
    sleepSpec.tv_sec = millis/1000;
    sleepSpec.tv_nsec = (millis % 1000) * 1000000;
    nanosleep(&sleepSpec, nullptr);

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSystemMutexHolder::StartupInitializeSystemMutex()
    {
    if (nullptr == s_systemCS)
        s_systemCS = new BeMutex();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeMutex& BeSystemMutexHolder::GetSystemMutex()
    {
    StartupInitializeSystemMutex();
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

    THREADNAME_INFO threadNameInfo(newName);
    __try {RaiseException(0x406D1388, 0, sizeof(threadNameInfo) / sizeof (ULONG_PTR), (ULONG_PTR*)&threadNameInfo);}
    __except (EXCEPTION_CONTINUE_EXECUTION){}
 #elif defined (__APPLE__)
    pthread_setname_np(newName);
 #endif

#endif // NDEBUG
    }

#if defined (BENTLEY_WINRT)
static T_ThreadStartHandler s_threadStartHandler = nullptr;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThreadUtilities::SetThreadStartHandler(T_ThreadStartHandler handler)
    {
    s_threadStartHandler = handler;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeThreadUtilities::StartNewThread(int stackSize, T_ThreadStart startAddr, void* arg)
    {
#if defined (__unix__)
    pthread_attr_t  threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setstacksize(&threadAttr, stackSize);

    pthread_t threadHandle;
    uintptr_t retval= pthread_create(&threadHandle, &threadAttr, startAddr, arg);
    if (0 == retval)
        pthread_detach(threadHandle);
    pthread_attr_destroy(&threadAttr);
    return (0 == retval) ? SUCCESS : ERROR;
#elif defined (BENTLEY_WIN32)
    uintptr_t handle = _beginthreadex(nullptr, (unsigned) stackSize, startAddr, arg, 0, nullptr);
    if (0 == handle)
        return ERROR;
    CloseHandle((HANDLE)handle);
    return SUCCESS;
#elif defined (BENTLEY_WINRT)
    if (nullptr == s_threadStartHandler)
        {
        BeAssert(false);
        return ERROR;
        }
    return (0 != s_threadStartHandler(startAddr, arg)) ? SUCCESS : ERROR;
#else
    #error unknown platform
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeThread::BeThread(Utf8CP threadName) : m_threadId(0), m_threadName(threadName) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP BeThread::GetThreadName() const 
    {
    return m_threadName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThread::RunThread(void* arg)
    {
    auto& thread = *(BeThread*) arg;
    thread.m_threadId = BeThreadUtilities::GetCurrentThreadId();
    if (!thread.m_threadName.empty())
        BeThreadUtilities::SetCurrentThreadName(thread.m_threadName.c_str());

    thread._Run();

    thread.m_threadId = 0;
    thread.m_threadName.clear();
    thread.Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThread::Start()
    {
    AddRef();
    if (SUCCESS != BeThreadUtilities::StartNewThread(1024 * 1024, RunPlatformThread, this))
        {
        BeAssert(false);
        Release();
        }
    }
