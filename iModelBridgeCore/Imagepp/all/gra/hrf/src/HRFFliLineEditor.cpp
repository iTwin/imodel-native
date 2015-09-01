//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFFliLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFFliLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFFliLineEditor.h>
#include <Imagepp/all/h/HRFFliFile.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFFliLineEditor::HRFFliLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile = static_cast<HRFFliFile*>(GetRasterFile().GetPtr());

    // Position should be the first valid data in file.
    m_PosInFile   = m_pRasterFile->m_OffsetToData;

    if(GetResolutionDescriptor()->GetBitsPerPixel() != 8)
        {
        HASSERT(!"Unsupported case");
        }
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFFliLineEditor::~HRFFliLineEditor()
    {
    }


//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFFliLineEditor::ReadBlock(uint64_t              pi_PosBlockX,
                                    uint64_t              pi_PosBlockY,
                                    Byte*                po_pData,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HFCLockMonitor SisterFileLock;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        return H_NOT_FOUND;

    uint32_t DataSize = 0;

    // Lock the sister file
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Read the data from the file.
    if(m_pRasterFile->m_pFliFile->GetCurrentPos() != m_PosInFile)
        m_pRasterFile->m_pFliFile->SeekToPos(m_PosInFile);

    DataSize = (uint32_t)(m_pRasterFile->GetChunkSize(1) - m_pRasterFile->GetChunkHeaderSize());

    if(m_pRasterFile->m_pFliFile->Read(po_pData, DataSize) != DataSize)
        return H_ERROR;

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    return H_SUCCESS;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFFliLineEditor::WriteBlock(uint64_t              pi_PosBlockX,
                                     uint64_t              pi_PosBlockY,
                                     const Byte*          pi_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    return H_ERROR;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFFliLineEditor::OnSynchronizedSharingControl()
    {
    // The close-open sequence will reload the info of the file. This way, we
    // are sure that the information that will be read will be up to date.
    static_cast<HRFFliFile*>(GetRasterFile().GetPtr())->SaveFliFile(true);
    static_cast<HRFFliFile*>(GetRasterFile().GetPtr())->Open();
    }