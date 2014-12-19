//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV16Int16.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV16Int16
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

class HRPPixelTypeV16Int16 : public HRPPixelType
    {
public:

    HPM_DECLARE_CLASS_DLL(_HDLLg,  1055)

    // Primary methods
    _HDLLg                 HRPPixelTypeV16Int16(HRPChannelType::ChannelRole pi_Role = HRPChannelType::USER,
                                                const int16_t*              pi_pNoDataValue = 0);
    _HDLLg                 HRPPixelTypeV16Int16(const HRPPixelTypeV16Int16& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV16Int16();

    virtual HPMPersistentObject*
    Clone() const;

    virtual unsigned short CountValueBits() const;

protected:

    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;
    };