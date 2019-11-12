//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeGray
//-----------------------------------------------------------------------------
// ...
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeGray : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_Gray)

public:

    virtual         ~HRPPixelTypeGray();


protected:


    // Primary methods
    HRPPixelTypeGray ();
    HRPPixelTypeGray(uint16_t pi_BitsGray,
                     uint16_t pi_IndexBits);
    HRPPixelTypeGray(uint16_t             pi_BitsGray,
                     HRPChannelType::ChannelRole pi_RoleChannel4,
                     HRPChannelType::DataType    pi_DataTypeChannel4,
                     uint16_t             pi_BitsChannel4,
                     uint16_t             pi_IndexBits);
    HRPPixelTypeGray(const HRPPixelTypeGray& pi_rObj);
private:
    };
END_IMAGEPP_NAMESPACE

