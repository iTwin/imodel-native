//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMutex.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCMutex
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Internal stuff.  Since the key in HFCMutex is allocated and initialised
// at first usage, we must prevent double initialisation that may occur
// if two concurrent operations are made.  To achieve this we use an internal
// static key.  it is not a HFCExclusiveKey to avoid the dependency.
//-----------------------------------------------------------------------------
#ifdef _WIN32
inline HFCMutex::InternalKey::InternalKey()
    {
    InitializeCriticalSection(&m_Key);
    }

inline HFCMutex::InternalKey::~InternalKey()
    {
    DeleteCriticalSection(&m_Key);
    }

inline void HFCMutex::InternalKey::ClaimKey()
    {
    EnterCriticalSection(&m_Key);
    }

inline void HFCMutex::InternalKey::ReleaseKey()
    {
    LeaveCriticalSection(&m_Key);
    }
#endif


//-----------------------------------------------------------------------------
// Claims this mutex.
//-----------------------------------------------------------------------------
inline void HFCMutex::ClaimKey()
    {
#ifdef _WIN32
    HPRECONDITION(GetHandle() != 0);
#endif

    WaitUntilSignaled();
    }


//-----------------------------------------------------------------------------
// Releases this mutex.
//-----------------------------------------------------------------------------
inline void HFCMutex::ReleaseKey()
    {
#ifdef _WIN32
    HPRECONDITION(GetHandle() != 0);

    ReleaseMutex(GetHandle());
#endif
    }

END_IMAGEPP_NAMESPACE