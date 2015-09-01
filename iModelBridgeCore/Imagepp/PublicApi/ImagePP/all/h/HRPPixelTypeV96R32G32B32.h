//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV96R32G32B32.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV96R32G32B32
//-----------------------------------------------------------------------------


#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV96R32G32B32 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V96R32G32B32)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV96R32G32B32();
    IMAGEPP_EXPORT                 HRPPixelTypeV96R32G32B32(const HRPPixelTypeV96R32G32B32& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV96R32G32B32();

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
