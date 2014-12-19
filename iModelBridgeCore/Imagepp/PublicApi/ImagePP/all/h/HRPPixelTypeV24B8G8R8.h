//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV24B8G8R8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV24R8G8B8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeBGR.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

class HRPPixelTypeV24B8G8R8 : public HRPPixelTypeBGR
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1023)

public:
    // Primary methods
    _HDLLg                HRPPixelTypeV24B8G8R8();

    _HDLLg                 HRPPixelTypeV24B8G8R8(const HRPPixelTypeV24B8G8R8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV24B8G8R8();

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
