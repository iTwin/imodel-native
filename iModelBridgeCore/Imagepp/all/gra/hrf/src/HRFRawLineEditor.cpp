//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRawLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFRawLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFRawLineEditor.h>
#include <Imagepp/all/h/HRFRawFile.h>



//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFRawLineEditor::HRFRawLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFRawLineEditor::~HRFRawLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// Read uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFRawLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                    uint64_t pi_PosBlockY,
                                    Byte*  po_pData,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_SUCCESS;

    // Temporary need for virtual ptr.
    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {

        HFCPtr<HRFRawFile> pRawFile   = static_cast<HRFRawFile*>(GetRasterFile().GetPtr());
        uint64_t           Offset = GetRasterFile()->GetOffset() +
                                     (pi_PosBlockY * GetResolutionDescriptor()->GetBytesPerWidth());

        // Lock the sister file if needed
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Get lock and synch.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
            pi_pSisterFileLock = &SisterFileLock;
            }

        if (pRawFile->m_pRawFile->GetCurrentPos() != Offset)
            pRawFile->m_pRawFile->SeekToPos(Offset);


        uint32_t DataSize = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth();
        if(pRawFile->m_pRawFile->Read(po_pData, DataSize) != DataSize)
            Status = H_ERROR;

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }
    else
        Status = H_NOT_FOUND;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// Write uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFRawLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                     uint64_t     pi_PosBlockY,
                                     const Byte*  pi_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_pData != 0);

    HSTATUS             Status      = H_SUCCESS;
    HFCPtr<HRFRawFile> pRawFile   = static_cast<HRFRawFile*>(GetRasterFile().GetPtr());
    uint64_t           Offset      = GetRasterFile()->GetOffset() +
                                      (pi_PosBlockY * GetResolutionDescriptor()->GetBytesPerWidth());

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (pRawFile->m_pRawFile->GetCurrentPos() != Offset)
        pRawFile->m_pRawFile->SeekToPos(Offset);

    uint32_t DataSize = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth();
    if(pRawFile->m_pRawFile->Write (pi_pData, DataSize) != DataSize)
        Status = H_ERROR;

    // Increment the counters.
    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sisterfile.
    SisterFileLock.ReleaseKey();

    return Status;
    }
