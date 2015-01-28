/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeThread.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "BeAtomic.h"

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! A synchronization primitive that can be used to protect shared data from being simultaneously accessed by multiple threads.
//! BeMutex offers exclusive, recursive ownership semantics.
//! @see std::recursive_mutex
//  @bsiclass 
//=======================================================================================
struct BeMutex
    {
private:
    double m_osMutex[1];

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
struct BeMutexHolder
    {
private:
    BeMutex* m_mutex;
    bool     m_owns;

public:
    enum class Lock : bool {No=false, Yes=true};

    //! Associate this BeMutexHolder with a BeMutex, optionally locking it.
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
    //! WaitOnCondition calls TestCondition with the critical section locked. WaitOnCondition
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
struct  BeConditionVariable
    {
private:
    double  m_osCV[2];
    mutable BeMutex m_mutex;

    void InfiniteWait(BeMutexHolder& holder);
    bool RelativeWait(BeMutexHolder& holder, uint32_t timeoutMillis);

public:
    static const uint32_t Infinite = 0xFFFFFFFF;

    BENTLEYDLL_EXPORT BeConditionVariable();
    BENTLEYDLL_EXPORT ~BeConditionVariable();

    // Get the mutex
    BeMutex& GetMutex() const {return m_mutex;}

    //! Like WaitOnCondition, but it does not enter the mutex at the start of the method or leave it at the end.
    //! Use this if the caller has already locked the mutex
    BENTLEYDLL_EXPORT bool ProtectedWaitOnCondition(BeMutexHolder&, IConditionVariablePredicate* predicate, uint32_t timeoutMillis);

    //! Enters the critical section, calls the predicate, and then waits on the condition if
    //! the predicate returns false.
    //! @param[in]      predicate   Provides an implementation of _TestCondition used to determine when to return.
    //!                             If NULL then WaitOnCondition returns the first time the condition is satisfied or a
    //!                             timeout occurs.
    //! @param[in]      timeoutMillis Time out period in milliseconds. Specify BeConditionVariable::Infinite to disable time out.
    //! @return         the value that predicate->_TestCondition returns or true if predicate is NULL.
    //! @remarks        WaitOnCondition first enters the mutex. If predicate is non-NULL it next uses predicate to invoke
    //!                 _TestCondition and returns true if the condition is satisfied. Of course, returning also releases the mutex.
    //!                 If the condition was not satisfied, it next waits for the mutex.
    //!                 The primitive automatically leaves the mutex waiting and re-enters it before returning. The
    //!                 return may be due to a time out or wake call. In case of wake call, WaitOnCondition returns
    //!                 true only if the condition is satisfied. In case of timeout, WaitOnCondition returns the value of _TestCondition.
    //!                 Note that the initial test waits if predicate is NULL, but the subsequent tests return if predicate is NULL.
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

#if defined (__APPLE__) || defined (ANDROID) || defined (__linux)
    typedef void* (*T_ThreadStart)(void*);
    #define THREAD_MAIN_DECL static void*
#elif defined (_WIN32) // Windows && WinRT
    typedef unsigned (__stdcall *T_ThreadStart)(void*);
    #define THREAD_MAIN_DECL static unsigned __stdcall 
#endif

#if defined (BENTLEY_WINRT)
typedef unsigned (__stdcall *T_ThreadStartHandler)(T_ThreadStart startAddr, void* arg);
#endif

//=======================================================================================
//! Utilities for dealing with threads
//  @bsiclass 
//=======================================================================================
struct  BeThreadUtilities
    {
//__PUBLISH_SECTION_END__

#if defined (BENTLEY_WINRT)
    BENTLEYDLL_EXPORT static void SetThreadStartHandler (T_ThreadStartHandler);
#endif
//__PUBLISH_SECTION_START__

    //! Set the name for the current thread. Useful for debugging only, not guaranteed to do anything
    //! @param[in] newName Name to call thread.
    BENTLEYDLL_EXPORT static void SetCurrentThreadName(Utf8CP newName);

    //! Start a new thread.
    //!@param[in] stackSize the number of bytes for the newly created thread's stack
    //!@param[in] startAddr the function to call at thread start. Thread exits when this function returns.
    //!@param[in] arg Argument to startAddr
    BENTLEYDLL_EXPORT static void StartNewThread (int stackSize, T_ThreadStart startAddr, void* arg);

    //! Suspend the current thread for a specified amount of time
    //! @param[in] millis   Duration of sleep in milliseconds
    BENTLEYDLL_EXPORT static void BeSleep (uint32_t millis);

    //! Get the identifier of the current thread
    BENTLEYDLL_EXPORT static intptr_t GetCurrentThreadId();
    };

//__PUBLISH_SECTION_END__
//=======================================================================================
//! Hold a lock on the system BeMutex
//  @bsiclass 
//=======================================================================================
struct BeSystemMutexHolder : BeMutexHolder
    {
    //! Enters the system critical section
    BeSystemMutexHolder() : BeMutexHolder(GetSystemMutex()) {}

    BENTLEYDLL_EXPORT static void StartupInitializeSystemMutex();

    //! Get the Bentley system CS. This can be used to bootstrap other CriticalSections.
    //! @remarks Program must call StartupInitializeSystemCriticalSection before calling this function.
    BENTLEYDLL_EXPORT static BeMutex& GetSystemMutex();
    };

//__PUBLISH_SECTION_START__


END_BENTLEY_NAMESPACE
