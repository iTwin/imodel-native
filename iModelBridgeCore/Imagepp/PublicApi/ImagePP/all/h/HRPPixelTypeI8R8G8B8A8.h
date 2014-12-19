//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI8R8G8B8A8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeI8R8G8B8A8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"

class HRPPixelTypeI8R8G8B8A8 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1037)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeI8R8G8B8A8  ();
    _HDLLg                 HRPPixelTypeI8R8G8B8A8  (const HRPPixelPalette& pi_Palette);

    _HDLLg                 HRPPixelTypeI8R8G8B8A8(const HRPPixelTypeI8R8G8B8A8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeI8R8G8B8A8();

    virtual HPMPersistentObject*
    Clone() const;

    virtual unsigned short CountValueBits() const;

protected:

    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;

private:

    };
