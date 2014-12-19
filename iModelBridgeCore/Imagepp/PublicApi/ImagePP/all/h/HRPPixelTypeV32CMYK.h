//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV32CMYK.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV32CMYK
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

class HRPPixelTypeV32CMYK : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1289)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeV32CMYK();

    _HDLLg                 HRPPixelTypeV32CMYK(const HRPPixelTypeV32CMYK& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV32CMYK();

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


