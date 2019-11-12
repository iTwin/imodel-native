//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV32B8G8R8X8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeBGR.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV32B8G8R8X8 : public HRPPixelTypeBGR
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V32B8G8R8X8)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV32B8G8R8X8();
    IMAGEPP_EXPORT                 HRPPixelTypeV32B8G8R8X8(const HRPPixelTypeV32B8G8R8X8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV32B8G8R8X8();

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


