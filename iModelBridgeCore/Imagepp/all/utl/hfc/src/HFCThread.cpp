//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCThread.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCThread
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCThread.h>

uint32_t         HFCThread::s_NbActiveThread = 0;
HFCExclusiveKey  HFCThread::s_NbActiveThreadKey;

/**----------------------------------------------------------------------------
 Constructor and destructor for this class.  After construction, the
 thread is not yet running.  The @k{StartThread} method must be called to do
 start the execution of the thread (via the @k{Go} method).

 Copy constructor and assignment operators are disabled for this class.

 @param pi_AutoDelete Tells if this object should be automatically deleted
                      after Go returns.  If true, the thread will be deleted
                      after that method terminates.

 @see StartThread
 @see EndThread
 @see Go
-----------------------------------------------------------------------------*/

HFCThread::HFCThread(bool          pi_AutoDelete,
                     const string&  pi_rThreadName)
    : HFCSynchro      (),
      m_StopEvent     (true, false),
      m_QueueSemaphore(0, LONG_MAX)
    {
    // Initialize
    m_Suspended  = true;
    m_AutoDelete = pi_AutoDelete;
    m_Handle     = NULL;
    m_Priority   = HFCTHREAD_PRIORITY_NORMAL;

    HFCMonitor(s_NbActiveThreadKey);
    s_NbActiveThread++;

#ifdef __HMR_DEBUG
    if (!pi_rThreadName.empty())
        {
        m_pThreadName = new char[pi_rThreadName.size() + 1];
        strcpy(m_pThreadName, pi_rThreadName.c_str());
        }
#endif
    }


/**----------------------------------------------------------------------------
 The destructor for this class.  The destructor closes the handle to the
 actual thread object killing it ungracefully.  It is the responsibility
 of derived classes to stop the thread before the HFCThread destructor
 is invoked.

 @see EndThread
-----------------------------------------------------------------------------*/
HFCThread::~HFCThread()
    {
    // close the handle
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    ::CloseHandle(m_Handle);
    m_Handle = NULL;
#endif
    HFCMonitor(s_NbActiveThreadKey);
    s_NbActiveThread--;
    }



/**----------------------------------------------------------------------------
 Starts the execution of the thread.  The resources of a running thread
 are allocated and the method Go is called.  The pi_StackSize parameter
 indicates the stack size for the new thread.

 Starting a stopped thread is not the same than suspending/resuming :
 this restarts a new thread from scratch, the method Go being called
 again.

 @param pi_StackSize Size of the stack (in bytes) that will be allocated for
                     this new thread.  Specifying zero for this parameter will
                     set the "default" size.  The parameter that was specified
                     in the constructor is ignored.

 @see Go
 @see EndThread
-----------------------------------------------------------------------------*/
bool HFCThread::StartThread(size_t pi_StackSize)
    {
    HPRECONDITION(pi_StackSize >= 0);

    //
    // NOTE: A stack size of 0 indicates that the new thread will have the
    // stack size of its creator.
    //

#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    HPRECONDITION(m_Handle == NULL || !CanRun());

    if (m_Handle != NULL)
        ::CloseHandle(m_Handle);

    // Start the thread.  Give it the address of the static
    // StartThreadProc, which the entry point of the new thread.
    // It will call the Go method.
    m_Handle = CreateThread(NULL,                                       // Security Descriptor
                            pi_StackSize,                               // Stack Size
                            (LPTHREAD_START_ROUTINE)StartThreadProc,    // The thread function
                            this,                                       // for the proc param
                            0,                                          // Creation Flags
                            &m_ThreadID);                               // Thread Desc.


    // Set the priority (in case it has changed before the execution
    SetPriority(m_Priority);
#endif

    return (m_Handle != NULL);
    }


/**----------------------------------------------------------------------------
 This method can be used to stop the execution of a thread without
 deleting the object that represents it.  When the method returns, the
 thread @u{may} not be already stopped.  A wait on the thread must be
 performed to receive the signaled that the thread has ended.

 @see Go
 @see StartThread
-----------------------------------------------------------------------------*/
void HFCThread::StopThread()
    {
    m_StopEvent.Signal();
    }


