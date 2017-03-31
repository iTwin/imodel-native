//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFUSgsNDFLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImageppInternal.h>
#include <ImagePP/all/h/HRFUSgsNDFLineEditor.h>#include <ImagePP/all/h/HRFUSgsNDFFile.h>


//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFUSgsNDFLineEditor::HRFUSgsNDFLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
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
HRFUSgsNDFLineEditor::~HRFUSgsNDFLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// Read uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFUSgsNDFLineEditor::ReadBlock(uint64_t pi_PosBlockX,
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

    HFCPtr<HRFUSgsNDFFile> pNDFFile   = static_cast<HRFUSgsNDFFile*>(GetRasterFile().GetPtr());

    uint32_t BufferPixelLength = GetResolutionDescriptor()->GetBlockWidth();
    uint64_t Offset = GetRasterFile()->GetOffset() + (pi_PosBlockY * BufferPixelLength);

    if(pNDFFile->m_HeaderInfo.m_NumberOfBands < 3)
        {
        if (pNDFFile->m_pRedFile->GetCurrentPos() != Offset)
            pNDFFile->m_pRedFile->SeekToPos(Offset);

        // Create Grayscale image
        if(pNDFFile->m_pRedFile->Read(po_pData, BufferPixelLength) != BufferPixelLength)
           return H_ERROR;
        }
    else
        {
        HArrayAutoPtr<Byte> pBuffRed(new Byte[BufferPixelLength]);
        HArrayAutoPtr<Byte> pBuffGreen(new Byte[BufferPixelLength]);
        HArrayAutoPtr<Byte> pBuffBlue(new Byte[BufferPixelLength]);

        if (pNDFFile->m_pRedFile->GetCurrentPos() != Offset)
            pNDFFile->m_pRedFile->SeekToPos(Offset);
        if (pNDFFile->m_pGreenFile->GetCurrentPos() != Offset)
            pNDFFile->m_pGreenFile->SeekToPos(Offset);
        if (pNDFFile->m_pBlueFile->GetCurrentPos() != Offset)
            pNDFFile->m_pBlueFile->SeekToPos(Offset);

        // Read each channel
        if(pNDFFile->m_pRedFile->Read(pBuffRed,     BufferPixelLength) != BufferPixelLength ||
           pNDFFile->m_pGreenFile->Read(pBuffGreen, BufferPixelLength) != BufferPixelLength ||
           pNDFFile->m_pBlueFile->Read(pBuffBlue,   BufferPixelLength) != BufferPixelLength)
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

