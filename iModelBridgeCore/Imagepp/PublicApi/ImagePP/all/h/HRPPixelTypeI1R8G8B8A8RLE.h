//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeI1R8G8B8A8RLE
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelType1bitInterface.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeI1R8G8B8A8RLE : public HRPPixelTypeRGB,
    public HRPPixelType1BitInterface
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_I1R8G8B8A8RLE)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeI1R8G8B8A8RLE    ();
    IMAGEPP_EXPORT                 HRPPixelTypeI1R8G8B8A8RLE    (const HRPPixelPalette& pi_Palette);

    IMAGEPP_EXPORT                 HRPPixelTypeI1R8G8B8A8RLE(const HRPPixelTypeI1R8G8B8A8RLE& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeI1R8G8B8A8RLE();

    HPMPersistentObject*
    Clone() const override;

    uint16_t CountValueBits() const override;

    // Not a const method because it returns a non const pointer to this.
    HRPPixelType1BitInterface*
    Get1BitInterface() override;

protected:

    const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const override;

    const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const override;
    };
END_IMAGEPP_NAMESPACE
