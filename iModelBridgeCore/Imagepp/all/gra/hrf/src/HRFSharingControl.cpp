//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSharingControl.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSharingControl
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFSharingControl.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCMonitor.h>
#include <Imagepp/all/h/HFCBinStreamLockManager.h>

/** ---------------------------------------------------------------------------
    Constructor
    Public
    This is the default constructor for this class
    ------------------------------------------------------------------------- */
HRFSharingControl::HRFSharingControl()
    {
    m_ModifCount            = 0;
    m_Offset                = 0;
    m_AccessMode            = HFC_READ_ONLY | HFC_SHARE_READ_WRITE;
    //m_IsOpen                = false;
    }


/** ---------------------------------------------------------------------------
    Destructor
    Public
    This function try to delete the sister file from it physical support
    ------------------------------------------------------------------------ */
HRFSharingControl::~HRFSharingControl()
    {
    //:> This function shall try to delete the sister file. This attemp has to
    //:> fail if another instance of this class has the same sister file opened.
    //:> It is automatically done by the destructor of the HFCLocalBinStream.

    //m_IsOpen = false;
    }

/** ---------------------------------------------------------------------------
    GetCurrentModifCount
    Protected
    This function returns the physical modification count
    ------------------------------------------------------------------------ */
uint32_t HRFSharingControl::GetCurrentModifCount()
    {
    uint32_t Count = 0;

    if (GetSisterFilePtr() != 0)
        {
        HFCLockMonitor SisterFileLock (GetLockManager());
        GetSisterFilePtr()->SeekToPos(m_Offset);
        GetSisterFilePtr()->Read(&Count, sizeof (uint32_t));
        SisterFileLock.ReleaseKey();
        }

    return Count;
    }

/** ---------------------------------------------------------------------------
    IncrementCurrentModifCount
    Public

    If a physical sister file has been created, opened and locked, add 1 to
    the current logical and physical counters.
    The logical and physical counter must be synchronized.
    ------------------------------------------------------------------------ */
void HRFSharingControl::IncrementCurrentModifCount()
    {
    HPRECONDITION (m_ModifCount == GetCurrentModifCount());
    HPRECONDITION (GetLockManager()->IsLocked());

    if (GetSisterFilePtr() != 0)
        {
        HASSERT(m_AccessMode.m_HasWriteAccess);

        ++m_ModifCount;

        GetSisterFilePtr()->SeekToPos(m_Offset);
        GetSisterFilePtr()->Write(&m_ModifCount, sizeof(uint32_t));
        }
    }
