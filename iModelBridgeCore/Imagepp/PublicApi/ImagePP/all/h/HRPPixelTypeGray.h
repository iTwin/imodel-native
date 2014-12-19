//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeGray.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeGray
//-----------------------------------------------------------------------------
// ...
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelType.h"

class HRPPixelTypeGray : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1020)

public:

    virtual         ~HRPPixelTypeGray();


protected:


    // Primary methods
    HRPPixelTypeGray ();
    HRPPixelTypeGray(unsigned short pi_BitsGray,
                     unsigned short pi_IndexBits);
    HRPPixelTypeGray(unsigned short             pi_BitsGray,
                     HRPChannelType::ChannelRole pi_RoleChannel4,
                     HRPChannelType::DataType    pi_DataTypeChannel4,
                     unsigned short             pi_BitsChannel4,
                     unsigned short             pi_IndexBits);
    HRPPixelTypeGray(const HRPPixelTypeGray& pi_rObj);
private:
    };

