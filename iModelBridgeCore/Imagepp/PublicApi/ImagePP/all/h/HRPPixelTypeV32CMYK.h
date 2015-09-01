//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV32CMYK.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV32CMYK
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV32CMYK : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V32CMYK)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV32CMYK();

    IMAGEPP_EXPORT                 HRPPixelTypeV32CMYK(const HRPPixelTypeV32CMYK& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV32CMYK();

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


