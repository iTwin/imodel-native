//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFFliLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFFliLineEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFFliLineEditor.h>
#include <ImagePP/all/h/HRFFliFile.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFFliLineEditor::HRFFliLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   uint16_t       pi_Resolution,
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
                                    Byte*                po_pData)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        return H_NOT_FOUND;

    uint32_t DataSize = 0;

    // Read the data from the file.
    if(m_pRasterFile->m_pFliFile->GetCurrentPos() != m_PosInFile)
        m_pRasterFile->m_pFliFile->SeekToPos(m_PosInFile);

    DataSize = (uint32_t)(m_pRasterFile->GetChunkSize(1) - m_pRasterFile->GetChunkHeaderSize());

    if(m_pRasterFile->m_pFliFile->Read(po_pData, DataSize) != DataSize)
        return H_ERROR;

    return H_SUCCESS;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFFliLineEditor::WriteBlock(uint64_t              pi_PosBlockX,
                                     uint64_t              pi_PosBlockY,
                                     const Byte*          pi_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    return H_ERROR;
    }
