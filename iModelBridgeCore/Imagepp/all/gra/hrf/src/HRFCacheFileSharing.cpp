//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFCacheFileSharing.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFCacheFileSharing
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFCacheFileSharing.h>
#include <Imagepp/all/h/HFCLocalBinStream.h>

/**----------------------------------------------------------------------------
    Constructor
    This methode instanciate the pointer on the physical instance of the sister
    file.

    @param pi_rpFile A pointer to the HFCBinStream of the Tiff file.
    @param pi_pLockManager A pointer to the object that does the file locking.
    @param pi_Offset The offset of the counter into the file.
    @param pi_AccessMode The acess mode of the file.
    ------------------------------------------------------------------------ */
HRFCacheFileSharing::HRFCacheFileSharing(HFCBinStream*            pi_rpFile,
                                         HFCBinStreamLockManager* pi_pLockManager,
                                         const uint64_t          pi_Offset,
                                         HFCAccessMode            pi_AccessMode)
    :HRFSharingControl()
    {
    HPRECONDITION (pi_rpFile != 0);
    HPRECONDITION (pi_pLockManager != 0);
    HPRECONDITION (pi_Offset != 0);
    HPRECONDITION (pi_rpFile->IsCompatibleWith(HFCLocalBinStream::CLASS_ID));

    m_AccessMode = pi_AccessMode;
    m_pSharingControlFile = (HFCLocalBinStream*)pi_rpFile;
    m_Offset = pi_Offset;

    m_pLockManager = pi_pLockManager;

    m_ModifCount = GetCurrentModifCount();
    }

/** ---------------------------------------------------------------------------
    Destructor
    Public
    This function try to delete the sister file from it physical support
    ------------------------------------------------------------------------ */
HRFCacheFileSharing::~HRFCacheFileSharing()
    {
    }