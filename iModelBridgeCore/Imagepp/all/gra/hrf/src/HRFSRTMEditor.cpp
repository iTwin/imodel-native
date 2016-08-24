//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSRTMEditor.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSRTMLineEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <Imagepp/all/h/HRFSRTMFile.h>
#include <Imagepp/all/h/HRFSRTMEditor.h>

//---------------------------------------------- HRFSRTMLineEditor

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFSRTMLineEditor::HRFSRTMLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   uint16_t       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
                                   : HRFResolutionEditor(pi_rpRasterFile,
                                   pi_Page,
                                   pi_Resolution,
                                   pi_AccessMode)
    {
    m_pRasterFile = static_cast<HRFSRTMFile*>(GetRasterFile().GetPtr());

    m_ExactBytesPerRow = m_pRasterFile->m_Width * 2;


    m_DataOffset = 0;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFSRTMLineEditor::~HRFSRTMLineEditor()
    {}

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFSRTMLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                    uint64_t pi_PosBlockY,
                                    Byte*  po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        return Status;
        }

    uint64_t offsetToLine;
    offsetToLine = pi_PosBlockY * m_ExactBytesPerRow;

    m_pRasterFile->m_pSRTMFile->SeekToPos(offsetToLine);

    uint32_t DataSize = GetResolutionDescriptor()->GetBytesPerBlockWidth();
    if (m_pRasterFile->m_pSRTMFile->Read(po_pData, DataSize) != DataSize)
        return Status;    // H_ERROR

    //I'll keep it as a reference for big endian, little endian switch that may be necessary. To remove once done.
    //// Switch reb end blue.
    //if (m_pRasterFile->m_IsBGR)
    //{
    //    Byte temporaryValue;
    //    for (uint32_t PixelIndex = 0; PixelIndex < GetResolutionDescriptor()->GetBytesPerBlockWidth(); PixelIndex += 3)
    //    {
    //        temporaryValue = po_pData[PixelIndex];
    //        po_pData[PixelIndex] = po_pData[PixelIndex + 2];
    //        po_pData[PixelIndex + 2] = temporaryValue;
    //    }
    //}

    Status = H_SUCCESS;
    return Status;
    }

//---------------------------------------------- HRFSRTMImageEditor


//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFSRTMImageEditor::HRFSRTMImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     uint16_t       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)
                                     : HRFResolutionEditor(pi_rpRasterFile,
                                     pi_Page,
                                     pi_Resolution,
                                     pi_AccessMode)
    {
    m_pRasterFile = static_cast<HRFSRTMFile*>(GetRasterFile().GetPtr());
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFSRTMImageEditor::~HRFSRTMImageEditor()
    {}

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFSRTMImageEditor::ReadBlock(uint64_t       pi_PosBlockX,
                                     uint64_t       pi_PosBlockY,
                                     Byte*          po_pData)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_pData != 0);

    HSTATUS Status = H_ERROR;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        return Status;
        }

    uint64_t ImageLen = m_pRasterFile->m_pSRTMFile->GetSize();

    // Read the compressed image in memory
    m_pRasterFile->m_pSRTMFile->SeekToPos(0);
    if (m_pRasterFile->m_pSRTMFile->Read(po_pData, ImageLen) != ImageLen)
        return Status;    // H_ERROR

    //The data is in big-endian order. If the system is not, we swap the bytes.
    if (!SystemIsBigEndian())
        {
        SwabArrayOfShort((uint16_t*) po_pData, ImageLen / sizeof(uint16_t));
        }

    Status = H_SUCCESS;
    return Status;
    }