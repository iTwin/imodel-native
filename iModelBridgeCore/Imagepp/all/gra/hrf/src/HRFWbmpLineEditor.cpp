//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFWbmpLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFWbmpLineEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRFWbmpFile.h>
#include <ImagePP/all/h/HRFWbmpLineEditor.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFWbmpLineEditor::HRFWbmpLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     uint16_t       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile           = static_cast<HRFWbmpFile*>(GetRasterFile().GetPtr());

    HASSERT(GetResolutionDescriptor()->GetWidth() <= UINT32_MAX);

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
HSTATUS HRFWbmpLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                     uint64_t pi_PosBlockY,
                                     Byte*  po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;

    // Temporary need for virtual ptr.
    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        return Status;
        }

    size_t offSetToLine = m_pRasterFile->m_OffsetToFirstRowInByte + ((uint32_t)pi_PosBlockY * m_ExactBytesPerRow);

    m_pRasterFile->m_pWbmpFile->SeekToPos(offSetToLine);

    uint32_t DataSize = GetResolutionDescriptor()->GetBytesPerBlockWidth();
    if(m_pRasterFile->m_pWbmpFile->Read(po_pData, DataSize) != DataSize)
        return Status;

    Status = H_SUCCESS;
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFWbmpLineEditor::WriteBlock(uint64_t              pi_PosBlockX,
                                      uint64_t              pi_PosBlockY,
                                      const Byte*           pi_pData)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);

    HSTATUS Status = H_ERROR;

    size_t offSetToLine = m_pRasterFile->m_OffsetToFirstRowInByte + ((uint32_t)pi_PosBlockY * m_ExactBytesPerRow);

    memcpy(m_pLineBuffer, pi_pData, GetResolutionDescriptor()->GetBytesPerBlockWidth());

    m_pRasterFile->m_pWbmpFile->SeekToPos(offSetToLine);

    if(m_pRasterFile->m_pWbmpFile->Write(m_pLineBuffer.get(), m_ExactBytesPerRow) != m_ExactBytesPerRow)
        return Status;

    Status = H_SUCCESS;
    return Status;
    }
