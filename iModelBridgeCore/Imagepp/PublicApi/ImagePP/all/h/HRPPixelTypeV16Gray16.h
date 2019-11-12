//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV16Gray16
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeGray.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV16Gray16 : public HRPPixelTypeGray
    {
public:

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V16Gray16)

    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV16Gray16();
    IMAGEPP_EXPORT                 HRPPixelTypeV16Gray16(const HRPPixelTypeV16Gray16& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV16Gray16();

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
