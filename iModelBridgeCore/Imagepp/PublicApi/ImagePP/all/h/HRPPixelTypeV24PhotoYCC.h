//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV24PhotoYCC.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV24PhotoYCC
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

class HRPPixelTypeV24PhotoYCC : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1036)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeV24PhotoYCC();

    _HDLLg                 HRPPixelTypeV24PhotoYCC(const HRPPixelTypeV24PhotoYCC& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV24PhotoYCC();

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


