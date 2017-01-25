//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV16R5G6B5.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV16R5G6B5
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV16R5G6B5 : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V16R5G6B5)

public:

    // Primary methods
    IMAGEPPTEST_EXPORT HRPPixelTypeV16R5G6B5();

    HRPPixelTypeV16R5G6B5(const HRPPixelTypeV16R5G6B5& pi_rObj);

    virtual         ~HRPPixelTypeV16R5G6B5();

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

