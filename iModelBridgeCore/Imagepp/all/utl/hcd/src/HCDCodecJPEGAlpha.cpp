//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecJPEGAlpha.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecJPEGAlpha
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecJPEGAlpha.h>
#include <Imagepp/all/h/HFCBuffer.h>
#include <Imagepp/all/h/HCDCodecRLE8.h>

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecJPEGAlpha::HCDCodecJPEGAlpha()
    : HCDCodecJPEG()
    {
    try
        {
        InitObject();
        }
    catch(...)
        {
        DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecJPEGAlpha::HCDCodecJPEGAlpha(uint32_t pi_Width,
                                     uint32_t pi_Height,
                                     uint32_t pi_BitsPerPixel)
    : HCDCodecJPEG(pi_Width, pi_Height, pi_BitsPerPixel)
    {
    try
        {
        InitObject();
        }
    catch(...)
        {
        DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecJPEGAlpha::HCDCodecJPEGAlpha(const HCDCodecJPEGAlpha& pi_rObj)
    : HCDCodecJPEG(pi_rObj)
    {
    try
        {
        DeepCopy(pi_rObj);
        }
    catch(...)
        {
        DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecJPEGAlpha::~HCDCodecJPEGAlpha()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecJPEGAlpha::Clone() const
    {
    return new HCDCodecJPEGAlpha(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecJPEGAlpha::CompressSubset(const void* pi_pInData,
                                         size_t      pi_InDataSize,
                                         void*       po_pOutBuffer,
                                         size_t      po_OutBufferSize)
    {
    HPRECONDITION(m_pCodecJpeg != 0);
    HPRECONDITION(m_pCodecRLE8 != 0);
    HFCBuffer PixelData  (1024, 1);
    HFCBuffer OpacityData(1024, 1);
    size_t    Result = 0;



    // extract the pixel & opacity data
    if (GetColorMode() == RGB_OPACITY)
        {
        size_t PixelDataSize   = GetWidth() * 3 * GetHeight();
        size_t OpacityDataSize = GetWidth() * GetHeight();

        Byte* pPixel   = PixelData.PrepareForNewData(PixelDataSize + 1);  // add 1 byte to permit copying four bytes at a time
        PixelData.SetNewDataSize(PixelDataSize);
        Byte* pOpacity = OpacityData.PrepareForNewData(OpacityDataSize);
        OpacityData.SetNewDataSize(OpacityDataSize);

        size_t SrcOffset   = 0;
        size_t DestPixelOffset   = 0;
        size_t DestOpacityOffset = 0;
        for (; SrcOffset < pi_InDataSize; SrcOffset+=4, DestPixelOffset+=3, ++DestOpacityOffset)
            {
            memcpy(pPixel + DestPixelOffset, (Byte*)pi_pInData + SrcOffset, 4);
            *(pOpacity + DestOpacityOffset) = *((Byte*)pi_pInData + SrcOffset + 3);
            }
        }
    else
        {
        size_t PixelDataSize   = GetWidth() * GetHeight();
        size_t OpacityDataSize = GetWidth() * GetHeight();

        Byte* pPixel   = PixelData.PrepareForNewData(PixelDataSize);
        PixelData.SetNewDataSize(PixelDataSize);
        Byte* pOpacity = OpacityData.PrepareForNewData(OpacityDataSize);
        OpacityData.SetNewDataSize(OpacityDataSize);

        size_t SrcOffset   = 0;
        size_t DestOffset   = 0;
        for (; SrcOffset < pi_InDataSize; SrcOffset+=2, ++DestOffset)
            {
            pPixel[DestOffset]   = *((Byte*)pi_pInData + SrcOffset);
            pOpacity[DestOffset] = *((Byte*)pi_pInData + SrcOffset + 1);
            }
        }

    // Compress the JPEG part
    size_t RLEOffset = 0;
    size_t CompressResult;
    Result += 4;
    if ((CompressResult = m_pCodecJpeg->CompressSubset(PixelData.GetData(),
                                                       PixelData.GetDataSize(),
                                                       (Byte*)po_pOutBuffer + Result,
                                                       po_OutBufferSize - Result)) > 0)
        {
        // update the result
        Result += CompressResult;
        RLEOffset = Result;

        // compress the RLE8 part
        if ((CompressResult = m_pCodecRLE8->CompressSubset(OpacityData.GetData(),
                                                           OpacityData.GetDataSize(),
                                                           (Byte*)po_pOutBuffer + Result,
                                                           po_OutBufferSize - Result)) > 0)
            {
            Result += CompressResult;
            }
        else
            Result = 0;
        }
    else
        Result = 0;

    // Set the RLE8 offset in the first 4 bytes
    if (Result != 0)
        {
        HASSERT_X64(RLEOffset < ULONG_MAX);
        *((uint32_t*)po_pOutBuffer) = (uint32_t)RLEOffset;
        }

    return (Result);
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecJPEGAlpha::DecompressSubset(const void* pi_pInData,
                                           size_t      pi_InDataSize,
                                           void*       po_pOutBuffer,
                                           size_t      pi_OutBufferSize)
    {
    HPRECONDITION(m_pCodecJpeg != 0);
    HPRECONDITION(m_pCodecRLE8 != 0);
    HFCBuffer OpacityData(1024, 1);
    size_t    Result = 0;

    size_t RLE8Offset = *((uint32_t*)pi_pInData);
    size_t JpegSize  = RLE8Offset - 4;
    size_t RLE8Size  = pi_InDataSize - RLE8Offset;
    size_t JPEGResult;
    size_t RLE8Result;
    Byte* pRLE8Data = OpacityData.PrepareForNewData(m_pCodecRLE8->GetWidth() * m_pCodecRLE8->GetHeight());
    if (((JPEGResult = m_pCodecJpeg->DecompressSubset((Byte*)pi_pInData + 4,
                                                      JpegSize,
                                                      po_pOutBuffer,
                                                      pi_OutBufferSize)) > 0) &&
        ((RLE8Result = m_pCodecRLE8->DecompressSubset((Byte*)pi_pInData + RLE8Offset,
                                                      RLE8Size,
                                                      pRLE8Data,
                                                      RLE8Size)) > 0))
        {
        // Expand the uncompressed RGB data to make place for the future alpha channel
        if (GetColorMode() == RGB_OPACITY)
            {
            // Compute the starting offset of the source and destination
            size_t SrcStart   = (GetWidth() * GetHeight() * 3) - 1;
            size_t DestStart  = (GetWidth() * GetHeight() * 4) - 1;
            size_t AlphaStart = (GetWidth() * GetHeight()) - 1;

            // Shift the data to make place for the Alpha channel
            Byte* pSrcData   = (Byte*)po_pOutBuffer + SrcStart;
            Byte* pDestData  = (Byte*)po_pOutBuffer + DestStart;
            Byte* pAlphaData = pRLE8Data + AlphaStart;
            for (; pSrcData >= po_pOutBuffer; pSrcData-=3, pDestData-=4, pAlphaData - 1)
                {
                *((uint32_t*)pDestData)  = 0x000000FF;
                *((uint32_t*)pDestData) |= *((uint32_t*)pSrcData);
                *((uint32_t*)pDestData) |= *pAlphaData;
                }
            }
        else
            {
            // Compute the starting offset of the source and destination
            size_t SrcStart   = (GetWidth() * GetHeight()) - 1;
            size_t DestStart  = (GetWidth() * GetHeight() * 2) - 1;

            // Shift the data to make place for the Alpha channel
            Byte* pSrcData   = (Byte*)po_pOutBuffer + SrcStart;
            Byte* pDestData  = (Byte*)po_pOutBuffer + DestStart;
            Byte* pAlphaData = pRLE8Data + SrcStart;
            for (; pSrcData >= po_pOutBuffer; pSrcData-=1, pDestData-=2, pAlphaData - 1)
                {
                *((unsigned short*)pDestData)  = 0x00FF;
                *((unsigned short*)pDestData) |= *((unsigned short*)pSrcData);
                *((unsigned short*)pDestData) |= *((Byte*)pAlphaData);
                }
            }

        Result = JPEGResult + RLE8Result;
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecJPEGAlpha::HasLineAccess() const
    {
    return false;
    }



//-----------------------------------------------------------------------------
// public
// GetMinimumSubsetSize
//-----------------------------------------------------------------------------
size_t HCDCodecJPEGAlpha::GetMinimumSubsetSize() const
    {
    size_t ImageWidthInBytes = (GetWidth() * GetBitsPerPixel() + GetLinePaddingBits() + 7) / 8;

    return ImageWidthInBytes * GetHeight();
    }


//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecJPEGAlpha::GetSubsetMaxCompressedSize() const
    {
    HPRECONDITION(m_pCodecJpeg != 0);
    HPRECONDITION(m_pCodecRLE8 != 0);

    return (m_pCodecJpeg->GetSubsetMaxCompressedSize() +
            m_pCodecRLE8->GetSubsetMaxCompressedSize() +
            4);
    }


//-----------------------------------------------------------------------------
// public
// InitObject
//-----------------------------------------------------------------------------
void HCDCodecJPEGAlpha::InitObject()
    {
    size_t BPP = GetBitsPerPixel();
    if (BPP > 8)
        BPP -= 8;

    // Create the codec
    m_pCodecJpeg = new HCDCodecIJG(GetWidth(), GetHeight(), BPP);
    m_pCodecRLE8 = new HCDCodecRLE8(GetWidth(), GetHeight(), 8);

    // Set the color type
    if(GetBitsPerPixel() == 16)
        SetColorMode(GRAYSCALE_OPACITY);
    else
        SetColorMode(RGB_OPACITY);
    }

//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------
void HCDCodecJPEGAlpha::DeepCopy(const HCDCodecJPEGAlpha& pi_rObj)
    {
    HPRECONDITION(pi_rObj.m_pCodecJpeg != 0);
    HPRECONDITION(pi_rObj.m_pCodecRLE8 != 0);

    InitObject();

    m_pCodecJpeg = (HCDCodecIJG*)pi_rObj.m_pCodecJpeg->Clone();
    m_pCodecRLE8 = (HCDCodecRLE8*)pi_rObj.m_pCodecRLE8->Clone();

    SetColorMode(pi_rObj.m_ColorMode);
    }


//-----------------------------------------------------------------------------
// private
// DeepDelete
//-----------------------------------------------------------------------------
void HCDCodecJPEGAlpha::DeepDelete()
    {
    m_pCodecJpeg = 0;
    m_pCodecRLE8 = 0;
    }


//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
void HCDCodecJPEGAlpha::Reset()
    {
    HPRECONDITION(m_pCodecJpeg != 0);
    HPRECONDITION(m_pCodecRLE8 != 0);

    HCDCodecJPEG::Reset();

    m_pCodecJpeg->Reset();
    m_pCodecRLE8->Reset();
    }

//-----------------------------------------------------------------------------
// public
// SetBitsPerPixel
//-----------------------------------------------------------------------------
void HCDCodecJPEGAlpha::SetBitsPerPixel(size_t pi_BitsPerPixel)
    {
    HPRECONDITION(m_pCodecJpeg != 0);
    HPRECONDITION(m_pCodecRLE8 != 0);
    HPRECONDITION((pi_BitsPerPixel == 16) || (pi_BitsPerPixel == 32));

    m_pCodecJpeg->SetBitsPerPixel(pi_BitsPerPixel - 8);

    if(pi_BitsPerPixel == 32)
        {
        m_pCodecJpeg->SetColorMode(HCDCodecIJG::RGB);
        m_ColorMode = RGB_OPACITY;
        }
    else
        {
        m_pCodecJpeg->SetColorMode(HCDCodecIJG::GRAYSCALE);
        m_ColorMode = GRAYSCALE_OPACITY;
        }
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
void HCDCodecJPEGAlpha::SetColorMode(ColorModes pi_Mode)
    {
    HPRECONDITION(m_pCodecJpeg != 0);
    HPRECONDITION(m_pCodecRLE8 != 0);
    HPRECONDITION((pi_Mode == RGB_OPACITY) || (pi_Mode == GRAYSCALE_OPACITY));

    m_ColorMode = pi_Mode;

    if (m_ColorMode == RGB_OPACITY)
        SetBitsPerPixel(32);
    else if (m_ColorMode== GRAYSCALE_OPACITY)
        SetBitsPerPixel(16);
    }



//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
HCDCodecJPEGAlpha::ColorModes HCDCodecJPEGAlpha::GetColorMode() const
    {
    return (m_ColorMode);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
void HCDCodecJPEGAlpha::SetQuality(Byte pi_Percentage)
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    m_pCodecJpeg->SetQuality(pi_Percentage);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
Byte HCDCodecJPEGAlpha::GetQuality() const
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    return m_pCodecJpeg->GetQuality();
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
void HCDCodecJPEGAlpha::SetOptimizeCoding(bool pi_Enable)
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    m_pCodecJpeg->SetOptimizeCoding(pi_Enable);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
bool HCDCodecJPEGAlpha::GetOptimizeCoding() const
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    return m_pCodecJpeg->GetOptimizeCoding();
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
uint32_t HCDCodecJPEGAlpha::CreateTables(void* po_pOutBuffer, uint32_t pi_OutBufferSize)
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    return m_pCodecJpeg->CreateTables(po_pOutBuffer, pi_OutBufferSize);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
void HCDCodecJPEGAlpha::SetProgressiveMode(bool pi_Enable)
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    m_pCodecJpeg->SetProgressiveMode(pi_Enable);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
bool HCDCodecJPEGAlpha::IsProgressive() const
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    return m_pCodecJpeg->IsProgressive();
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
void HCDCodecJPEGAlpha::ReadHeader(const void* pi_pInData, size_t pi_InDataSize)
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    m_pCodecJpeg->ReadHeader(pi_pInData, pi_InDataSize);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
void HCDCodecJPEGAlpha::SetAbbreviateMode(bool pi_Enable)
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    m_pCodecJpeg->SetAbbreviateMode(pi_Enable);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
bool HCDCodecJPEGAlpha::GetAbbreviateMode() const
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    return m_pCodecJpeg->GetAbbreviateMode();
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
void HCDCodecJPEGAlpha::CopyTablesFromDecoderToEncoder()
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    m_pCodecJpeg->CopyTablesFromDecoderToEncoder();
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
void HCDCodecJPEGAlpha::SetQuantizationTable(int pi_Slot, const unsigned int* pi_pTable)
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    m_pCodecJpeg->SetQuantizationTable(pi_Slot, pi_pTable);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
HCDCodecIJG::SubsamplingModes HCDCodecJPEGAlpha::GetSubsamplingMode() const
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    return m_pCodecJpeg->GetSubsamplingMode();
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
void HCDCodecJPEGAlpha::SetSubsamplingMode(HCDCodecIJG::SubsamplingModes pi_Mode)
    {
    HPRECONDITION(m_pCodecJpeg != 0);

    m_pCodecJpeg->SetSubsamplingMode(pi_Mode);
    }