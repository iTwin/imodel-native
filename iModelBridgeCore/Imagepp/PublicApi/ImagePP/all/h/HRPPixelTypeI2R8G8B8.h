//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI2R8G8B8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeI2R8G8B8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"

class HRPPixelTypeI2R8G8B8 : public HRPPixelTypeRGB
    {

    HPM_DECLARE_CLASS_DLL(_HDLLg,  1152)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeI2R8G8B8    ();
    _HDLLg                 HRPPixelTypeI2R8G8B8    (const HRPPixelPalette& pi_Palette);

    _HDLLg                 HRPPixelTypeI2R8G8B8(const HRPPixelTypeI2R8G8B8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeI2R8G8B8();

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
