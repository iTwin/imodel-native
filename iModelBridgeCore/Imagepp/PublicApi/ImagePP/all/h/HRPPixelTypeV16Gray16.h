//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV16Gray16.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV16Gray16
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeGray.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV16Gray16 : public HRPPixelTypeGray
    {
public:

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V16Gray16)

    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV16Gray16();
    IMAGEPP_EXPORT                 HRPPixelTypeV16Gray16(const HRPPixelTypeV16Gray16& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV16Gray16();

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
