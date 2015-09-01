//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCThread.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HFCThread
//-----------------------------------------------------------------------------

#include "HFCMonitor.h"

BEGIN_IMAGEPP_NAMESPACE

/**----------------------------------------------------------------------------
 Returns the current priority of the thread

 @return A HFCThreadPriority enum value.

 @see SetPriority
 @see DecreasePriority
 @see IncreasePriority
-----------------------------------------------------------------------------*/
inline HFCThread::HFCThreadPriority HFCThread::GetPriority() const
    {
    HFCMonitor Monitor(m_Key);

    return m_Priority;
    }

/**----------------------------------------------------------------------------
 Indicates when there is no more data available in the message queue.  The
 method does not wait for new data to be available.

 @return true if the data queue for this thread is empty.

 @see Queue
 @see DequeueWait
 @see WaitForQueue
-----------------------------------------------------------------------------*/
inline bool HFCThread::IsQueueEmpty() const
    {
    HFCMonitor Monitor(m_Key);

    return m_Queue.empty();
    }

/**----------------------------------------------------------------------------
 Suspends the execution of the current thread until data is placed in the
 queue. If data is already present in the queue, this method returns
 immediately.  The data is not retrieved from the queue.

 Each thread owns a queue of data pointers into which other threads may
 place pointers to data that may be required by this thread, by using the
 @k{Queue} method.  The order, type and number of data objects depends only
 the protocol determined by derived classes.

 @see DequeueWait
 @see Queue
 @see IsQueueEmpty
-----------------------------------------------------------------------------*/
inline void HFCThread::WaitForQueue()
    {
    m_QueueSemaphore.WaitUntilSignaled();
    }


/**----------------------------------------------------------------------------
 Suspends the execution of the current thread until data is placed in
 the queue, or if the specified delay has elapsed.  If data is already
 present in the queue, this method returns immediately.  The data is not
 retrieved from the queue.

 Each thread owns a queue of data pointers into which other threads may
 place pointers to data that may be required by this thread, by using
 the Queue method.  The order, type and number of data objects depends
 only the protocol determined by derived classes.

 @param pi_TimeOut Maximum delay, in milliseconds, to wait for data to arrive
                   in the queue.  This delay may be changed to infinite by
                   specifying the constant INFINITE.

 @see DequeueWait
 @see Queue
 @see IsQueueEmpty
-----------------------------------------------------------------------------*/
inline void HFCThread::WaitForQueue(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);

    m_QueueSemaphore.WaitUntilSignaled(pi_TimeOut);
    }


/**----------------------------------------------------------------------------
 Returns the execution state of this thread: true if it is suspended.

 @return true if this thread is suspended, false otherwise.

 @see Suspend
 @see Resume
-----------------------------------------------------------------------------*/
inline bool HFCThread::IsSuspended() const
    {
    HFCMonitor Monitor(m_Key);

    return m_Suspended;
    }


/**----------------------------------------------------------------------------
 Returns the value that was passed for the parameter @r{pi_AutoDelete} at the
 construction of this object.  That value tells if the destructor will be
 automatically called after the method @k{Go} returns.

 @return true if the destructor is called automatically after the end of the
         execution of this thread (after the method Go returns).
-----------------------------------------------------------------------------*/
inline bool HFCThread::AutoDeletes() const
    {
    HFCMonitor Monitor(m_Key);

    return m_AutoDelete;
    }

/**----------------------------------------------------------------------------
 Static method. Pauses the current thread for a specified time.  It is
 like doing Suspend/Resume automatically with a specific timing: a
 paused thread doesn't consume CPU time, it "sleeps".

 This method can be used to force preemption by specifying a time of zero.

 @param pi_Duration Duration of the pause, in milliseconds.

 @see Suspend
 @see Resume
-----------------------------------------------------------------------------*/
inline void HFCThread::Sleep(uint32_t pi_Duration)
    {
    HPRECONDITION(pi_Duration >= 0);

#ifdef _WIN32
    ::Sleep(pi_Duration);
#endif
    }

/**----------------------------------------------------------------------------
 Returns the handle to the actual OS object.
-----------------------------------------------------------------------------*/
inline HFCHandle HFCThread::GetHandle() const
    {
    return m_Handle;
    }

/**----------------------------------------------------------------------------
 Check if a thread can continue running.
-----------------------------------------------------------------------------*/
inline bool HFCThread::CanRun() const
    {
    return (!m_StopEvent.WaitUntilSignaled(0));
    }
END_IMAGEPP_NAMESPACE