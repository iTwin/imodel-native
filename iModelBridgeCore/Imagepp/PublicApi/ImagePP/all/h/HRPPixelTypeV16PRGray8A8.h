//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV16PRGray8A8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV16PRGray8A8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

class HRPPixelTypeV16PRGray8A8 : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1053)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeV16PRGray8A8();

    _HDLLg                 HRPPixelTypeV16PRGray8A8(const HRPPixelTypeV16PRGray8A8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV16PRGray8A8();

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
