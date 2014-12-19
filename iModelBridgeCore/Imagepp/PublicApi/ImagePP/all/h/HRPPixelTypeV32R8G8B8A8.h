//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV32R8G8B8A8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"

class HRPPixelTypeV32R8G8B8A8 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1031)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeV32R8G8B8A8();
    _HDLLg                 HRPPixelTypeV32R8G8B8A8(const HRPPixelTypeV32R8G8B8A8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV32R8G8B8A8();

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


