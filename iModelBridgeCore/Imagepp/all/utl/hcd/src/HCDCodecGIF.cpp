//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecGIF.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecGIF
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecGIF.h>

#define HCD_CODEC_NAME     L"LZWGif"

// Estimation of the compression ratio (to be adjusted by Dominic if required)
#define HCDCODECGIF_COMPRESSION_RATIO   0.1

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecGIF::HCDCodecGIF()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecGIF::HCDCodecGIF(size_t pi_Width,
                         size_t pi_Height,
                         size_t pi_BitsPerPixel)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    pi_BitsPerPixel)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecGIF::HCDCodecGIF(const HCDCodecGIF& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecGIF::~HCDCodecGIF()
    {
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecGIF::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    bool State;

    if(pi_Bits == 1 || pi_Bits == 8)
        State =  true;
    else
        State =  false;

    return State;
    }

//-----------------------------------------------------------------------------
// public
// GetEstimatedCompressionRatio
//-----------------------------------------------------------------------------
double HCDCodecGIF::GetEstimatedCompressionRatio() const
    {
    return HCDCODECGIF_COMPRESSION_RATIO;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecGIF::GetSubsetMaxCompressedSize() const
    {
    // **** TO VERIFY ***
    return (GetSubsetWidth() * 4 * GetSubsetHeight());
    }
