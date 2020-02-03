//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFBmpLineEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRFBmpFile.h>
#include <ImagePP/all/h/HRFBmpLineEditor.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFBmpLineEditor::HRFBmpLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   uint16_t       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile           = static_cast<HRFBmpFile*>(GetRasterFile().GetPtr());

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
HRFBmpLineEditor::~HRFBmpLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFBmpLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                    uint64_t pi_PosBlockY,
                                    Byte* po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    // Temporary need for virtual ptr.
    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        return H_NOT_FOUND;

    uint64_t offSetToLine;
    uint64_t new_PosBlockY = ((uint64_t)GetResolutionDescriptor()->GetHeight()-1) - pi_PosBlockY;

    offSetToLine          = m_pRasterFile->m_BmpFileHeader.m_OffBitsToData +
                            (new_PosBlockY * m_ExactBytesPerRow);

    m_pRasterFile->m_pBmpFile->SeekToPos(offSetToLine);

    uint32_t DataSize = GetResolutionDescriptor()->GetBytesPerBlockWidth();
    if(m_pRasterFile->m_pBmpFile->Read(po_pData, DataSize) != DataSize)
        return H_ERROR;

    return H_SUCCESS;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFBmpLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                     uint64_t     pi_PosBlockY,
                                     const Byte*  pi_pData)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);

    HSTATUS Status = H_ERROR;
    uint64_t offSetToLine;

    uint64_t new_PosBlockY = ((uint64_t)GetResolutionDescriptor()->GetHeight()-1) - pi_PosBlockY;

    offSetToLine          = m_pRasterFile->m_BmpFileHeader.m_OffBitsToData +
                            (new_PosBlockY * m_ExactBytesPerRow);

    memcpy(m_pLineBuffer, pi_pData, GetResolutionDescriptor()->GetBytesPerBlockWidth());

    m_pRasterFile->m_pBmpFile->SeekToPos(offSetToLine);

    if(m_pRasterFile->m_pBmpFile->Write(m_pLineBuffer, m_ExactBytesPerRow) != m_ExactBytesPerRow)
        goto WRAPUP;


    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }
