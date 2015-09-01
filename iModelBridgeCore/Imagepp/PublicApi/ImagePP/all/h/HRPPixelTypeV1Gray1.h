//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV1Gray1.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV1Gray1
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeGray.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"
#include "HRPPixelType1BitInterface.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelConverter;

class HRPPixelTypeV1Gray1 : public HRPPixelTypeGray,
    public HRPPixelType1BitInterface
    {
    // HCHK HL
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V1Gray1)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV1Gray1();

    IMAGEPP_EXPORT                 HRPPixelTypeV1Gray1(const HRPPixelTypeV1Gray1& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV1Gray1();

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
END_IMAGEPP_NAMESPACE
