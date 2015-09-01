//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI8R8G8B8Mask.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeI8R8G8B8Mask
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>

/*----------------------------------------------------------------------------+
|Class HRPPixelTypeI8R8G8B8Mask
|
| This is a special pixel type that creates a mask that will be used
| by DC to edit raster data.
+----------------------------------------------------------------------------*/
BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeI8R8G8B8Mask : public HRPPixelTypeI8R8G8B8
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_I8R8G8B8Mask)

public:
    /*----------------------------------------------------------------------------+
    |    Public member Functions
    +----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT                     HRPPixelTypeI8R8G8B8Mask();
    IMAGEPP_EXPORT                     HRPPixelTypeI8R8G8B8Mask(const HRPPixelPalette& pi_Palette);
    IMAGEPP_EXPORT                     HRPPixelTypeI8R8G8B8Mask(const HRPPixelTypeI8R8G8B8Mask& pi_rObj);

    IMAGEPP_EXPORT virtual             ~HRPPixelTypeI8R8G8B8Mask();

    virtual HPMPersistentObject*
    Clone() const;

protected:
    /*----------------------------------------------------------------------------+
    |    Protected member Functions
    +----------------------------------------------------------------------------*/
    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;
    };
END_IMAGEPP_NAMESPACE


