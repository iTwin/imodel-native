//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeV32PR8PG8PB8A8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV32PR8PG8PB8A8 : public HRPPixelType
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V32PR8PG8PB8A8)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV32PR8PG8PB8A8();
    IMAGEPP_EXPORT                 HRPPixelTypeV32PR8PG8PB8A8(const HRPPixelTypeV32PR8PG8PB8A8& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV32PR8PG8PB8A8();

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


