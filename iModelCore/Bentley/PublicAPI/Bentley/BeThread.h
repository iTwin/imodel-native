/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeThread.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "BeCriticalSection.h"

#include "BeAtomic.h"

BEGIN_BENTLEY_NAMESPACE

struct BeConditionVariable;

//__PUBLISH_SECTION_END__

//=======================================================================================
//! A helper class that ties ownership of a BeCriticalSection object to the scope of a variable.
//  @bsiclass 
//=======================================================================================
struct BeSystemCriticalSectionHolder
    {
    //! Enters the system critical section
    BeSystemCriticalSectionHolder () {BeCriticalSection::GetSystemCriticalSection().Enter ();}

    //! Leaves the system critical section
    ~BeSystemCriticalSectionHolder () {BeCriticalSection::GetSystemCriticalSection().Leave ();}
    };

//! Use this macro to generate a function that safely creates a singleton instance of a critical section.
//! You can use this approach to avoid creating a critical section object unless or until you actually use it.
//! Example:
//! \code
//! SINGLETON_CRITICAL_SECTION(MyCS)
//! static bool s_value;
//! static void foo()
//!   {
//!   MyCS().Enter();
//!   s_value = true;
//!   MyCS().Leave();
//!   }
//! \endcode
#define SINGLETON_CRITICAL_SECTION(CSNAME)\
static BeCriticalSection& CSNAME ()               \
    {                                             \
    static BeCriticalSection* s_##CSNAME;         \
    if (NULL == s_##CSNAME)                       \
        {                                         \
        BeSystemCriticalSectionHolder sysCS;      \
        if (NULL == s_##CSNAME)                   \
            {                                     \
            s_##CSNAME = new BeCriticalSection ();\
            }                                     \
        }                                         \
    return *s_##CSNAME;                           \
    }
//__PUBLISH_SECTION_START__

//=======================================================================================
//! Provides implementation of predicate for a condition variable
//  @bsiclass 
//=======================================================================================
struct IConditionVariablePredicate
    {
    //! WaitOnCondition calls TestCondition with the critical section locked. WaitOnCondition
    //! returns to its caller if TestCondition returns true or it gets a timeout.
    virtual bool _TestCondition (BeConditionVariable &cv) = 0;
    };

//=======================================================================================
//! A condition variable object.
//  @bsiclass 
//=======================================================================================
struct  BeConditionVariable
    {
private:
    uint8_t                 m_conditionVariable[64];
    BeCriticalSection*      m_csect;
    bool                    m_ownCSect;
    bool                    m_isValid;

    void InfiniteWait();
    void RelativeWait(bool&timedOut, uint32_t timeoutMillis);

public:
    static const uint32_t   Infinite        = 0xFFFFFFFF;

    //! @private
    void* GetNativeConditionVariable ()  { return m_conditionVariable; }

    //! Constructs the condition variable object
    //! @param[in]      csect If csect is not NULL the constructor associates it with the condition variable.
    //!                 If csect is NULL the constructor creates a new critical section and associates it with
    //!                 condition variable.  If the constructor creates a new critical section then the
    //!                 destructor deletes it.
    BENTLEYDLL_EXPORT BeConditionVariable (BeCriticalSection* csect=NULL);
    BENTLEYDLL_EXPORT ~BeConditionVariable ();
    BeCriticalSection&  GetCriticalSection () const { return *m_csect; }
    //! Enters the critical section, calls the predicate, and then waits on the condition if
    //! the predicate returns false.
    //! @param[in]      predicate   Provides an implementation of _TestCondition used to determine when to return.
    //!                             If NULL then WaitOnCondition returns the first time the condition is satisfied or a
    //!                             timeout occurs.
    //! @param[in]      timeoutMillis Time out period in milliseconds. Specify BeConditionVariable::Infinite to disable time out.
    //! @return         the value that predicate->_TestCondition returns or true if predicate is NULL.
    //! @remarks        WaitOnCondition first enters the critical section.  If predicate is non-NULL it next uses predicate to invoke
    //!                 _TestCondition and returns true if the condition is satisfied. Of course, returning also releases the critical
    //!                 section.  If the condition was not satisfied, it next uses the OS's primitive to wait for the critical section.
    //!                 The primitive automatically leaves the critical section before waiting and re-enters it before returning.  The
    //!                 OS primitive may return due to a time out or due to a wake call.  In case of wake call, WaitOnCondition returns
    //!                 true only if the condition is satisfied. In case of timeout, WaitOnCondition returns the value of _TestCondition.
    //!                 Note that the initial test waits if predicate is NULL, but the subsequent tests return if predicate is NULL.
    BENTLEYDLL_EXPORT bool WaitOnCondition (IConditionVariablePredicate* predicate, uint32_t timeoutMillis);
    //! Like WaitOnCondition, but it does not enter the critical section at the start of the method or leave it at the end.
    //! Use this if the caller has to enter the critical section prior to calling WaitOnCondion.
    BENTLEYDLL_EXPORT bool ProtectedWaitOnCondition (IConditionVariablePredicate* predicate, uint32_t timeoutMillis);
    //! Wakes at least one thread that is waiting on the condition variable.
    //! @param[in]      wakeAll If true Wake wakes all thread waiting on the condition variable. Otherwise, it just wakes one.
    BENTLEYDLL_EXPORT void Wake (bool wakeAll);
    //! False if creation failed or the BeCriticalSection is not valid.
    BENTLEYDLL_EXPORT bool GetIsValid () const;
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
    BENTLEYDLL_EXPORT static void   SetThreadStartHandler (T_ThreadStartHandler);
#endif
//__PUBLISH_SECTION_START__

    //! Set the name for the current thread. Useful for debugging only, not guaranteed to do anything
    //! @param[in] newName Name to call thread.
    BENTLEYDLL_EXPORT static void SetCurrentThreadName(Utf8CP newName);

    //! Start a new thread.
    //!@param[in] stackSize the number of bytes for the newly created thread's stack
    //!@param[in] startAddr the function to call at thread start. Thread exits when this function returns.
    //!@param[in] arg Argument to startAddr
    BENTLEYDLL_EXPORT static uintptr_t StartNewThread (int stackSize, T_ThreadStart startAddr, void* arg);

    //! Suspend the current thread for a specified amount of time
    //! @param[in] millis   Duration of sleep in milliseconds
    BENTLEYDLL_EXPORT static void BeSleep (uint32_t millis);

    //! Get the identifier of the current thread
    BENTLEYDLL_EXPORT static intptr_t GetCurrentThreadId();
    };

END_BENTLEY_NAMESPACE
