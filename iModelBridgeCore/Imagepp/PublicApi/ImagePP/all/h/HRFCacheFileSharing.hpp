//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFCacheFileSharing.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFCacheFileSharing
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** ---------------------------------------------------------------------------
    GetSisterFilePtr
    Protected
    This method returns a pointer on the HFCBinStream to use IO routines.

    @return A pointer to the HTiff file stream.
    ------------------------------------------------------------------------ */
inline HFCBinStream* HRFCacheFileSharing::GetSisterFilePtr() const
    {
    HPRECONDITION (m_pSharingControlFile != 0);

    return (HFCBinStream*) m_pSharingControlFile;
    }

/** ---------------------------------------------------------------------------
    IncrementCurrentModifCount
    Public
    Returns true if the first 4 bytes of the TIFF file are locked.

    @return true if the file is locked.
    ------------------------------------------------------------------------ */
inline bool HRFCacheFileSharing::IsLocked() const
    {
    return m_pLockManager->IsLocked();
    }

/** ---------------------------------------------------------------------------
    GetLockManager
    Public
    Returns a pointer on the HFCBinStreamLockManager object

    @return A pointer on the object that does the lock on the file.
    ------------------------------------------------------------------------ */
inline HFCBinStreamLockManager* HRFCacheFileSharing::GetLockManager()
    {
    return m_pLockManager;
    }
END_IMAGEPP_NAMESPACE
