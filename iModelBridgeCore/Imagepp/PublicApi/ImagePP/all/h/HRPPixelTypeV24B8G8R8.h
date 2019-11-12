//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV24R8G8B8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeBGR.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV24B8G8R8 : public HRPPixelTypeBGR
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V24B8G8R8)

public:
    // Primary methods
    IMAGEPP_EXPORT                HRPPixelTypeV24B8G8R8();

    IMAGEPP_EXPORT                 HRPPixelTypeV24B8G8R8(const HRPPixelTypeV24B8G8R8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV24B8G8R8();

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
