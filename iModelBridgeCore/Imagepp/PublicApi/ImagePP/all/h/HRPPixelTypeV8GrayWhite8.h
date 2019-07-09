//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV8GrayWhite8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"


BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV8GrayWhite8 : public HRPPixelType
    {
public:

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V8GrayWhite8)

    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV8GrayWhite8();
    IMAGEPP_EXPORT                 HRPPixelTypeV8GrayWhite8(const HRPPixelTypeV8GrayWhite8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV8GrayWhite8();

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
