//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecDeflate.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecDeflate
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodecDeflate.h>

#define HCD_CODEC_NAME L"Deflate"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecDeflate::HCDCodecDeflate()
    : HCDCodecVector(HCD_CODEC_NAME)
    {
    }


//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HCDCodecDeflate::HCDCodecDeflate(size_t pi_DataSize)
    : HCDCodecVector(HCD_CODEC_NAME,
                     pi_DataSize)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecDeflate::HCDCodecDeflate(const HCDCodecDeflate& pi_rObj)
    : HCDCodecVector(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecDeflate::~HCDCodecDeflate()
    {
    }

