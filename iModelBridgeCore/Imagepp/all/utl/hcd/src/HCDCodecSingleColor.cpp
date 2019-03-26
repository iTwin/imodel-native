//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecSingleColor.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecSingleColor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HCDCodecSingleColor.h>

#define HCD_CODEC_NAME "Single Color"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecSingleColor::HCDCodecSingleColor()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecSingleColor::HCDCodecSingleColor(size_t pi_Width,
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
HCDCodecSingleColor::HCDCodecSingleColor(const HCDCodecSingleColor& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecSingleColor::~HCDCodecSingleColor()
    {
    }

