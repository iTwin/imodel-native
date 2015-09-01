//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV8GrayWhite8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV8GrayWhite8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"


BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV8GrayWhite8 : public HRPPixelType
    {
public:

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V8GrayWhite8)

    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV8GrayWhite8();
    IMAGEPP_EXPORT                 HRPPixelTypeV8GrayWhite8(const HRPPixelTypeV8GrayWhite8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV8GrayWhite8();

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
END_IMAGEPP_NAMESPACE
