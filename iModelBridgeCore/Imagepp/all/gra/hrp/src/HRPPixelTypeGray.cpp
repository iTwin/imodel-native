//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeGray.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeGray
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPPixelTypeGray.h>
#include <Imagepp/all/h/HRPChannelOrgGray.h>



HPM_REGISTER_ABSTRACT_CLASS(HRPPixelTypeGray, HRPPixelType)

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HRPPixelTypeGray::HRPPixelTypeGray ()
    : HRPPixelType(HRPChannelOrgGray(8,
                                     HRPChannelType::UNUSED,
                                     HRPChannelType::VOID_CH,
                                     0),
                   0)
    {
    }


//-----------------------------------------------------------------------------
// Constructor for straight Gray
//-----------------------------------------------------------------------------
HRPPixelTypeGray::HRPPixelTypeGray(unsigned short pi_BitsGray,
                                   unsigned short pi_IndexBits)
    : HRPPixelType(HRPChannelOrgGray(pi_BitsGray,
                                     HRPChannelType::UNUSED,
                                     HRPChannelType::VOID_CH,
                                     0),
                   pi_IndexBits)
    {
    }

//-----------------------------------------------------------------------------
// Constructor for Gray channels plus a 4th channel.
// Useful for creating GrayA or GrayU pixel types.
//-----------------------------------------------------------------------------
HRPPixelTypeGray::HRPPixelTypeGray(unsigned short             pi_BitsGray,
                                   HRPChannelType::ChannelRole pi_RoleChannel4,
                                   HRPChannelType::DataType    pi_DataTypeChannel4,
                                   unsigned short             pi_BitsChannel4,
                                   unsigned short             pi_IndexBits)
    : HRPPixelType(HRPChannelOrgGray(pi_BitsGray,
                                     pi_RoleChannel4,
                                     pi_DataTypeChannel4,
                                     pi_BitsChannel4),
                   pi_IndexBits)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeGray::HRPPixelTypeGray(const HRPPixelTypeGray& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeGray::~HRPPixelTypeGray()
    {
    }

