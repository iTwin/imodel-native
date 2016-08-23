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
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>

#define RASTER_FILE     static_cast<HRFxChFile*>(GetRasterFile().GetPtr())

/** ---------------------------------------------------------------------------
    Constructor.

    @param pi_rpRasterFile Raster file.
    @param pi_Page         Page descriptor.
    @param pi_Resolution   Resolution descriptor.
    @param pi_AccessMode   Access and sharing mode.
    ---------------------------------------------------------------------------
 */
HRFxChEditorRGBA::HRFxChEditorRGBA(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                           uint32_t              pi_Page,
                           unsigned short       pi_Resolution,
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
HRFxChEditorRGBA::~HRFxChEditorRGBA()
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
HSTATUS HRFxChEditorRGBA::ReadBlock(uint64_t pi_PosBlockX,
                                uint64_t pi_PosBlockY,
                                Byte*  po_pData)
    {
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;
    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        return H_NOT_FOUND;

    // sources and .xch are the same block size
    uint32_t BufferPixelLength = GetResolutionDescriptor()->GetBlockWidth() * GetResolutionDescriptor()->GetBlockHeight();
    // the source data, not the .xch
    size_t bytesPerPixel = RASTER_FILE->m_pRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBitsPerPixel() / 8;

    HArrayAutoPtr<Byte> pBuffRed(new Byte[BufferPixelLength*bytesPerPixel*(RASTER_FILE->m_ChannelCount)]);
    Byte* pBuffGreen = &(pBuffRed[BufferPixelLength*bytesPerPixel]);
    Byte* pBuffBlue  = &(pBuffRed[BufferPixelLength*bytesPerPixel*2]);
    Byte* pBuffAlpha = 0; 

    if (RASTER_FILE->m_ChannelCount == 4 && RASTER_FILE->m_pAlphaFile)
        {
        pBuffAlpha  = &(pBuffRed[BufferPixelLength*bytesPerPixel*3]);
        if ((Status = RASTER_FILE->m_AlphaFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffAlpha)) != H_SUCCESS)
            return H_NOT_FOUND;
        }

    //:> Read each channel
    if ((Status = RASTER_FILE->m_RedFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffRed)) != H_SUCCESS ||
        (Status = RASTER_FILE->m_GreenFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffGreen)) != H_SUCCESS ||
        (Status = RASTER_FILE->m_BlueFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffBlue)) != H_SUCCESS)
        return H_NOT_FOUND;

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
            if (bytesPerPixel == 1) // 8 bits
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
        else   // ChannelCount == 4
            {
            if (bytesPerPixel == 1) // 8 bits
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
                uint16_t* pBuffCh416 = (uint16_t *)pBuffAlpha;

                for (uint32_t pos = 0, bufPos = 0; pos < BufferPixelLength; pos++, bufPos += 3)
                    {
                    pData[bufPos]     = pBuffRed16[pos];
                    pData[bufPos + 1] = pBuffGreen16[pos];
                    pData[bufPos + 2] = pBuffBlue16[pos];
                    pData[bufPos + 3] = pBuffCh416[pos];
                    }
                }
            }
        }

    Status = H_SUCCESS;
    return Status;
    }


//----------------------- HRFxChEditorPanchromatic

/** ---------------------------------------------------------------------------
    Constructor.

    @param pi_rpRasterFile Raster file.
    @param pi_Page         Page descriptor.
    @param pi_Resolution   Resolution descriptor.
    @param pi_AccessMode   Access and sharing mode.
    ---------------------------------------------------------------------------
 */
    HRFxChEditorPanchromatic::HRFxChEditorPanchromatic(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                           uint32_t              pi_Page,
                           unsigned short  		 pi_Resolution,
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
    HRFxChEditorPanchromatic::~HRFxChEditorPanchromatic()
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
HSTATUS HRFxChEditorPanchromatic::ReadBlock(uint64_t pi_PosBlockX,
                                            uint64_t pi_PosBlockY,
                                            Byte*  po_pData)
    {
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    HPRECONDITION(RASTER_FILE->m_ChannelCount == 4);

    HPRECONDITION(RASTER_FILE->m_pPanchromaticFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->
                  IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID));
    uint32_t bytesPerPixel = 2;     // V16


    HSTATUS Status = H_ERROR;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        return H_NOT_FOUND;

    uint32_t BufferPixelLength = GetResolutionDescriptor()->GetBlockWidth() * GetResolutionDescriptor()->GetBlockHeight();

    HArrayAutoPtr<Byte> pBuffRed(new Byte[BufferPixelLength* bytesPerPixel * (RASTER_FILE->m_ChannelCount)]);
    Byte* pBuffGreen = &(pBuffRed[BufferPixelLength*bytesPerPixel]);
    Byte* pBuffBlue  = &(pBuffRed[BufferPixelLength*bytesPerPixel*2]);
    Byte* pBuffChrom = &(pBuffRed[BufferPixelLength*bytesPerPixel * 3]);

    if ((Status = RASTER_FILE->m_PanchromaticFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffChrom)) != H_SUCCESS)
        return H_NOT_FOUND;

    // Panchromatic has a 2X better resolution.
    pi_PosBlockX = pi_PosBlockX / 2;
    pi_PosBlockY = pi_PosBlockY / 2;
    //:> Read each channel
    if ((Status = RASTER_FILE->m_RedFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffRed)) != H_SUCCESS ||
        (Status = RASTER_FILE->m_GreenFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffGreen)) != H_SUCCESS ||
        (Status = RASTER_FILE->m_BlueFileResolutionEditor[m_Resolution]->ReadBlock(pi_PosBlockX, pi_PosBlockY, pBuffBlue)) != H_SUCCESS)
        return H_NOT_FOUND;

    //:> Fill client buffer ...
    uint16_t* pData = (uint16_t *)po_pData;
    uint16_t* pBuffRed16 = (uint16_t *)pBuffRed.get();
    uint16_t* pBuffGreen16 = (uint16_t *)pBuffGreen;
    uint16_t* pBuffBlue16 = (uint16_t *)pBuffBlue;
    uint16_t* pBuffCh416 = (uint16_t *)pBuffChrom;

