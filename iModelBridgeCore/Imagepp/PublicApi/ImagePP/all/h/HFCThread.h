//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCThread.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCThread
//-----------------------------------------------------------------------------


//:> IMPORTANT NOTE:  Actual implementation works only in Win95 and WinNT


#pragma once

#include "HFCSynchro.h"
#include "HFCEvent.h"
#include "HFCSemaphore.h"
#include "HFCInterlockedValue.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>

BEGIN_IMAGEPP_NAMESPACE
/**

    This class is used to create threads.  A thread is a single path of
    execution inside a given process.  All threads of a specific process use
    the same code and data areas in memory, they may only have different
    stacks and execution parameters (i.e. instruction pointer). A process
    always has at least one thread, the "main thread", from which other
    threads may be created: they will be executed in parallel.

    Each object of this class is a single running thread. Its execution can
    be controlled from the inside (by itself) or from the outside (by
    another thread that knows it, usually its creator), by calling methods
    designed for this purpose.

    All threads also have a level of priority that defines how much CPU time
    is made available to them relative to other threads.  The are 6 levels
    of priority supported: lowest, low, normal, high, highest and time
    critical.  The priority can be changed at any time: using either the
    SetPriority or the increase and decrease methods.  Note that the time
    critical priority can only be set with the SetPriority methods.  Also,
    the highest priority level should be used with care, as it affects
    seriously the execution speed of other threads; it is usually set for
    short times.  The time critical priority should only be used by those
    who know what they are doing with a system.  This priority uses all of
    the CPU and no other thread can be executed including the system thread.

    Communication between threads can be made through the data queues
    associated with them, which are defined by this class.  See the Queue
    method for details.

    The HFCThread is derived from HFCSynchro so the wait methods can be used
    on it.  A thread is "not signaled" while it is executing.  When the
    execution is finished, (the k{Go} method is over), the thread object
    becomes "signaled".  This is the best way to check if a thread has ended
    before starting a task that depends on this.

    The code running in a thread is mainly defined in methods of the thread
    class, starting at the @k{Go} method, a virtual method that is called to
    start it (usually after its creation).  For this reason, there will be a
    class that inherits from HFCThread that will be defined for each
    situation where a thread is required.  That class will define a
    constructor, a @k{Go} implementation and any method or data member useful
    for that case.

    Other methods can be overridden, these are the thread control methods:
    they may be overridden to "intercept" some requests to the thread in
    order to perform any particular action.

    The proper way to implement the @k{Go} method of a derived thread is to
    perform a loop that check if the thread is allowed to continue running.
    The @k{CanRun} method is defined for this purpose.  To stop a running
    thread, the @k{StopThread} method can be used.  This method sends a signal
    that changes the Boolean value returned by @k{CanRun}.  It is the caller's
    responsibility to wait for the thread to end using the @k{WaitUntilSignaled}
    method.  The @k{m_StopEvent} can also be used if the thread is a sleeper
    thread that depends on 2 or more events to be triggered.

    @h3{Example 1}

    @code
    |  void MyThread::Go()
    |  {
    |      // initialize thread
    |      ...
    |
    |      while (CanRun())
    |      {
    |          // perform thread code
    |          ...
    |      }
    |
    |      // Clean up thread
    |      ...
    |  }
    @end

    @h3{Example 2}

    @code
    |  void MyThread::Go()
    |  {
    |      // initialize thread
    |      HFCSynchroContainer Synchros;
    |      Synchros.AddSynchro(&m_StopEvent);  // defines in protected members
    |      Synchros.AddSynchro(&m_MyEvent);
    |
    |      // sleep until the stop event or my event is triggered.  If the
    |      // stop event is triggered, stop the loop and end the thread
    |      while (HFCSynchro::WaitForMultipleObjects(Synchros, false) == 1)
    |      {
    |         // perform thread code
    |         ...
    |      }
    |
    |      // Clean up thread
    |      ...
    |  }
    @end

    @see HFCSynchro

*/


