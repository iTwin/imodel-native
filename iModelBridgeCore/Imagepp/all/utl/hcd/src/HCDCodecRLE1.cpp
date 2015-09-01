//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecRLE1.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecRLE1
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecRLE1.h>

#define HCD_CODEC_NAME L"RLE1bit"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecRLE1::HCDCodecRLE1()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecRLE1::HCDCodecRLE1(size_t pi_Width,
                           size_t pi_Height)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    1)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecRLE1::HCDCodecRLE1(const HCDCodecRLE1& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecRLE1::~HCDCodecRLE1()
    {
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecRLE1::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    if(pi_Bits == 1)
        return true;
    else
        return false;
    }

//-----------------------------------------------------------------------------
// public
// GetEstimatedCompressionRatio
//-----------------------------------------------------------------------------
double HCDCodecRLE1::GetEstimatedCompressionRatio() const
    {
    // After some tests, 3.33:1 looks more realistic than 6.66:1
    // return 0.15;
    return 0.3;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecRLE1::GetSubsetMaxCompressedSize() const
    {
    return ((GetSubsetWidth() + 2) * 2 * GetSubsetHeight());
    }
