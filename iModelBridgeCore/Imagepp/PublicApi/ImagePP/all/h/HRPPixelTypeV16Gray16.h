//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV16Gray16.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV16Gray16
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeGray.h"

class HRPPixelTypeV16Gray16 : public HRPPixelTypeGray
    {
public:

    HPM_DECLARE_CLASS_DLL(_HDLLg,  1270)

    // Primary methods
    _HDLLg                 HRPPixelTypeV16Gray16();
    _HDLLg                 HRPPixelTypeV16Gray16(const HRPPixelTypeV16Gray16& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV16Gray16();

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
