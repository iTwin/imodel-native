//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV32B8G8R8X8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV32B8G8R8X8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeBGR.h"

class HRPPixelTypeV32B8G8R8X8 : public HRPPixelTypeBGR
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1375)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeV32B8G8R8X8();
    _HDLLg                 HRPPixelTypeV32B8G8R8X8(const HRPPixelTypeV32B8G8R8X8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV32B8G8R8X8();

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


