//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV16B5G5R5
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV16B5G5R5 : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V16B5G5R5)

public:

    // Primary methods
    IMAGEPPTEST_EXPORT HRPPixelTypeV16B5G5R5();

    HRPPixelTypeV16B5G5R5(const HRPPixelTypeV16B5G5R5& pi_rObj);

    virtual         ~HRPPixelTypeV16B5G5R5();

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

