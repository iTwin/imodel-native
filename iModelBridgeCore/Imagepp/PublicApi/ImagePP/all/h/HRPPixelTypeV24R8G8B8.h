//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV24RGB
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV24R8G8B8 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V24R8G8B8)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV24R8G8B8();
    IMAGEPP_EXPORT                 HRPPixelTypeV24R8G8B8(const HRPPixelTypeV24R8G8B8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV24R8G8B8();

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