/**----------------------------------------------------------------------------
 Sets the priority of this thread to a specified priority.

 @param pi_Priority The new priority to set to the thread.  Must be one of
                    the priority enum values.

 @see IncreasePriority
 @see DecreasePriority
 @see GetPriority
-----------------------------------------------------------------------------*/
void HFCThread::SetPriority(HFCThreadPriority pi_Priority)
    {
    HPRECONDITION(pi_Priority >= HFCTHREAD_PRIORITY_LOWEST);
    HPRECONDITION(pi_Priority <= HFCTHREAD_PRIORITY_TIME_CRITICAL);
    HFCMonitor Monitor(m_Key);

    // Priority must really chang
    if (m_Priority != pi_Priority)
        {
        m_Priority = pi_Priority;
        Monitor.ReleaseKey();

        // Change the thread priority if the thread is running
#if defined (ANDROID) || defined (__APPLE__)
        //DM-Android
#elif defined (_WIN32)
        if (m_Handle != NULL)
            {
            int32_t Priority;
            switch(pi_Priority)
                {
                case HFCTHREAD_PRIORITY_LOW:
                    Priority = THREAD_PRIORITY_BELOW_NORMAL;
                    break;
                case HFCTHREAD_PRIORITY_NORMAL:
                    Priority = THREAD_PRIORITY_NORMAL;
                    break;
                case HFCTHREAD_PRIORITY_HIGH:
                    Priority = THREAD_PRIORITY_ABOVE_NORMAL;
                    break;
                case HFCTHREAD_PRIORITY_HIGHEST:
                    Priority = THREAD_PRIORITY_HIGHEST;
                    break;
                case HFCTHREAD_PRIORITY_TIME_CRITICAL:
                    Priority = THREAD_PRIORITY_TIME_CRITICAL;
                    break;

                case HFCTHREAD_PRIORITY_LOWEST:
                default:
                    HASSERT(HFCTHREAD_PRIORITY_LOWEST == pi_Priority);
                    Priority = THREAD_PRIORITY_LOWEST;
                    break;
                }

            // Set the priority
            SetThreadPriority(m_Handle, Priority);
            }
#endif
        }
    }


/**----------------------------------------------------------------------------
 Sets the priority of this thread to "high".  From this time on, the
 thread will have much more CPU time to execute itself.  Use it with
 care, as giving high priority to a thread seriously affect other
 threads.

 The priority will not be changed to time critical with this. The time
 critical priority must be set by hand.

 @see DecreasePriority
 @see SetPriority
 @see GetPriority
-----------------------------------------------------------------------------*/
void HFCThread::IncreasePriority()
    {
    HFCThreadPriority Priority;

        {
        HFCMonitor Monitor(m_Key);

        // Change the priority if it has not reached the highest yet.
        if (m_Priority < HFCTHREAD_PRIORITY_HIGHEST)
            {
            int32_t* pPriority = (int32_t*)&m_Priority;
            (*pPriority)++;
            }

        Priority = m_Priority;
        }

    // change the priority
    SetPriority(Priority);
    }


/**----------------------------------------------------------------------------
 Sets the priority of this thread to "low".  From this time on, the
 thread will have much less CPU time to execute itself.  This may be
 desirable when creating threads that execute tasks in background while
 not affecting the main thread too much.

 @see SetPriority
 @see DecreasePriority
 @see GetPriority
-----------------------------------------------------------------------------*/
void HFCThread::DecreasePriority()
    {
    HFCThreadPriority Priority;

        {
        HFCMonitor Monitor(m_Key);

        // Change the priority if it has not reached the lowest yet.
        if (m_Priority > HFCTHREAD_PRIORITY_LOWEST)
            {
            int32_t* pPriority = (int32_t*)&m_Priority;
            (*pPriority)--;
            }

        Priority = m_Priority;
        }

    // change the priority
    SetPriority(Priority);
    }


/**----------------------------------------------------------------------------
  Suspends the execution of this thread.  A suspended thread is stopped
  but not killed:  it doesn't consume CPU time but can be restarted at
  any time by calling Resume, making it continue what it was doing.

  @see Resume
  @see IsSuspended
-----------------------------------------------------------------------------*/
uint32_t HFCThread::Suspend()
    {
#if defined (ANDROID) || defined (__APPLE__)
        //DM-Android
        return 0;
#elif defined (_WIN32)
        {
        HFCMonitor Monitor(m_Key);
        m_Suspended = true;
        }

    DWORD Count = SuspendThread(m_Handle);
    HPOSTCONDITION(Count != 0xFFFFFFFF);
    return (uint32_t) Count;
#endif
    }


/**----------------------------------------------------------------------------
 Resumes the execution of this thread, after being suspended by calling
 @k{Suspend}.  Obviously this method cannot be called from inside the
 thread itself.

 @see Suspend
 @see IsSuspended
-----------------------------------------------------------------------------*/
uint32_t HFCThread::Resume()
    {
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
    return 0;
#elif defined (_WIN32)

    DWORD Count;
    Count = ResumeThread(m_Handle);

    HPRECONDITION(Count != 0xFFFFFFFF);

    if (Count == 1)
        {
        HFCMonitor Monitor(m_Key);
        m_Suspended = false;
        }

    return (uint32_t)Count;
#endif
    }


