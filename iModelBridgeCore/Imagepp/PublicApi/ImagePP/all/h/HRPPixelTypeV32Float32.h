//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV32Float32.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV32Float32
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

class HRPPixelTypeV32Float32 : public HRPPixelType
    {
public:

    HPM_DECLARE_CLASS_DLL(_HDLLg,  1054)

    // Primary methods
    _HDLLg                 HRPPixelTypeV32Float32(
        HRPChannelType::ChannelRole pi_Role = HRPChannelType::USER,
        const float*               pi_pNoDataValue = 0);
    _HDLLg                 HRPPixelTypeV32Float32(const HRPPixelTypeV32Float32& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV32Float32();

    virtual HPMPersistentObject*
    Clone() const;

    virtual unsigned short CountValueBits() const;

protected:

    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;
    };