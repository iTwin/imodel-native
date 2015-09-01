//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecCCITT.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecCCITT
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecCCITT.h>

#define HCDCODECCCITT_WORSTCASEINFLATION            6

// After some tests, 20:1 looks more realistic than 50:1
#define HCDCODECCCITT_ESTIMATEDCOMPRESSIONRATIO     0.05            // 20:1
//#define HCDCODECCCITT_ESTIMATEDCOMPRESSIONRATIO     0.02          // 50:1


#define HCD_CODEC_NAME L"CCITT"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecCCITT::HCDCodecCCITT()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    m_bitrevtable = false;
    m_photometric = CCITT_PHOTOMETRIC_MINISWHITE;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecCCITT::HCDCodecCCITT( size_t pi_Width,
                              size_t pi_Height)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    1)
    {
    m_bitrevtable = false;
    m_photometric = CCITT_PHOTOMETRIC_MINISWHITE;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecCCITT::HCDCodecCCITT(const HCDCodecCCITT& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    m_photometric = pi_rObj.m_photometric;
    m_bitrevtable = pi_rObj.m_bitrevtable;
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecCCITT::~HCDCodecCCITT()
    {
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecCCITT::IsBitsPerPixelSupported(size_t pi_BitsPerPixel) const
    {
    if(pi_BitsPerPixel == 1)
        return true;
    else
        return false;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecCCITT::GetSubsetMaxCompressedSize() const
    {
    return ((GetSubsetWidth() + 7) / 8 * GetSubsetHeight() * HCDCODECCCITT_WORSTCASEINFLATION);
    }

//-----------------------------------------------------------------------------
// public
// GetEstimatedCompressionRatio
//-----------------------------------------------------------------------------
double HCDCodecCCITT::GetEstimatedCompressionRatio() const
    {
    return HCDCODECCCITT_ESTIMATEDCOMPRESSIONRATIO;
    }

//-----------------------------------------------------------------------------
// public
// SetPhotometric
//-----------------------------------------------------------------------------
void HCDCodecCCITT::SetPhotometric(unsigned short pi_Photo)
    {
    m_photometric = pi_Photo;
    }

//-----------------------------------------------------------------------------
// public
// GetPhotometric
//-----------------------------------------------------------------------------
unsigned short HCDCodecCCITT::GetPhotometric() const
    {
    return (m_photometric);
    }

//-----------------------------------------------------------------------------
// public
// SetBitRevTable
//-----------------------------------------------------------------------------
void HCDCodecCCITT::SetBitRevTable(bool pi_Reverse)
    {
    m_bitrevtable = pi_Reverse;
    }
