//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeRGB.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    virtual HRPQuantizedPalette*
    CreateQuantizedPalette(uint32_t pi_MaxEntries) const;

    virtual uint32_t
    FindNearestEntryInPalette(const void* pi_pValue) const;

protected:

    // Primary methods
    HRPPixelTypeRGB ();
    HRPPixelTypeRGB(unsigned short pi_BitsRed,
                    unsigned short pi_BitsGreen,
                    unsigned short pi_BitsBlue,
                    unsigned short pi_BitsExtra,
                    unsigned short pi_IndexBits,
                    bool   pi_IsBitsExtraAlpha = true);
    HRPPixelTypeRGB(unsigned short pi_BitsRed,
                    unsigned short pi_BitsGreen,
                    unsigned short pi_BitsBlue,
                    unsigned short pi_BitsAlpha,
                    unsigned short pi_IndexBits,
                    unsigned short pi_IndexBitsRed,
                    unsigned short pi_IndexBitsGreen,
                    unsigned short pi_IndexBitsBlue,
                    unsigned short pi_IndexBitsAlpha);
    HRPPixelTypeRGB(unsigned short             pi_BitsRed,
                    unsigned short             pi_BitsGreen,
                    unsigned short             pi_BitsBlue,
                    unsigned short             pi_BitsAlpha,
                    HRPChannelType::ChannelRole pi_RoleChannel5,
                    HRPChannelType::DataType    pi_DataTypeChannel5,
                    unsigned short             pi_BitsChannel5,
                    unsigned short             pi_IndexBits);
    HRPPixelTypeRGB(const HRPPixelTypeRGB& pi_rObj);

private:
    };
END_IMAGEPP_NAMESPACE

