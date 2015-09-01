//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSharingControl.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** ---------------------------------------------------------------------------
    NeedSynchronization
    Public
    This function returns true if the counters are different

    @return true if the physical and logical counter are different
    ------------------------------------------------------------------------ */
inline bool HRFSharingControl::NeedSynchronization()
    {
    return (m_ModifCount != GetCurrentModifCount());
    }

/** ---------------------------------------------------------------------------
    Synchronize
    Public (HRFSharingControl)
    This method synchronize the physical and logical counters
    ------------------------------------------------------------------------ */
inline void HRFSharingControl::Synchronize()
    {
    //HPRECONDITION (m_IsOpen);

    m_ModifCount = GetCurrentModifCount();
    }
END_IMAGEPP_NAMESPACE

