//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFxChEditor.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HRFxChFile.h>
#include <Imagepp/all/h/HRFxChEditor.h>
#include <Imagepp/all/h/HRFHMRFile.h>

#define RASTER_FILE     static_cast<HRFxChFile*>(GetRasterFile().GetPtr())

/** ---------------------------------------------------------------------------
    Constructor.

    @param pi_rpRasterFile Raster file.
    @param pi_Page         Page descriptor.
    @param pi_Resolution   Resolution descriptor.
    @param pi_AccessMode   Access and sharing mode.
    ---------------------------------------------------------------------------
 */
HRFxChEditor::HRFxChEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                           uint32_t              pi_Page,
                           uint16_t       pi_Resolution,
                           HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    }


/** ---------------------------------------------------------------------------
    Destructor.
    ---------------------------------------------------------------------------
 */
HRFxChEditor::~HRFxChEditor()
    {
    }


/** ---------------------------------------------------------------------------
    Read a specific block of pixels beginning at @i{pi_PosBlockX}, @i{pi_PosBlockY}.

    @param pi_PosBlockX  X position of the block in the file.
    @param pi_PosBlockY  Y position of the block in the file.
    @param po_pData      Buffer to contain data.

    @return HSTATUS.
    ---------------------------------------------------------------------------
 */
HSTATUS HRFxChEditor::ReadBlock(uint64_t pi_PosBlockX,
                                uint64_t pi_PosBlockY,
                                Byte*  po_pData)
    {
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

        {
        uint32_t BufferPixelLength = GetResolutionDescriptor()->GetBlockWidth() * GetResolutionDescriptor()->GetBlockHeight();
        size_t bytesPerPixel = GetResolutionDescriptor()->GetBitsPerPixel() / 8;

        HArrayAutoPtr<Byte> pBuffRed(new Byte[BufferPixelLength*bytesPerPixel*(RASTER_FILE->m_ChannelCount)]);
        Byte* pBuffGreen = &(pBuffRed[BufferPixelLength*bytesPerPixel]);
        Byte* pBuffBlue  = &(pBuffRed[BufferPixelLength*bytesPerPixel*2]);
        Byte* pBuffAlpha = 0;

        if (RASTER_FILE->m_ChannelCount == 4)
            pBuffAlpha  = &(pBuffRed[BufferPixelLength*bytesPerPixel*3]);

        //:> Read each channel
        if ((Status = RASTER_FILE->m_RedFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffRed)) != H_SUCCESS ||
            (Status = RASTER_FILE->m_GreenFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffGreen)) != H_SUCCESS ||
            (Status = RASTER_FILE->m_BlueFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffBlue)) != H_SUCCESS)
            goto WRAPUP;

        if (RASTER_FILE->m_ChannelCount == 4)
            if ((Status = RASTER_FILE->m_AlphaFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffAlpha)) != H_SUCCESS)
                goto WRAPUP;

        //:> Fill client buffer ...

        //:> Check red channel only since all channels are of the same format
        if (RASTER_FILE->m_pRedFile->GetClassID() == HRFHMRFile::CLASS_ID)
            {
            if (RASTER_FILE->m_ChannelCount == 3)
                {
                for (uint32_t pos=0, bufPos=0; pos<BufferPixelLength; pos++, bufPos+=3)
                    {
                    po_pData[bufPos]   = RASTER_FILE->m_pRedMap[(pBuffRed[pos])];
                    po_pData[bufPos+1] = RASTER_FILE->m_pGreenMap[(pBuffGreen[pos])];
                    po_pData[bufPos+2] = RASTER_FILE->m_pBlueMap[(pBuffBlue[pos])];
                    }
                }
            else
                {
                for (uint32_t pos=0, bufPos=0; pos<BufferPixelLength; pos++, bufPos+=4)
                    {
                    po_pData[bufPos]   = RASTER_FILE->m_pRedMap[(pBuffRed[pos])];
                    po_pData[bufPos+1] = RASTER_FILE->m_pGreenMap[(pBuffGreen[pos])];
                    po_pData[bufPos+2] = RASTER_FILE->m_pBlueMap[(pBuffBlue[pos])];
                    po_pData[bufPos+3] = RASTER_FILE->m_pAlphaMap[(pBuffAlpha[pos])];
                    }
                }
            }
        else // grayscale
            {
            if (RASTER_FILE->m_ChannelCount == 3)
                {
                if (bytesPerPixel == 3) // 8 bits
                    {
                    for (uint32_t pos=0, bufPos=0; pos<BufferPixelLength; pos++, bufPos+=3)
                        {
                        po_pData[bufPos]   = pBuffRed[pos];
                        po_pData[bufPos+1] = pBuffGreen[pos];
                        po_pData[bufPos+2] = pBuffBlue[pos];
                        }
                    }
                else    // 16 bits
                    {
                    uint16_t* pData = (uint16_t *)po_pData;
                    uint16_t* pBuffRed16 = (uint16_t *)pBuffRed.get();
                    uint16_t* pBuffGreen16 = (uint16_t *)pBuffGreen;
                    uint16_t* pBuffBlue16 = (uint16_t *)pBuffBlue;
                    for (uint32_t pos = 0, bufPos = 0; pos < BufferPixelLength; pos++, bufPos += 3)
                        {
                        pData[bufPos]     = pBuffRed16[pos];
                        pData[bufPos + 1] = pBuffGreen16[pos];
                        pData[bufPos + 2] = pBuffBlue16[pos];
                        }
                    }
                }
            else
                {
                if (bytesPerPixel == 3) // 8 bits
                    {
                    for (uint32_t pos=0, bufPos=0; pos<BufferPixelLength; pos++, bufPos+=4)
                        {
                        po_pData[bufPos]   = pBuffRed[pos];
                        po_pData[bufPos+1] = pBuffGreen[pos];
                        po_pData[bufPos+2] = pBuffBlue[pos];
                        po_pData[bufPos+3] = pBuffAlpha[pos];
                        }
                    }
                else    // 16 bits
                    {
                    uint16_t* pData = (uint16_t *)po_pData;
                    uint16_t* pBuffRed16 = (uint16_t *)pBuffRed.get();
                    uint16_t* pBuffGreen16 = (uint16_t *)pBuffGreen;
                    uint16_t* pBuffBlue16 = (uint16_t *)pBuffBlue;
                    uint16_t* pBuffAlpha16 = (uint16_t *)pBuffAlpha;
                    for (uint32_t pos = 0, bufPos = 0; pos < BufferPixelLength; pos++, bufPos += 3)
                        {
                        pData[bufPos]     = pBuffRed16[pos];
                        pData[bufPos + 1] = pBuffGreen16[pos];
                        pData[bufPos + 2] = pBuffBlue16[pos];
                        pData[bufPos + 3] = pBuffAlpha16[pos];
                        }
                    }
                }
            }
        }
    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }