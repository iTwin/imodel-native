//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV32R8G8B8X8.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV32R8G8B8X8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV32R8G8B8X8 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT, HRPPixelTypeId_V32R8G8B8X8)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV32R8G8B8X8();
    IMAGEPP_EXPORT                 HRPPixelTypeV32R8G8B8X8(const HRPPixelTypeV32R8G8B8X8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV32R8G8B8X8();

    virtual HPMPersistentObject*
    Clone() const;

    virtual uint16_t CountValueBits() const;

protected:

    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;


private:
    };
END_IMAGEPP_NAMESPACE


