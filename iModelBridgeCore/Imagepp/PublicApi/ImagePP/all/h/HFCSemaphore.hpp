//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSemaphore.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCSemaphore
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//:Ignore
//-----------------------------------------------------------------------------
// Internal stuff.  Since the key in HFCSemaphore is allocated and initialised
// at first usage, we must prevent double initialisation that may occur
// if two concurrent operations are made.  To achieve this we use an internal
// static key.  it is not a HFCExclusiveKey to avoid the dependency.
//-----------------------------------------------------------------------------
#ifdef _WIN32
inline HFCSemaphore::InternalKey::InternalKey()
    {
    InitializeCriticalSection(&m_Key);
    }

inline HFCSemaphore::InternalKey::~InternalKey()
    {
    DeleteCriticalSection(&m_Key);
    }

inline void HFCSemaphore::InternalKey::ClaimKey()
    {
    EnterCriticalSection(&m_Key);
    }

inline void HFCSemaphore::InternalKey::ReleaseKey()
    {
    LeaveCriticalSection(&m_Key);
    }
#endif
//:End Ignore

/**----------------------------------------------------------------------------
 Sets the state of this semaphore to "signaled".  Those signals are accumulated
 in the semaphore; they are removed one by one by calling @k{WaitUntilSignaled}.

 @see WaitUntilSignaled
-----------------------------------------------------------------------------*/
inline void HFCSemaphore::Signal()
    {
#ifdef _WIN32
    HPRECONDITION(GetHandle() != 0);

    ReleaseSemaphore(GetHandle(), 1, NULL);
#endif
    }
END_IMAGEPP_NAMESPACE