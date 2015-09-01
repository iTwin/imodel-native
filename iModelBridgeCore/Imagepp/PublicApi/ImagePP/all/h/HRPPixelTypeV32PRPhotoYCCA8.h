//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV32PRPhotoYCCA8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV32PRPhotoYCCA8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV32PRPhotoYCCA8 : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V32PRPhotoYCCA8)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV32PRPhotoYCCA8();
    IMAGEPP_EXPORT                 HRPPixelTypeV32PRPhotoYCCA8(const HRPPixelTypeV32PRPhotoYCCA8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV32PRPhotoYCCA8();

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


