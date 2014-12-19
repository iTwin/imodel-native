//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV32A8R8G8B8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV32A8R8G8B8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

class HRPPixelTypeV32A8R8G8B8 : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1079)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeV32A8R8G8B8();
    _HDLLg                 HRPPixelTypeV32A8R8G8B8(const HRPPixelTypeV32A8R8G8B8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV32A8R8G8B8();

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


