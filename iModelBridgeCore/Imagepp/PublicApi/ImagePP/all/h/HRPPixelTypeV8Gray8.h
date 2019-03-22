//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV8Gray8.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV8Gray8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeGray.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV8Gray8 : public HRPPixelTypeGray
    {
public:

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V8Gray8)

    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV8Gray8();
    IMAGEPP_EXPORT                 HRPPixelTypeV8Gray8(const HRPPixelTypeV8Gray8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV8Gray8();

    HPMPersistentObject*
    Clone() const override;

    uint16_t CountValueBits() const override;

protected:

    const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const override;

    const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const override;

private:

    };
END_IMAGEPP_NAMESPACE
