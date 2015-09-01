//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI1R8G8B8RLE.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeI1R8G8B8RLE
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelType1BitInterface.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeI1R8G8B8RLE : public HRPPixelTypeRGB,
    public HRPPixelType1BitInterface
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_I1R8G8B8RLE)

public:
    IMAGEPP_EXPORT                 HRPPixelTypeI1R8G8B8RLE();
    IMAGEPP_EXPORT                 HRPPixelTypeI1R8G8B8RLE(const HRPPixelPalette& pi_Palette);
    IMAGEPP_EXPORT                 HRPPixelTypeI1R8G8B8RLE(const HRPPixelTypeI1R8G8B8RLE& pi_rObj);
    IMAGEPP_EXPORT virtual         ~HRPPixelTypeI1R8G8B8RLE();

    virtual HPMPersistentObject*
    Clone() const;

    virtual unsigned short CountValueBits() const;

    // Not a const method because it returns a non const pointer to this.
    virtual HRPPixelType1BitInterface*
    Get1BitInterface();

protected:

    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;
    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;
    };
END_IMAGEPP_NAMESPACE

