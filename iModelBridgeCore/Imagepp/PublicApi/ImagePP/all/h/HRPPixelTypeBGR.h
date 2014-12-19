//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeBGR.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeBGR
//-----------------------------------------------------------------------------
// ...
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelType.h"

class HRPPixelTypeBGR : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1024)

public:

    virtual     ~HRPPixelTypeBGR();


protected:


    // Primary methods
    HRPPixelTypeBGR();
    HRPPixelTypeBGR(unsigned short pi_BitsBlue,
                    unsigned short pi_BitsGreen,
                    unsigned short pi_BitsRed,
                    unsigned short pi_BitsExtra,
                    unsigned short pi_IndexBits,
                    bool   pi_IsBitsExtraAlpha = false);

    HRPPixelTypeBGR(unsigned short             pi_BitsBlue,
                    unsigned short             pi_BitsGreen,
                    unsigned short             pi_BitsRed,
                    unsigned short             pi_BitsAlpha,
                    HRPChannelType::ChannelRole pi_RoleChannel4,
                    HRPChannelType::DataType    pi_DataTypeChannel4,
                    unsigned short             pi_BitsChannel4,
                    unsigned short             pi_IndexBits);

    HRPPixelTypeBGR(const HRPPixelTypeBGR& pi_rObj);

private:
    };

