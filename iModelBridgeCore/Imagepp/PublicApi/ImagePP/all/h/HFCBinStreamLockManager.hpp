//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCBinStreamLockManager.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HFCBinStreamLockManager
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** ---------------------------------------------------------------------------
    Constructor
    Public
    This is the only constructor allowed for this object. It creates the object
    with all the parameters it needs to perform a lock on a file.
    If the pi_pBinStream parameter is set to 0, it means that no file is
    associate with the object yet. The locks will then be virtual. This means
    that they will not be performed for real and only the m_refCount object
    will be increment.

    @param pi_pBinStream A pointer on the file stream to lock.
    @param pi_Pos The position in the file where to begin the lock.
    @param pi_Size The size of the block to lock.
    @param pi_Share Permit or not the other program to acess the locked part.
    ------------------------------------------------------------------------ */
inline HFCBinStreamLockManager::HFCBinStreamLockManager(HFCBinStream* pi_pBinStream,
                                                        uint64_t pi_Pos,
                                                        uint64_t pi_Size,
                                                        bool pi_Share)
    {
    HPRECONDITION (pi_Pos >= 0);
    HPRECONDITION (pi_Size >= 0);

    m_pBinStream = pi_pBinStream;
    m_Position = pi_Pos;
    m_Size = pi_Size;
    m_Share = pi_Share;
    m_RefCount = 0;
    }

/** ---------------------------------------------------------------------------
    Destructor
    Public
    Destruct the instance of this class.
    ------------------------------------------------------------------------ */
inline HFCBinStreamLockManager::~HFCBinStreamLockManager()
    {
    HPRECONDITION ((m_pBinStream == 0) || (m_RefCount == 0));
    }

/** ---------------------------------------------------------------------------
    Lock
    Public
    Perform a lock on the file stream with the specied parameters
    ------------------------------------------------------------------------ */
inline void HFCBinStreamLockManager::Lock()
    {
    // If the file stream is not instanciate, we perform a virtual lock by
    // incrementing the m_RefCount attribute.
    if ((m_pBinStream != 0) && (m_RefCount == 0))
        m_pBinStream->Lock(m_Position, m_Size, m_Share);

    ++m_RefCount;
    }

/** ---------------------------------------------------------------------------
    Unlock
    Public
    Unlock the file stream. The file must have been locked at first.
    ------------------------------------------------------------------------ */
inline void HFCBinStreamLockManager::UnLock()
    {
    HPRECONDITION (m_RefCount > 0);

    --m_RefCount;

    if ((m_pBinStream != 0) && (m_RefCount == 0))
        m_pBinStream->Unlock(m_Position, m_Size);
    }

/** ---------------------------------------------------------------------------
    Claim
    Public
    This method is called by the HFCLockManager templated class. It only calls
    a lock on the file stream.
    ------------------------------------------------------------------------ */
inline void HFCBinStreamLockManager::ClaimKey()
    {
    Lock();
    }

/** ---------------------------------------------------------------------------
    ReleaseKey
    Public
    This method is called by the HFCLockManager templated class. It only calls
    an unlock on the file stream.
    ------------------------------------------------------------------------ */
inline void HFCBinStreamLockManager::ReleaseKey()
    {
    UnLock();
    }

/** ---------------------------------------------------------------------------
    IsLock
    Public
    This function returns true if a lock has been perform on the file stream
    and has not been unlocked yet.

    @return true if the file stream is locked.
    ------------------------------------------------------------------------ */
inline bool HFCBinStreamLockManager::IsLocked()
    {
    return m_RefCount > 0;
    }

/** ---------------------------------------------------------------------------
    GetRefCount
    Public
    This function returns the RefCount of the object.
    It must only be called in the OpenFile of the HRFSisterFileSharing object.

    @return Byte The locking counter value.
    ------------------------------------------------------------------------ */
inline Byte HFCBinStreamLockManager::GetRefCount() const
    {
    return m_RefCount;
    }

/** ---------------------------------------------------------------------------
    SetSisterFileStream
    Public

    This method set the pointer on the sister file stream when an object
    detect that it has been created since the last lock.
    ------------------------------------------------------------------------ */
inline void HFCBinStreamLockManager::SetSisterFileStream(HFCBinStream* pi_pBinStream)
    {
    HPRECONDITION (pi_pBinStream != 0);
    HPRECONDITION (m_pBinStream == 0);

    m_pBinStream = pi_pBinStream;

    // If there was already a virtual lock on the sister file, convert it to
    // a real one physicaly. The refCount is not change because it is not a
    // new lock but a conversion from an old one.
    if (m_RefCount != 0)
        m_pBinStream->Lock(m_Position, m_Size, m_Share);
    }
END_IMAGEPP_NAMESPACE