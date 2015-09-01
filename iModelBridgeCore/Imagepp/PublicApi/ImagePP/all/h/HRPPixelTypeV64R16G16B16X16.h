//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV64R16G16B16X16.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV64R16G16B16X16
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"
//#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV64R16G16B16X16 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V64R16G16B16X16)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV64R16G16B16X16();
    IMAGEPP_EXPORT                 HRPPixelTypeV64R16G16B16X16(const HRPPixelTypeV64R16G16B16X16& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV64R16G16B16X16();

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
