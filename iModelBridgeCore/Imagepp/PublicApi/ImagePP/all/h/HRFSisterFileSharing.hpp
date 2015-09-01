//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSisterFileSharing.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSisterFileSharing
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** ---------------------------------------------------------------------------
    GetLockManager
    Public
    This method returns a pointer on the HFCBinStreamLockManager to intantiate
    a HFCLockMonitor (lock) on the sister file.

    @return A pointer on the m_pLockManager object
    ------------------------------------------------------------------------ */
inline HFCBinStreamLockManager* HRFSisterFileSharing::GetLockManager()
    {
    HPRECONDITION (m_pLockManager != 0);

    // This method is called to instantiate a Lock on the sister file.
    // If the sister file is not instantiate at this point, we will attempt
    // to do it, if it exist.
    if ((!m_BypassSharing) && (m_pSharingControlFile == 0))
        OpenFile();

    return m_pLockManager.get();
    }

/** ---------------------------------------------------------------------------
    GetSisterFilePtr
    Public
    This method returns a pointer on the HFCBinStream to use with IO methods

    @return A pointer on the sister file stream.
    ------------------------------------------------------------------------ */
inline HFCBinStream* HRFSisterFileSharing::GetSisterFilePtr() const
    {
    return (HFCBinStream*)m_pSharingControlFile.get();
    }

/** ---------------------------------------------------------------------------
    IsLocked
    Public

    @return true if the sister file is currently locked
    ------------------------------------------------------------------------ */
inline bool HRFSisterFileSharing::IsLocked() const
    {
    return m_pLockManager->IsLocked();
    }


/** ---------------------------------------------------------------------------
    Public
    Set the bypass sharing flag if it can't be set at construction.
    Only works if set before the first call to GetLockManager.
    ------------------------------------------------------------------------ */
inline void HRFSisterFileSharing::SetSharingBypass(bool pi_BypassSharing)
    {
    if (m_pSharingControlFile == 0)
        {
        m_BypassSharing = pi_BypassSharing;
        }
    }
END_IMAGEPP_NAMESPACE

