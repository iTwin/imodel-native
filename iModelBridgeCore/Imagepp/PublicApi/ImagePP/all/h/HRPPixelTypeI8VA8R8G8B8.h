//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI8VA8R8G8B8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    virtual HPMPersistentObject*
    Clone() const;

    virtual size_t  CountPixelRawDataBits() const {
        return 16;
        };

    virtual unsigned short CountValueBits() const;

    virtual void    SetDefaultCompositeValue(const void* pi_pValue);

    virtual HRPQuantizedPalette*
    CreateQuantizedPalette(uint32_t pi_MaxEntries) const;
    virtual uint32_t FindNearestEntryInPalette(const void* pi_pValue) const;


protected:

    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;

private:

    };
END_IMAGEPP_NAMESPACE
