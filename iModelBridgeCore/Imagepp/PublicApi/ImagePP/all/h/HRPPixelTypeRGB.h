//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeRGB.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeRGB
//-----------------------------------------------------------------------------
// ...
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeRGB : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_BaseRGB)

public:


    virtual     ~HRPPixelTypeRGB();

    HRPQuantizedPalette*
    CreateQuantizedPalette(uint32_t pi_MaxEntries) const override;

    uint32_t
    FindNearestEntryInPalette(const void* pi_pValue) const override;

protected:

    // Primary methods
    HRPPixelTypeRGB ();
    HRPPixelTypeRGB(uint16_t pi_BitsRed,
                    uint16_t pi_BitsGreen,
                    uint16_t pi_BitsBlue,
                    uint16_t pi_BitsExtra,
                    uint16_t pi_IndexBits,
                    bool   pi_IsBitsExtraAlpha = true);
    HRPPixelTypeRGB(uint16_t pi_BitsRed,
                    uint16_t pi_BitsGreen,
                    uint16_t pi_BitsBlue,
                    uint16_t pi_BitsAlpha,
                    uint16_t pi_IndexBits,
                    uint16_t pi_IndexBitsRed,
                    uint16_t pi_IndexBitsGreen,
                    uint16_t pi_IndexBitsBlue,
                    uint16_t pi_IndexBitsAlpha);
    HRPPixelTypeRGB(uint16_t             pi_BitsRed,
                    uint16_t             pi_BitsGreen,
                    uint16_t             pi_BitsBlue,
                    uint16_t             pi_BitsAlpha,
                    HRPChannelType::ChannelRole pi_RoleChannel5,
                    HRPChannelType::DataType    pi_DataTypeChannel5,
                    uint16_t             pi_BitsChannel5,
                    uint16_t             pi_IndexBits);
    HRPPixelTypeRGB(const HRPPixelTypeRGB& pi_rObj);

private:
    };
END_IMAGEPP_NAMESPACE

