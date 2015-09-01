//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecFPXSingleColor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecFPXSingleColor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecFPXSingleColor.h>


//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecFPXSingleColor::HCDCodecFPXSingleColor()
    : HCDCodecSingleColor()
    {
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecFPXSingleColor::HCDCodecFPXSingleColor(size_t pi_Width,
                                               size_t pi_Height,
                                               size_t pi_BitsPerPixel)
    : HCDCodecSingleColor(pi_Width, pi_Height, pi_BitsPerPixel)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecFPXSingleColor::HCDCodecFPXSingleColor(const HCDCodecFPXSingleColor& pi_rObj)
    : HCDCodecSingleColor(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecFPXSingleColor::~HCDCodecFPXSingleColor()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecFPXSingleColor::Clone() const
    {
    return new HCDCodecFPXSingleColor(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecFPXSingleColor::CompressSubset(const void* pi_pInData,
                                              size_t pi_InDataSize,
                                              void* po_pOutBuffer,
                                              size_t pi_OutBufferSize)
    {
    HASSERT(!"HCDCodecFPXSingleColor::CompressSubset not supported");
    // by default, return 0 byte processed
    return 0;
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecFPXSingleColor::DecompressSubset(const void* pi_pInData,
                                                size_t pi_InDataSize,
                                                void* po_pOutBuffer,
                                                size_t pi_OutBufferSize)
    {
    size_t BytesPerPixel = GetBitsPerPixel() / 8;

    size_t TileSize = GetSubsetWidth() * GetSubsetHeight() * BytesPerPixel;

    Byte* pOut = (Byte*)po_pOutBuffer;
    Byte* pIn = (Byte*)pi_pInData;

    if (BytesPerPixel == 1)
        memset(po_pOutBuffer, *pIn, TileSize);
    else if (BytesPerPixel == 3)
        {
        for (size_t i = 0; i < TileSize; i += BytesPerPixel)
            {
            pOut[i    ] = pIn[0];
            pOut[i + 1] = pIn[1];
            pOut[i + 2] = pIn[2];
            }
        }
    else if (BytesPerPixel == 4)
        {
        for (size_t i = 0; i < TileSize; i += BytesPerPixel)
            {
            pOut[i    ] = pIn[0];
            pOut[i + 1] = pIn[1];
            pOut[i + 2] = pIn[2];
            pOut[i + 3] = pIn[3];
            }
        }

    return TileSize;
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecFPXSingleColor::HasLineAccess() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixel
//-----------------------------------------------------------------------------
bool HCDCodecFPXSingleColor::IsBitsPerPixelSupported(size_t pi_BitsPerPixel) const
    {
    return true;
    }