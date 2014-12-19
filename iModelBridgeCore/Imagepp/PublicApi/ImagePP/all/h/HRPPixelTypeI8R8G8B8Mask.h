//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI8R8G8B8Mask.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
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
class HRPPixelTypeI8R8G8B8Mask : public HRPPixelTypeI8R8G8B8
    {

    HPM_DECLARE_CLASS_DLL(_HDLLg,  1278)

public:
    /*----------------------------------------------------------------------------+
    |    Public member Functions
    +----------------------------------------------------------------------------*/
    _HDLLg                     HRPPixelTypeI8R8G8B8Mask();
    _HDLLg                     HRPPixelTypeI8R8G8B8Mask(const HRPPixelPalette& pi_Palette);
    _HDLLg                     HRPPixelTypeI8R8G8B8Mask(const HRPPixelTypeI8R8G8B8Mask& pi_rObj);

    _HDLLg virtual             ~HRPPixelTypeI8R8G8B8Mask();

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


