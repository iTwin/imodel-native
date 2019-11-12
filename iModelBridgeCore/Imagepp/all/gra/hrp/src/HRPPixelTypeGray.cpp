//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeGray
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRPPixelTypeGray.h>
#include <ImagePP/all/h/HRPChannelOrgGray.h>



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
HRPPixelTypeGray::HRPPixelTypeGray(uint16_t pi_BitsGray,
                                   uint16_t pi_IndexBits)
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
HRPPixelTypeGray::HRPPixelTypeGray(uint16_t             pi_BitsGray,
                                   HRPChannelType::ChannelRole pi_RoleChannel4,
                                   HRPChannelType::DataType    pi_DataTypeChannel4,
                                   uint16_t             pi_BitsChannel4,
                                   uint16_t             pi_IndexBits)
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

