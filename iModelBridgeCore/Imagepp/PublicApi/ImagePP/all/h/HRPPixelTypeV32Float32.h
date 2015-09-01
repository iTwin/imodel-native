//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV32Float32.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV32Float32
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV32Float32 : public HRPPixelType
    {
public:

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V32Float32)

    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV32Float32(
        HRPChannelType::ChannelRole pi_Role = HRPChannelType::USER,
        const float*               pi_pNoDataValue = 0);
    IMAGEPP_EXPORT                 HRPPixelTypeV32Float32(const HRPPixelTypeV32Float32& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV32Float32();

    virtual HPMPersistentObject*
    Clone() const;

    virtual unsigned short CountValueBits() const;

protected:

    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;
    };
END_IMAGEPP_NAMESPACE
