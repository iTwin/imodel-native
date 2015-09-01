//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeBGR.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeBGR
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPPixelTypeBGR.h>
#include <Imagepp/all/h/HRPChannelOrgBGR.h>

HPM_REGISTER_ABSTRACT_CLASS(HRPPixelTypeBGR, HRPPixelType)


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HRPPixelTypeBGR::HRPPixelTypeBGR()
    : HRPPixelType(HRPChannelOrgBGR(2, 1, 0, 3,
                                    HRPChannelType::UNUSED,
                                    HRPChannelType::VOID_CH,
                                    0),
                   0)
    {
    }


//-----------------------------------------------------------------------------
// Constructor for straight BGR
//-----------------------------------------------------------------------------
HRPPixelTypeBGR::HRPPixelTypeBGR(unsigned short pi_BitsBlue,
                                 unsigned short pi_BitsGreen,
                                 unsigned short pi_BitsRed,
                                 unsigned short pi_BitsExtra,
                                 unsigned short pi_IndexBits,
                                 bool   pi_IsBitsExtraAlpha)
    : HRPPixelType(HRPChannelOrgBGR(pi_BitsBlue,
                                    pi_BitsGreen,
                                    pi_BitsRed,
                                    pi_BitsExtra,
                                    HRPChannelType::UNUSED,
                                    HRPChannelType::VOID_CH,
                                    0,
                                    pi_IsBitsExtraAlpha),
                   pi_IndexBits)
    {
    }

//-----------------------------------------------------------------------------
// Constructor for BGR channels plus a 4th channel.
// Useful for creating BGRA or BGRU pixel types.
//-----------------------------------------------------------------------------
HRPPixelTypeBGR::HRPPixelTypeBGR(unsigned short             pi_BitsBlue,
                                 unsigned short             pi_BitsGreen,
                                 unsigned short             pi_BitsRed,
                                 unsigned short             pi_BitsAlpha,
                                 HRPChannelType::ChannelRole pi_RoleChannel4,
                                 HRPChannelType::DataType    pi_DataTypeChannel4,
                                 unsigned short             pi_BitsChannel4,
                                 unsigned short             pi_IndexBits)
    : HRPPixelType(HRPChannelOrgBGR(pi_BitsBlue,
                                    pi_BitsGreen,
                                    pi_BitsRed,
                                    pi_BitsAlpha,
                                    pi_RoleChannel4,
                                    pi_DataTypeChannel4,
                                    pi_BitsChannel4),
                   pi_IndexBits)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeBGR::HRPPixelTypeBGR(const HRPPixelTypeBGR& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeBGR::~HRPPixelTypeBGR()
    {
    }

