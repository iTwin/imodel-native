//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI4R8G8B8A8.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeI4R8G8B8A8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelConverter;

class HRPPixelTypeI4R8G8B8A8 : public HRPPixelTypeRGB
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_I4R8G8B8A8)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeI4R8G8B8A8    ();
    IMAGEPP_EXPORT                 HRPPixelTypeI4R8G8B8A8    (const HRPPixelPalette& pi_Palette);

    IMAGEPP_EXPORT                 HRPPixelTypeI4R8G8B8A8(const HRPPixelTypeI4R8G8B8A8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeI4R8G8B8A8();

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
