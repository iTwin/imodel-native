//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV8GrayWhite8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV8GrayWhite8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"


class HRPPixelTypeV8GrayWhite8 : public HRPPixelType
    {
public:

    HPM_DECLARE_CLASS_DLL(_HDLLg,  1249)

    // Primary methods
    _HDLLg                 HRPPixelTypeV8GrayWhite8();
    _HDLLg                 HRPPixelTypeV8GrayWhite8(const HRPPixelTypeV8GrayWhite8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV8GrayWhite8();

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
