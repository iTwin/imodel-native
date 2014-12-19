//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV8Gray8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV8Gray8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeGray.h"

class HRPPixelTypeV8Gray8 : public HRPPixelTypeGray
    {
public:

    HPM_DECLARE_CLASS_DLL(_HDLLg,  1248)

    // Primary methods
    _HDLLg                 HRPPixelTypeV8Gray8();
    _HDLLg                 HRPPixelTypeV8Gray8(const HRPPixelTypeV8Gray8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV8Gray8();

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