/**----------------------------------------------------------------------------
 Adds a new object to a thread's queue.  It is usually called by the
 creator of the thread to indicate a new task to perform.  The thread
 can wait for new data by using the @k{DequeueWait} method.

 Each thread owns a queue of data pointers into which other threads may
 place data that may be required by this thread, implementing an
 asynchronous communication system between threads.  The order, type and
 number of data objects depends only the protocol determined by derived
 classes.

 Note that the pointer to the data is given to the thread, so it is
 important the caller does not destroy the object or gives a pointer to
 a local object.

 @param pi_pData A pointer to some data to place in the pointer queue of
                 this thread.

 @see DequeueWait
 @see WaitForQueue
 @see IsQueueEmpty
-----------------------------------------------------------------------------*/
void HFCThread::Queue(void* pi_pData)
    {
    // add the object to the queue
        {
        HFCMonitor Monitor(m_Key);
        m_Queue.push_back(pi_pData);
        }

    // Signal the addition
    m_QueueSemaphore.Signal();
    }


/**----------------------------------------------------------------------------
 Removes data from the data queue, waiting for data if it is empty.  It
 returns if data appears in the queue.

 This function should be called from within the thread, to allow it to
 receive data from another thread in an asynchronous way.

 Each thread owns a queue of data pointers into which other threads may
 place pointers to data that may be required by this thread, by using
 the @k{Queue} method.  The order, type and number of data objects
 depends only the protocol determined by derived classes.

 The retrieved pointer is removed from the queue.  If the queue is
 empty, the current thread is suspended and will be resumed as soon as
 data is available.

 @return A pointer to data, cast as a void pointer.

 @see WaitForQueue
 @see IsQueueEmpty
-----------------------------------------------------------------------------*/
void* HFCThread::DequeueWait()
    {
    void* pResult = 0;

    // wait for the data to arrive
    m_QueueSemaphore.WaitUntilSignaled();

    // extract the data from the queue
        {
        HFCMonitor Monitor(m_Key);
        pResult = m_Queue.front();
        m_Queue.pop_front();
        }

    return pResult;
    }


/**----------------------------------------------------------------------------
 Removes data from the data queue, waiting for data if it is empty.  It
 returns if data appears in the queue, or if the specified delay has
 elapsed.

 This function should be called from within the thread, to allow it to
 receive data from another thread in an asynchronous way.

 Each thread owns a queue of data pointers into which other threads may
 place pointers to data that may be required by this thread, by using
 the @k{Queue} method.  The order, type and number of data objects
 depends only the protocol determined by derived classes.

 The retrieved pointer is removed from the queue.  If the queue is
 empty, the current thread is suspended and will be resumed as soon as
 data is available.

 @param pi_Timeout Maximum delay, in milliseconds, to wait for data to
                   arrive in the queue.

 @return A pointer to data, cast as a void pointer.

 @see WaitForQueue
 @see IsQueueEmpty
-----------------------------------------------------------------------------*/
void* HFCThread::DequeueWait(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);
    void* pResult = 0;

    // Wait for the data to arrive or for the time out to expire, whichever is first
    if (m_QueueSemaphore.WaitUntilSignaled(pi_TimeOut))
        {
        // Extract the data from the queue
        HFCMonitor Monitor(m_Key);
        pResult = m_Queue.front();
        m_Queue.pop_front();
        }

    return pResult;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ghislain.Tardif 05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static void s_SetThreadName(const char* name)
    {
#if defined(_WIN32) && defined(__HMR_DEBUG)

    // From MSDN: "How to: Set a Thread Name in Native Code"
    // http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx

    const DWORD MS_VC_EXCEPTION=0x406D1388;

    #pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO
        {
        DWORD dwType;        // Must be 0x1000.
        LPCSTR szName;       // Pointer to name (in user addr space).
        DWORD dwThreadID;    // Thread ID (-1=caller thread).
        DWORD dwFlags;       // Reserved for future use, must be zero.
        } THREADNAME_INFO;
    #pragma pack(pop)

    THREADNAME_INFO ThreadInfo;
    ThreadInfo.dwType = 0x1000;
    ThreadInfo.szName = name;
    ThreadInfo.dwThreadID = -1;
    ThreadInfo.dwFlags = 0;

    __try // Do NOT change for try/catch(..) it crashes when the program is started outside of the debugger.
        {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(ThreadInfo)/sizeof(ULONG_PTR), (const ULONG_PTR*)&ThreadInfo);
        }
    // Do NOT change for try/catch(..) it crashes when the program is started outside of the debugger.
    __except(EXCEPTION_CONTINUE_EXECUTION)
        {
        }
#endif
    }

/**----------------------------------------------------------------------------
 Static function that is used by Win32::StartThread
-----------------------------------------------------------------------------*/
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
uint32_t HFCThread::StartThreadProc(HFCThread* pi_pThread)
    {
        {
        HFCMonitor Monitor(pi_pThread->m_Key);
        pi_pThread->m_Suspended = false;
        }

    if(pi_pThread->m_pThreadName != NULL)
        s_SetThreadName(pi_pThread->m_pThreadName);

    // Execute the thread
    pi_pThread->Go();

        {
        HFCMonitor Monitor(pi_pThread->m_Key);
        pi_pThread->m_Suspended = true;
        }

    // if it is flagged as auto-deleting, delete it now
    if (pi_pThread->AutoDeletes())
        delete pi_pThread;

    return (0);
    }
#endif