#if (1)
    for (uint32_t pos = 0, bufPos = 0; pos < BufferPixelLength; pos++, bufPos += 3)
        {
        // Convert RGB to IHS
        double red = pBuffRed16[pos >> 1] / 65535.0;
        double green = pBuffGreen16[pos >> 1] / 65535.0;
        double blue = pBuffBlue16[pos >> 1] / 65535.0;

        double Intensity(red+green+blue);
        HASSERT(Intensity >= 0.0 && Intensity <= 3.0);

        double Hue(0.0);
        double minval = min(red,min(green,blue));
        if (blue == minval)
            {
            if (!HDOUBLE_EQUAL_EPSILON(Intensity - (3*blue), 0.0))
                Hue = (green - blue) / (Intensity - (3*blue));
            }
        else if (red == minval)
            Hue = ((blue - red) / (Intensity - (3*red))) + 1;
        else //(green == minval)
            Hue = ((red - green) / (Intensity - (3*green))) + 2;
        HASSERT(Hue >= 0.0 && Hue <= 3.0);

        double Saturation;
        if (HDOUBLE_EQUAL_EPSILON(Intensity,0.0))
            Saturation = 0.0;
        else
            {
            if (Hue <= 1.0)
                Saturation = (Intensity - (3 * blue)) / Intensity;
            else if (Hue <= 2.0)
                Saturation = (Intensity - (3 * red)) / Intensity;
            else // (Hue <= 3.0)
                Saturation = (Intensity - (3 * green)) / Intensity;
            }
        HASSERT(Saturation >= 0.0 && Saturation <= 1.0);

        // Update intensity channel by the panchromatic value
        Intensity = pBuffCh416[pos] / 65535.0;

        // Convert IHS to RGB
        if (pBuffRed16[pos >> 1] == 0 && pBuffGreen16[pos >> 1] == 0 && pBuffBlue16[pos >> 1] == 0)
            {   // Keep the true black - transparency
            pData[bufPos]     = 0;
            pData[bufPos + 1] = 0;
            pData[bufPos + 2] = 0;
            }
        else if (Hue <= 1.0)
            {
            pData[bufPos]     = (uint16_t)(Intensity * (1 + (2*Saturation) - (3*Saturation*Hue)) / 3.0 * 65535);
            pData[bufPos + 1] = (uint16_t)(Intensity * (1 - Saturation     + (3 * Saturation*Hue)) / 3.0 * 65535);
            pData[bufPos + 2] = (uint16_t)(Intensity * (1 - Saturation) / 3.0 * 65535);
            }
        else if (Hue <= 2.0)
            {
            pData[bufPos]     = (uint16_t)(Intensity * (1 - Saturation) / 3.0 * 65535);
            pData[bufPos + 1] = (uint16_t)(Intensity * (1 + (2 * Saturation) - (3 * Saturation*(Hue-1.0))) / 3.0 * 65535);
            pData[bufPos + 2] = (uint16_t)(Intensity * (1 - Saturation       + (3 * Saturation*(Hue - 1.0))) / 3.0 * 65535);
            }
        else // (Hue <= 3.0)
            {
            pData[bufPos]     = (uint16_t)(Intensity * (1 - Saturation + (3 * Saturation*(Hue - 2.0))) / 3.0 * 65535);
            pData[bufPos + 1] = (uint16_t)(Intensity * (1 - Saturation) / 3.0 * 65535);
            pData[bufPos + 2] = (uint16_t)(Intensity * (1 + (2 * Saturation) - (3 * Saturation*(Hue - 2.0))) / 3.0 * 65535);
            }
        }
#endif
#if (0)   // Simple mean
    for (uint32_t pos = 0, bufPos = 0; pos < BufferPixelLength; pos++, bufPos += 3)
        {
        pData[bufPos]     = (pBuffRed16[pos>>1] + pBuffCh416[pos]) >> 1;
        pData[bufPos + 1] = (pBuffGreen16[pos >> 1] + pBuffCh416[pos]) >> 1;
        pData[bufPos + 2] = (pBuffBlue16[pos >> 1] + pBuffCh416[pos]) >> 1;
        }
#endif

    Status = H_SUCCESS;
    return Status;
    }