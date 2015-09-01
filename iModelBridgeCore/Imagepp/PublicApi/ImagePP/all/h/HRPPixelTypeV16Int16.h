//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV16Int16.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV16Int16
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV16Int16 : public HRPPixelType
    {
public:

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V16Int16)

    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV16Int16(HRPChannelType::ChannelRole pi_Role = HRPChannelType::USER,
                                                const int16_t*              pi_pNoDataValue = 0);
    IMAGEPP_EXPORT                 HRPPixelTypeV16Int16(const HRPPixelTypeV16Int16& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV16Int16();

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
