//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeI2R8G8B8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeI2R8G8B8 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_I2R8G8B8)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeI2R8G8B8    ();
    IMAGEPP_EXPORT                 HRPPixelTypeI2R8G8B8    (const HRPPixelPalette& pi_Palette);

    IMAGEPP_EXPORT                 HRPPixelTypeI2R8G8B8(const HRPPixelTypeI2R8G8B8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeI2R8G8B8();

    HPMPersistentObject*
    Clone() const override;

    uint16_t CountValueBits() const override;

protected:

    const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const override;

    const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const override;

private:

    };
END_IMAGEPP_NAMESPACE
