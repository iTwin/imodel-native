/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeThread.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "BeAtomic.h"
#include "RefCounted.h"
#include "WString.h"

#if defined (_WIN32)
    //=======================================================================================
    //
    // All of this miserable crap is here only because the stupid C++/CLI compiler doesn't support std::threads.
    // Until we can get rid of the whole Windows "managed" abomination, we're stuck with this nonsense.
    //
    // BEMUTEX_DATA_ARRAY_LENGTH = sizeof (std::recursive_mutex) / sizeof (void*)
    // BECONDITIONVARIABLE_DATA_ARRAY_LENGTH = sizeof (std::condition_variable_any) / sizeof (void*)
    // Array of void* is used to force pointer type alignment
    //=======================================================================================
    #if (_MSC_VER >= 1900)
        #if defined(_M_X64)
            #define BEMUTEX_DATA_ARRAY_LENGTH (80 / sizeof(void*))
            #define BECONDITIONVARIABLE_DATA_ARRAY_LENGTH (88 / sizeof(void*))
        #else
            #define BEMUTEX_DATA_ARRAY_LENGTH (48 / sizeof(void*))
            #define BECONDITIONVARIABLE_DATA_ARRAY_LENGTH (48 / sizeof(void*))
        #endif
    #else
        #define BEMUTEX_DATA_ARRAY_LENGTH 1
        #define BECONDITIONVARIABLE_DATA_ARRAY_LENGTH 2
    #endif
#else
    //=======================================================================================
    // Sane platforms...
    //=======================================================================================
    #include <mutex>
    #include <condition_variable>
    #define BEMUTEX_DATA_ARRAY_LENGTH sizeof(std::recursive_mutex) / sizeof(void*)
    #define BECONDITIONVARIABLE_DATA_ARRAY_LENGTH sizeof(std::condition_variable_any) / sizeof(void*)
#endif

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! A synchronization primitive that can be used to protect shared data from being simultaneously accessed by multiple threads.
//! BeMutex offers exclusive, recursive ownership semantics.
//! @see std::recursive_mutex
//  @bsiclass
//=======================================================================================
struct BeMutex : NonCopyableClass
{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

private:
    void* m_osMutex[BEMUTEX_DATA_ARRAY_LENGTH];

#ifdef __clang__
#pragma clang diagnostic pop
#endif

public:
    BENTLEYDLL_EXPORT BeMutex();
    BENTLEYDLL_EXPORT ~BeMutex();

    //! lock the mutex
    BENTLEYDLL_EXPORT void lock();
    void Enter() {lock();}

    //! unlock the mutex
    BENTLEYDLL_EXPORT void unlock();
    void Leave() {unlock();}
};

//=======================================================================================
//! A BeMutex ownership wrapper.
//! @see std::unique_lock
//  @bsiclass
//=======================================================================================
struct BeMutexHolder : NonCopyableClass
{
private:
    BeMutex* m_mutex;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

    bool m_owns;

#ifdef __clang__
#pragma clang diagnostic pop
#endif

public:
    enum class Lock : bool {No=false, Yes=true};

    //! Associate this BeMutexHolder with a BeMutex, optionally locking it.
    //! @param[in] mutex the BeMutex to hold for the scope of this BeMutexHolder
    //! @param[in] lock if Yes, the lock on mutex is acquired via mutex.lock()
    BENTLEYDLL_EXPORT BeMutexHolder(BeMutex& mutex, Lock lock=Lock::Yes);

    //! Unlocks BeMutex if locked.
    BENTLEYDLL_EXPORT ~BeMutexHolder();

    //! Get the BeMutex associated with this BeMutexHolder
    BeMutex* GetMutex() {return m_mutex;}
    //! Unlock the BeMutex
    BENTLEYDLL_EXPORT void unlock();
    //! Lock the BeMutex
    BENTLEYDLL_EXPORT void lock();
    //! Determine whether the BeMutex is currently locked by this BeMutexHolder
    BENTLEYDLL_EXPORT bool owns_lock();
};

//=======================================================================================
//! Provides implementation of predicate for a BeConditionVariable
//  @bsiclass
//=======================================================================================
struct IConditionVariablePredicate
{
    //! WaitOnCondition calls TestCondition with the mutex locked. WaitOnCondition
    //! returns to its caller if TestCondition returns true or it gets a timeout.
    virtual bool _TestCondition(struct BeConditionVariable &cv) = 0;
};

//=======================================================================================
//! A synchronization primitive that can be used to block a thread, or multiple threads at the same time, until:
//! 1) a notification is received from another thread, 2) a timeout expires, or 3) a spurious wakeup occurs.
//! @see std::condition_variable_any
//! @note BeConditionVariable differs from std::condition_variable_any in that it includes a BeMutex internally, wheras
//! std::condition_variable_any requires an external mutex.
//  @bsiclass
//=======================================================================================
struct  BeConditionVariable : NonCopyableClass
{
private:
    void* m_osCV[BECONDITIONVARIABLE_DATA_ARRAY_LENGTH];
    mutable BeMutex m_mutex;

public:
    BENTLEYDLL_EXPORT void InfiniteWait(BeMutexHolder& holder);
    BENTLEYDLL_EXPORT bool RelativeWait(BeMutexHolder& holder, uint32_t timeoutMillis);

