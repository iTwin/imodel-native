//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV16PRGray8A8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV16PRGray8A8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV16PRGray8A8 : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V16PRGray8A8)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV16PRGray8A8();

    IMAGEPP_EXPORT                 HRPPixelTypeV16PRGray8A8(const HRPPixelTypeV16PRGray8A8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV16PRGray8A8();

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
