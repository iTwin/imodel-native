//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFUSgsFastL7ALineEditor.h>
#include <ImagePP/all/h/HRFUSgsFastL7AFile.h>



//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFUSgsFastL7ALineEditor::HRFUSgsFastL7ALineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                   uint32_t              pi_Page,
                                                   uint16_t       pi_Resolution,
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
HRFUSgsFastL7ALineEditor::~HRFUSgsFastL7ALineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// Read uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFUSgsFastL7ALineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                            uint64_t pi_PosBlockY,
                                            Byte*  po_pData)
    {
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        return H_NOT_FOUND;
        }

    HFCPtr<HRFUSgsFastL7AFile> pFastL7AFile   = static_cast<HRFUSgsFastL7AFile*>(GetRasterFile().GetPtr());

    uint32_t BufferPixelLength = GetResolutionDescriptor()->GetBlockWidth();

    uint64_t Offset = GetRasterFile()->GetOffset() + (pi_PosBlockY * BufferPixelLength);

    if(pFastL7AFile->m_HeaderInfo.m_NumberOfBands < 3)
        {
        if (pFastL7AFile->m_pRedFile->GetCurrentPos() != Offset)
            pFastL7AFile->m_pRedFile->SeekToPos(Offset);

        // Create Grayscale image
        if(pFastL7AFile->m_pRedFile->Read(po_pData, BufferPixelLength) != BufferPixelLength)
            return H_ERROR;
        }
    else
        {
        HArrayAutoPtr<Byte> pBuffRed(new Byte[BufferPixelLength]);
        HArrayAutoPtr<Byte> pBuffGreen(new Byte[BufferPixelLength]);
        HArrayAutoPtr<Byte> pBuffBlue(new Byte[BufferPixelLength]);

        if (pFastL7AFile->m_pRedFile->GetCurrentPos() != Offset)
            pFastL7AFile->m_pRedFile->SeekToPos(Offset);
        if (pFastL7AFile->m_pGreenFile->GetCurrentPos() != Offset)
            pFastL7AFile->m_pGreenFile->SeekToPos(Offset);
        if (pFastL7AFile->m_pBlueFile->GetCurrentPos() != Offset)
            pFastL7AFile->m_pBlueFile->SeekToPos(Offset);

        // Read each channel
        if (pFastL7AFile->m_pRedFile->Read(pBuffRed,     BufferPixelLength) != BufferPixelLength ||
            pFastL7AFile->m_pGreenFile->Read(pBuffGreen, BufferPixelLength) != BufferPixelLength ||
            pFastL7AFile->m_pBlueFile->Read(pBuffBlue,   BufferPixelLength) != BufferPixelLength)
            return H_ERROR;

        // Create RGB image
        for (uint32_t pos=0, bufPos=0; pos<BufferPixelLength; pos++, bufPos+=3)
            {
            po_pData[bufPos]   = pBuffRed[pos];
            po_pData[bufPos+1] = pBuffGreen[pos];
            po_pData[bufPos+2] = pBuffBlue[pos];
            }
        }

    return H_SUCCESS;
    }

