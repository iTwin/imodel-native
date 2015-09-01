//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecPackBits.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecPackBits
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodecPackBits.h>

#define HCD_CODEC_NAME L"PackBits"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecPackBits::HCDCodecPackBits()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDCodecPackBits::HCDCodecPackBits(size_t   pi_Width,
                                   size_t   pi_Height,
                                   size_t   pi_BitsPerPixel)
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
HCDCodecPackBits::HCDCodecPackBits(const HCDCodecPackBits& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecPackBits::~HCDCodecPackBits()
    {
    }



//-----------------------------------------------------------------------------
// public
// GetEstimatedCompressionRatio
//-----------------------------------------------------------------------------
double HCDCodecPackBits::GetEstimatedCompressionRatio() const
    {
    return 0.3;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecPackBits::GetSubsetMaxCompressedSize() const
    {
    size_t NbBytes;

    NbBytes = (((GetSubsetWidth() * GetBitsPerPixel()) + 7) / 8) * GetSubsetHeight();

    // at most, 1 extra byte was add at every 128 bytes
    return (NbBytes + ((NbBytes / 128) + 1));
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecPackBits::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    return true;
    }
