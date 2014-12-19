//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV16B5G5R5.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV16B5G5R5
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelType.h"

class HRPPixelTypeV16B5G5R5 : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1049)

public:

    // Primary methods
    HRPPixelTypeV16B5G5R5();

    HRPPixelTypeV16B5G5R5(const HRPPixelTypeV16B5G5R5& pi_rObj);

    virtual         ~HRPPixelTypeV16B5G5R5();

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