    static const uint32_t Infinite = 0xffffffff;

    BENTLEYDLL_EXPORT BeConditionVariable();
    BENTLEYDLL_EXPORT ~BeConditionVariable();

    // Get the mutex
    BeMutex& GetMutex() const {return m_mutex;}

    //! Like WaitOnCondition, but it does not enter the mutex at the start of the method or leave it at the end.
    //! Use this if the caller has already locked the mutex
    BENTLEYDLL_EXPORT bool ProtectedWaitOnCondition(BeMutexHolder&, IConditionVariablePredicate* predicate, uint32_t timeoutMillis);

    //! Enters the mutex, calls the predicate, and then waits on the condition if
    //! the predicate returns false.
    //! @param[in]      predicate   Provides an implementation of _TestCondition used to determine when to return.
    //!                             If not nullptr then WaitOnCondition returns the first time the condition is satisfied or a
    //!                             timeout occurs.
    //! @param[in]      timeoutMillis Time out period in milliseconds. Specify BeConditionVariable::Infinite to disable time out.
    //! @return         the value that predicate->_TestCondition returns or true if predicate is nullptr.
    //! @remarks        WaitOnCondition first enters the mutex. If predicate is non-nullptr it next uses predicate to invoke
    //!                 _TestCondition and returns true if the condition is satisfied. Of course, returning also releases the mutex.
    //!                 If the condition was not satisfied, it next waits for the mutex.
    //!                 The primitive automatically leaves the mutex waiting and re-enters it before returning. The
    //!                 return may be due to a time out or wake call. In case of wake call, WaitOnCondition returns
    //!                 true only if the condition is satisfied. In case of timeout, WaitOnCondition returns the value of _TestCondition.
    //!                 Note that the initial test waits if predicate is nullptr, but the subsequent tests return if predicate is nullptr.
    bool WaitOnCondition(IConditionVariablePredicate* predicate, uint32_t timeoutMillis)
        {
        BeMutexHolder holder(m_mutex);
        return ProtectedWaitOnCondition(holder, predicate, timeoutMillis);
        }

    //! Notify one thread waiting on this BeConditionVariable.
    BENTLEYDLL_EXPORT void notify_one();
    //! Notify all threads waiting on this BeConditionVariable.
    BENTLEYDLL_EXPORT void notify_all();
};

#if defined (__APPLE__) || defined (ANDROID) || defined (__linux) || defined (__EMSCRIPTEN__)
    typedef void* (*T_ThreadStart)(void*);
    #define THREAD_MAIN_IMPL void*
#elif defined (_WIN32) // Windows && WinRT
    typedef unsigned (__stdcall *T_ThreadStart)(void*);
    #define THREAD_MAIN_IMPL unsigned __stdcall
#endif

#define THREAD_MAIN_DECL static THREAD_MAIN_IMPL

#if defined (BENTLEY_WINRT)
typedef unsigned (__stdcall *T_ThreadStartHandler)(T_ThreadStart startAddr, void* arg);
#endif

//=======================================================================================
//! Utilities for dealing with threads
//  @bsiclass
//=======================================================================================
struct  BeThreadUtilities
{

#if defined (BENTLEY_WINRT)
    BENTLEYDLL_EXPORT static void SetThreadStartHandler(T_ThreadStartHandler);
#endif

    //! Set the name for the current thread. Useful for debugging only, not guaranteed to do anything
    //! @param[in] newName Name to call thread.
    BENTLEYDLL_EXPORT static void SetCurrentThreadName(Utf8CP newName);

    //! Start a new thread.
    //!@param[in] stackSize the number of bytes for the newly created thread's stack
    //!@param[in] startAddr the function to call at thread start. Thread exits when this function returns.
    //!@param[in] arg Argument to startAddr
    BENTLEYDLL_EXPORT static BentleyStatus StartNewThread(int stackSize, T_ThreadStart startAddr, void* arg);

    //! Suspend the current thread for a specified amount of time
    //! @param[in] millis   Duration of sleep in milliseconds
    BENTLEYDLL_EXPORT static void BeSleep(uint32_t millis);

    //! Get the identifier of the current thread
    BENTLEYDLL_EXPORT static intptr_t GetCurrentThreadId();

    //! see documentation for std::thread::hardware_concurrency()
    BENTLEYDLL_EXPORT static uint32_t GetHardwareConcurrency();
};


//__PUBLISH_SECTION_END__
//=======================================================================================
//! Hold a lock on the system BeMutex
//  @bsiclass
//=======================================================================================
struct BeSystemMutexHolder : BeMutexHolder
{
    //! Enters the system mutex
    BeSystemMutexHolder() : BeMutexHolder(GetSystemMutex()) {}

    BENTLEYDLL_EXPORT static void StartupInitializeSystemMutex();

    //! Get the Bentley system CS. This can be used to bootstrap other BeMutexs.
    //! @remarks Program must call StartupInitializeSystemMutex before calling this function.
    BENTLEYDLL_EXPORT static BeMutex& GetSystemMutex();
};
//__PUBLISH_SECTION_START__

END_BENTLEY_NAMESPACE
