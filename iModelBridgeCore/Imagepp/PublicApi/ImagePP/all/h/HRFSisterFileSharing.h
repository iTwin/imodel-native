//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSisterFileSharing.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSisterFileSharing
//-----------------------------------------------------------------------------
#pragma once

#include "HRFSharingControl.h"
#include "HFCURL.h"
#include "HFCLocalBinStream.h"
#include "HFCBinStreamLockManager.h"

/** ---------------------------------------------------------------------------
    This class implements the creation and the utilization of the physical
    sister file. It is a descendant of the class HRFSharingControl.

    @see HRFSharingControl
// ------------------------------------------------------------------------- */
BEGIN_IMAGEPP_NAMESPACE
class HRFSisterFileSharing : public HRFSharingControl
    {
    HDECLARE_CLASS_ID(HRFSisterFileSharingId, HRFSharingControl);

public:
    //:> Default constructor.
    IMAGEPP_EXPORT HRFSisterFileSharing(const HFCPtr<HFCURL>& pi_rpURL,
                                HFCAccessMode         pi_AccessMode = HFC_READ_ONLY);

    //:> Special constructor: Bypass sharing control
    IMAGEPP_EXPORT HRFSisterFileSharing(const HFCPtr<HFCURL>& pi_rpURL,
                                HFCAccessMode         pi_AccessMode,
                                bool                 pi_BypassSharing);

    virtual ~HRFSisterFileSharing();

    virtual HFCBinStreamLockManager* GetLockManager();

    virtual bool   IsLocked() const;

    void            SetSharingBypass(bool pi_BypassSharing);

protected:
    HFCPtr<HFCURL>                    m_pURL;
    HAutoPtr<HFCBinStreamLockManager> m_pLockManager;
    HAutoPtr<HFCLocalBinStream>       m_pSharingControlFile;
    bool                             m_BypassSharing;

    virtual HFCBinStream* GetSisterFilePtr() const;

private:
    //:> Compute the sister file name from the original.
    void SetSisterFileURL   (const HFCPtr<HFCURL>& pi_rpURL);

    //:> Try to open a sister file if it exist.
    void OpenFile();
    };
END_IMAGEPP_NAMESPACE

#include "HRFSisterFileSharing.hpp"
