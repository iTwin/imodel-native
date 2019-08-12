//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV96R32G32B32
//-----------------------------------------------------------------------------


#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV96R32G32B32 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V96R32G32B32)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV96R32G32B32();
    IMAGEPP_EXPORT                 HRPPixelTypeV96R32G32B32(const HRPPixelTypeV96R32G32B32& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV96R32G32B32();

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
