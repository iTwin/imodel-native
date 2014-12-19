//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFWbmpLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFWbmpLineEditor
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRFWbmpFile.h>
#include <Imagepp/all/h/HRFWbmpLineEditor.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFWbmpLineEditor::HRFWbmpLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     unsigned short       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile           = static_cast<HRFWbmpFile*>(GetRasterFile().GetPtr());

    HASSERT(GetResolutionDescriptor()->GetWidth() <= ULONG_MAX);

    m_UsedBitsPerRow        = GetResolutionDescriptor()->GetPixelType()->CountPixelRawDataBits() *
                              (uint32_t)GetResolutionDescriptor()->GetWidth();

    m_PaddingBitsPerRow     = m_pRasterFile->m_PaddingBitsPerRow;

    m_BitsPerRow            = m_UsedBitsPerRow + m_PaddingBitsPerRow;
    m_ExactBytesPerRow      = m_BitsPerRow / 8;

    m_pLineBuffer           = new Byte[m_ExactBytesPerRow];

    // Fill the buffer's end with 0.
    memset(m_pLineBuffer + GetResolutionDescriptor()->GetBytesPerBlockWidth(), 0x00, (m_PaddingBitsPerRow / 8));
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFWbmpLineEditor::~HRFWbmpLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFWbmpLineEditor::ReadBlock(uint32_t pi_PosBlockX,
                                     uint32_t pi_PosBlockY,
                                     Byte* po_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;
    HFCLockMonitor SisterFileLock;

    // Temporary need for virtual ptr.
    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    // Lock the sister file
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    size_t offSetToLine = m_pRasterFile->m_OffsetToFirstRowInByte +
                          (pi_PosBlockY * m_ExactBytesPerRow);

    m_pRasterFile->m_pWbmpFile->SeekToPos(offSetToLine);

    uint32_t DataSize = GetResolutionDescriptor()->GetBytesPerBlockWidth();
    if(m_pRasterFile->m_pWbmpFile->Read((void*)po_pData, DataSize) != DataSize)
        goto WRAPUP;

    SisterFileLock.ReleaseKey();

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFWbmpLineEditor::WriteBlock(uint32_t              pi_PosBlockX,
                                      uint32_t              pi_PosBlockY,
                                      const Byte*          pi_pData,
                                      HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);

    HSTATUS Status = H_ERROR;

    size_t offSetToLine = m_pRasterFile->m_OffsetToFirstRowInByte +
                          (pi_PosBlockY * m_ExactBytesPerRow);

    memcpy(m_pLineBuffer, pi_pData, GetResolutionDescriptor()->GetBytesPerBlockWidth());

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    m_pRasterFile->m_pWbmpFile->SeekToPos(offSetToLine);

    if(m_pRasterFile->m_pWbmpFile->Write((void*)m_pLineBuffer, m_ExactBytesPerRow) != m_ExactBytesPerRow)
        goto WRAPUP;

    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }