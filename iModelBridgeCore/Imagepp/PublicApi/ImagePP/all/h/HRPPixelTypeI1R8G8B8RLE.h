//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI1R8G8B8RLE.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeI1R8G8B8RLE
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelType1BitInterface.h"

class HRPPixelTypeI1R8G8B8RLE : public HRPPixelTypeRGB,
    public HRPPixelType1BitInterface
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1785)

public:
    _HDLLg                 HRPPixelTypeI1R8G8B8RLE();
    _HDLLg                 HRPPixelTypeI1R8G8B8RLE(const HRPPixelPalette& pi_Palette);
    _HDLLg                 HRPPixelTypeI1R8G8B8RLE(const HRPPixelTypeI1R8G8B8RLE& pi_rObj);
    _HDLLg virtual         ~HRPPixelTypeI1R8G8B8RLE();

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