class HNOVTABLEINIT HFCThread : public HFCSynchro
    {
public:

    //:> Types

    // Priority values for a thread
    enum HFCThreadPriority
        {
        HFCTHREAD_PRIORITY_LOWEST           = 0,
        HFCTHREAD_PRIORITY_LOW              = 1,
        HFCTHREAD_PRIORITY_NORMAL           = 2,
        HFCTHREAD_PRIORITY_HIGH             = 3,
        HFCTHREAD_PRIORITY_HIGHEST          = 4,
        HFCTHREAD_PRIORITY_TIME_CRITICAL    = 5
        };


    //:> Construction/destruction

    IMAGEPP_EXPORT                HFCThread(bool           pi_AutoDelete = false,
                                    const  string&  pi_rThreadName = "");
    IMAGEPP_EXPORT virtual         ~HFCThread() = 0;

    //:> Thread Control

    //:> Changes the priority settings of the thread
    IMAGEPP_EXPORT virtual void        SetPriority     (HFCThreadPriority pi_Priority);
    IMAGEPP_EXPORT virtual void        IncreasePriority();
    IMAGEPP_EXPORT virtual void        DecreasePriority();
    IMAGEPP_EXPORT virtual HFCThreadPriority
    GetPriority() const;

    //:> Suspends/resumes the thread
    IMAGEPP_EXPORT virtual uint32_t    Suspend();
    IMAGEPP_EXPORT virtual uint32_t    Resume();
    IMAGEPP_EXPORT bool               IsSuspended() const;

    //:> Sleep in the calling thread.  Default value relinquishes
    //:> the context to another thread.
    static void         Sleep(uint32_t pi_Duration = 0);


    //:> Inter-thread communication

    void                Queue       (void* pi_pData);
    void*               DequeueWait ();
    void*               DequeueWait (uint32_t pi_TimeOut);
    void                WaitForQueue();
    void                WaitForQueue(uint32_t pi_TimeOut);
    bool               IsQueueEmpty() const;


    //:> Thread execution

    //:> These methods starts & stops the execution of the thread
    IMAGEPP_EXPORT virtual bool       StartThread(size_t pi_StackSize = 0);
    IMAGEPP_EXPORT virtual void        StopThread ();

    //:> Main thread algorithm (MUST be overridden)
    virtual void        Go() = 0;

    //:> Indicates if the thread will destroy itself when it ends.
    bool               AutoDeletes() const;

    static uint32_t         s_NbActiveThread;
    static HFCExclusiveKey  s_NbActiveThreadKey;

protected:

    virtual HFCHandle   GetHandle() const;

    virtual bool       CanRun() const;

    // this event is signaled when the thread must stop.
    // Protected so that descendant may use the event in
    // a multiple wait if CanRun() is not enough.
    HFCEvent            m_StopEvent;

    // The handle to the OS thread object
    HFCHandle           m_Handle;


private:

#ifdef _WIN32
    // This method is used by the implementation of thread in Win32.
    static uint32_t     StartThreadProc(HFCThread* pi_pThread);
#endif


    // Attributes

    // The all-mighty exclusive key
    mutable HFCExclusiveKey
    m_Key;

    // The thread priority
    HFCThreadPriority       m_Priority;

    // Message Queue for the thread
    typedef deque<void*>
    MessageQueue;
    MessageQueue            m_Queue;
    HFCSemaphore            m_QueueSemaphore;

    // Indicates if the thread is suspended
    bool                   m_Suspended;

    // Indicates if the thread will destroy itself once terminated.
    bool                   m_AutoDelete;

    unsigned long          m_ThreadID;
    HArrayAutoPtr<char>    m_pThreadName;

    // Disabled Methods

    HFCThread(const HFCThread&);
    HFCThread&          operator=(const HFCThread&);
    };

END_IMAGEPP_NAMESPACE
#include "HFCThread.hpp"

