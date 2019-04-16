//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV24PhotoYCC
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV24PhotoYCC : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V24PhotoYCC)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV24PhotoYCC();

    IMAGEPP_EXPORT                 HRPPixelTypeV24PhotoYCC(const HRPPixelTypeV24PhotoYCC& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV24PhotoYCC();

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


