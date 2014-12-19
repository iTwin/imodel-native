//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI4R8G8B8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeI4R8G8B8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

class HRPPixelConverter;

class HRPPixelTypeI4R8G8B8 : public HRPPixelTypeRGB
    {

    HPM_DECLARE_CLASS_DLL(_HDLLg,  1153)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeI4R8G8B8    ();
    _HDLLg                 HRPPixelTypeI4R8G8B8    (const HRPPixelPalette& pi_Palette);

    _HDLLg                 HRPPixelTypeI4R8G8B8(const HRPPixelTypeI4R8G8B8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeI4R8G8B8();

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
