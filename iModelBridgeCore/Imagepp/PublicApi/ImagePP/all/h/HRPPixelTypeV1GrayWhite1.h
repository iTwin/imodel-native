//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV1GrayWhite1.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV1GrayWhite1
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"
#include "HRPPixelType1BitInterface.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV1GrayWhite1 : public HRPPixelType,
    public HRPPixelType1BitInterface
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V1GrayWhite1)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV1GrayWhite1();

    IMAGEPP_EXPORT                 HRPPixelTypeV1GrayWhite1(const HRPPixelTypeV1GrayWhite1& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV1GrayWhite1();

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
