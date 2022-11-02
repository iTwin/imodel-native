/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>

#if defined (BENTLEYCONFIG_OS_WINDOWS)
    #include <windows.h>
    #include <objbase.h>
    #include <process.h>
    #if defined (BENTLEY_WINRT)
        #include <processthreadsapi.h>
    #endif
#elif defined (__unix__)
    #include <pthread.h>
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

#if defined (BENTLEYCONFIG_OS_WINDOWS)
    static DWORD toKey(void* k) {return (DWORD) (intptr_t)k;}
    static void* toPtr(DWORD k) {return (void*) (intptr_t)k;}
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        static pthread_key_t toKey(void* k) {return (pthread_key_t) (intptr_t)k;}
        static void* toPtr(pthread_key_t k){return (void*) (intptr_t)k;}
    #else
        static void* toPtr(void* k) {return k;}
    #endif
#else
#error unknown runtime
#endif

BeMutex* s_systemCS;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void* BeThreadLocalStorage::Create(BeThreadLocalStorage::Destructor destructor)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)
    // note: FLS acts the same as TLS if you don't create fibers
    DWORD key = FlsAlloc(destructor);
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_key_t key;
        pthread_key_create(&key, destructor);
    #else
        void* key = NULL;
    #endif
#else
#error unknown runtime
#endif
    return toPtr(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThreadLocalStorage::Delete(void* key)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)
    // note: FLS acts the same as TLS if you don't create fibers
    FlsFree(toKey(key));
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_key_delete(toKey(key));
    #endif
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThreadLocalStorage::SetValue(void* key, void* v)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)
    // note: FLS acts the same as TLS if you don't create fibers
    FlsSetValue(toKey(key), v);
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        pthread_setspecific(toKey(key), v);
    #else
        m_value = v;
    #endif
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void* BeThreadLocalStorage::GetValue(void* key)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)
    // note: FLS acts the same as TLS if you don't create fibers
    return FlsGetValue(toKey(key));
#elif defined (__unix__)
    #if defined (BETHREAD_USE_PTHREAD)
        return pthread_getspecific(toKey(key));
    #else
        return m_value;
    #endif
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
BeMutex::~BeMutex()     {to_mutex(this).~recursive_mutex(); /* May assert if mutex is still locked */ }
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
// @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t BeNumerical::ResetFloatingPointExceptions(uint32_t newFpuMask)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)

    _clearfp();
    _fpreset();

    if (0 == newFpuMask)
PUSH_DISABLE_DEPRECATION_WARNINGS
        newFpuMask = _controlfp(0, 0);

    uint32_t retval =  _controlfp(newFpuMask, MCW_EM);
    return retval;
POP_DISABLE_DEPRECATION_WARNINGS

#elif defined (__unix__)

    /*WIP_NONPORT*/
    return 0;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeDuration::Sleep()
    {
    if (!IsTowardsFuture())
        return;

    std::this_thread::sleep_for(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSystemMutexHolder::StartupInitializeSystemMutex()
    {
    if (nullptr == s_systemCS)
        s_systemCS = new BeMutex();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeMutex& BeSystemMutexHolder::GetSystemMutex()
    {
    StartupInitializeSystemMutex();
    return *s_systemCS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThreadUtilities::SetCurrentThreadName(Utf8CP newName)
    {
#if !defined (NDEBUG)
 #if defined (BENTLEYCONFIG_OS_WINDOWS)
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
    PUSH_REVIEWED_STATIC_ANALYSIS_WARNING(6320 6322)
    __try {RaiseException(0x406D1388, 0, sizeof(threadNameInfo) / sizeof (ULONG_PTR), (ULONG_PTR*)&threadNameInfo);}
    __except (EXCEPTION_EXECUTE_HANDLER){}
    POP_REVIEWED_STATIC_ANALYSIS_WARNING
 #elif defined (__APPLE__)
    pthread_setname_np(newName);
 #elif defined (__unix__)
    pthread_t threadid = pthread_self();
    pthread_setname_np(threadid, newName);
 #endif

#endif // NDEBUG
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeThreadUtilities::GetDefaultStackSize()
    {
    //on linux, we can only use 16k, 2MB, 8MB, etc
    //8MB is default on linux
    return 2 * 1024 * 1024;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeThreadUtilities::StartNewThread(T_ThreadStart startAddr, void* arg, int stackSize)
    {
#if defined (__unix__)
    int result;
    pthread_attr_t  threadAttr;
    result = pthread_attr_init(&threadAttr);
    UNUSED_VARIABLE(result);

    BeAssert(0 == result);
    result = pthread_attr_setstacksize(&threadAttr, stackSize);
    BeAssert(0 == result);

    pthread_t threadHandle;
    int retval= pthread_create(&threadHandle, &threadAttr, startAddr, arg);
    BeAssert(0 == retval);
    if (0 == retval)
    {
        result = pthread_detach(threadHandle);
        BeAssert(0 == result);
    }
    result = pthread_attr_destroy(&threadAttr);
    BeAssert(0 == result);
    return (0 == retval) ? SUCCESS : ERROR;
#elif defined(BENTLEYCONFIG_OS_WINDOWS)
    uintptr_t handle = _beginthreadex(nullptr, (unsigned) stackSize, startAddr, arg, 0, nullptr);
    if (0 == handle)
        return ERROR;
    CloseHandle((HANDLE)handle);
    return SUCCESS;
#else
    #error unknown platform
    return ERROR;
#endif
    }

#if defined (BENTLEY_CHANGE)
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeThread::BeThread(Utf8CP threadName) : m_threadId(0), m_threadName(threadName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP BeThread::GetThreadName() const
    {
    return m_threadName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeThread::Start()
    {
    AddRef();
    if (SUCCESS != BeThreadUtilities::StartNewThread(BeThreadUtilities::GetDefaultStackSize(), RunPlatformThread, this))
        {
        BeAssert(false);
        Release();
        }
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint64_t BeThreadUtilities::GetCurrentProcessId()
    {
#ifdef _WIN32
    return (uint64_t) ::GetCurrentProcessId();
#else
    return (uint64_t) getpid();
#endif
    }
