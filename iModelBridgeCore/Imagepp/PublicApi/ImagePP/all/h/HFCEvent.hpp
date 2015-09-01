//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCEvent.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCEvent
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

//-----------------------------------------------------------------------------
// Internal stuff.  Since the key in HFCEvent is allocated and initialised
// at first usage, we must prevent double initialisation that may occur
// if two concurrent operations are made.  To achieve this we use an internal
// static key.  it is not a HFCExclusiveKey to avoid the dependency.
//-----------------------------------------------------------------------------
#ifdef _WIN32
inline HFCEvent::InternalKey::InternalKey()
    {
    InitializeCriticalSection(&m_Key);
    }

inline HFCEvent::InternalKey::~InternalKey()
    {
    DeleteCriticalSection(&m_Key);
    }

inline void HFCEvent::InternalKey::ClaimKey()
    {
    EnterCriticalSection(&m_Key);
    }

inline void HFCEvent::InternalKey::ReleaseKey()
    {
    LeaveCriticalSection(&m_Key);
    }
#endif


/**----------------------------------------------------------------------------
 The state of a manual-reset event object remains signaled until it is
 set explicitly to the not signaled state by the Reset method. Any number
 of waiting threads, or threads that subsequently begin wait operations
 for the specified event object by calling one of the wait methods, can
 be released while the object's state is signaled.

 The state of an auto-reset event object remains signaled until a single
 waiting thread is released, at which time the system automatically sets
 the state to not signaled. If no threads are waiting, the event object's
 state remains signaled.

 @see WaitUntilSignaled
 @see Reset
 @see Pulse
-----------------------------------------------------------------------------*/
inline void HFCEvent::Signal()
    {
#ifdef _WIN32
    HPRECONDITION(GetHandle() != 0);

    SetEvent(GetHandle());
#endif
    }


/**----------------------------------------------------------------------------
 Resets the state of the event, which becomes not signaled.

 The state of an event object remains not signaled until it is explicitly
 set to signaled by the Signal or Pulse function. This not signaled state
 blocks the execution of any threads that have specified the event object
 in a call to one of the wait.

 The Reset method is used primarily for manual-reset event objects, which
 must be set explicitly to the not signaled state. Auto-reset event
 objects automatically change from signaled to not signaled after a
 single waiting thread is released.

 @see Signal
 @see Pulse
-----------------------------------------------------------------------------*/
inline void HFCEvent::Reset()
    {
#ifdef _WIN32
    HPRECONDITION(GetHandle() != 0);

    ResetEvent(GetHandle());
#endif
    }


/**----------------------------------------------------------------------------
 Briefly changes the state of the event to signaled, then changes it to
 not signaled in a single operation after releasing the appropriate
 number of waiting threads.

 For a manual-reset event object, all waiting threads that can be
 released immediately are released. The function then resets the event
 object's state to not signaled and returns.

 For an auto-reset event object, the function resets the state to not
 signaled and returns after releasing a single waiting thread, even if
 multiple threads are waiting.

 If no threads are waiting, or if no thread can be released immediately,
 Pulse simply sets the event object's state to nonsignaled and
 returns.

 Note that for a thread using the multiple-object wait functions to wait
 for all specified objects to be signaled, Pulse can set the event
 object's state to signaled and reset it to nonsignaled without causing
 the wait function to return. This happens if not all of the specified
 objects are simultaneously signaled.

 @see WaitUntilSignaled
 @see Reset
 @see Signal
-----------------------------------------------------------------------------*/
inline void HFCEvent::Pulse()
    {
#ifdef _WIN32
    HPRECONDITION(GetHandle() != 0);

    PulseEvent(GetHandle());
#endif
    }



END_IMAGEPP_NAMESPACE