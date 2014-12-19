//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV32PR8PG8PB8A8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV32PR8PG8PB8A8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

class HRPPixelTypeV32PR8PG8PB8A8 : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1034)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeV32PR8PG8PB8A8();
    _HDLLg                 HRPPixelTypeV32PR8PG8PB8A8(const HRPPixelTypeV32PR8PG8PB8A8& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV32PR8PG8PB8A8();

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


