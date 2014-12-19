//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV1Gray1.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV1Gray1
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeGray.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"
#include "HRPPixelType1BitInterface.h"

class HRPPixelConverter;

class HRPPixelTypeV1Gray1 : public HRPPixelTypeGray,
    public HRPPixelType1BitInterface
    {
    // HCHK HL
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1246)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeV1Gray1();

    _HDLLg                 HRPPixelTypeV1Gray1(const HRPPixelTypeV1Gray1& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV1Gray1();

    virtual HPMPersistentObject*
    Clone() const;

    virtual unsigned short CountValueBits() const;

    // Not a const method because it returns a non const pointer to this.
    virtual HRPPixelType1BitInterface*
    Get1BitInterface();

protected:

    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;
    };
