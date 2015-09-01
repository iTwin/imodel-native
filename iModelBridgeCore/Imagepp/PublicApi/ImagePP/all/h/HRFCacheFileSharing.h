//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFCacheFileSharing.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFCacheFileSharing
//-----------------------------------------------------------------------------

#pragma once

#include "HRFSharingControl.h"
#include "HFCBinStreamLockManager.h"

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
class HFCLocalBinStream;
class HFCBinStream;
//:End Ignore

/** ---------------------------------------------------------------------------
    This class does the gestion of the synchronization of the file and the
    memory by comparing a physical and a logical counters. The difference
    between this class and the HRFSisterFileSharing is that the physical
    counter is implented directly into the iTiff (hTiff and HMR) file.

    @see HRFSharingControl
// ------------------------------------------------------------------------- */
class HRFCacheFileSharing : public HRFSharingControl
    {
    HDECLARE_CLASS_ID(HRFFileId_CacheSharing, HRFSharingControl);

public:
    //:> Default constructor.
    HRFCacheFileSharing(HFCBinStream*               pi_rpFile,
                        HFCBinStreamLockManager*    pi_pLockManager,
                        const uint64_t             pi_Offset,
                        HFCAccessMode               pi_AccessMode = HFC_READ_ONLY);

    virtual ~HRFCacheFileSharing();

    virtual bool   IsLocked() const;

    virtual HFCBinStreamLockManager* GetLockManager();

protected:
    //:> Attributes
    HFCBinStreamLockManager* m_pLockManager;
    HFCLocalBinStream*       m_pSharingControlFile;

    virtual HFCBinStream* GetSisterFilePtr() const;

private :
    };
END_IMAGEPP_NAMESPACE

#include "HRFCacheFileSharing.hpp"
