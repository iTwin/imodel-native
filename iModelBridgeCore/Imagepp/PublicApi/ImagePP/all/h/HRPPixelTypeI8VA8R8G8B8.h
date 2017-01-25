//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI8VA8R8G8B8.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeI8VA8R8G8B8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeI8VA8R8G8B8 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_I8VA8R8G8B8)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeI8VA8R8G8B8  ();
    IMAGEPP_EXPORT                 HRPPixelTypeI8VA8R8G8B8  (const HRPPixelPalette& pi_Palette);

    IMAGEPP_EXPORT                 HRPPixelTypeI8VA8R8G8B8(const HRPPixelTypeI8VA8R8G8B8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeI8VA8R8G8B8();

    HPMPersistentObject*
    Clone() const override;

    virtual size_t  CountPixelRawDataBits() const {
        return 16;
        };

    uint16_t CountValueBits() const override;

    void    SetDefaultCompositeValue(const void* pi_pValue) override;

    HRPQuantizedPalette*
    CreateQuantizedPalette(uint32_t pi_MaxEntries) const override;
    uint32_t FindNearestEntryInPalette(const void* pi_pValue) const override;


protected:

    const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const override;

    const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const override;

private:

    };
END_IMAGEPP_NAMESPACE
