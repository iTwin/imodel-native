//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeBGR
//-----------------------------------------------------------------------------
// ...
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeBGR : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_BGR)

public:

    virtual     ~HRPPixelTypeBGR();


protected:


    // Primary methods
    HRPPixelTypeBGR();
    HRPPixelTypeBGR(uint16_t pi_BitsBlue,
                    uint16_t pi_BitsGreen,
                    uint16_t pi_BitsRed,
                    uint16_t pi_BitsExtra,
                    uint16_t pi_IndexBits,
                    bool   pi_IsBitsExtraAlpha = false);

    HRPPixelTypeBGR(uint16_t             pi_BitsBlue,
                    uint16_t             pi_BitsGreen,
                    uint16_t             pi_BitsRed,
                    uint16_t             pi_BitsAlpha,
                    HRPChannelType::ChannelRole pi_RoleChannel4,
                    HRPChannelType::DataType    pi_DataTypeChannel4,
                    uint16_t             pi_BitsChannel4,
                    uint16_t             pi_IndexBits);

    HRPPixelTypeBGR(const HRPPixelTypeBGR& pi_rObj);

private:
    };
END_IMAGEPP_NAMESPACE

