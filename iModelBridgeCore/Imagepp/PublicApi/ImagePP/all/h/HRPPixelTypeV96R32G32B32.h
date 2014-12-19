//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV96R32G32B32.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV96R32G32B32
//-----------------------------------------------------------------------------


#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

class HRPPixelTypeV96R32G32B32 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1288)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeV96R32G32B32();
    _HDLLg                 HRPPixelTypeV96R32G32B32(const HRPPixelTypeV96R32G32B32& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV96R32G32B32();

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
