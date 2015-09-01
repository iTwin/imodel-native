//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV48R16G16B16.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeV48R16G16B16
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Note
//
// This pixel type work with channel set to 11 bits. We must re-design pixel type
// to support most pixel channel
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelTypeRGB.h"
#include "HRPPixelPalette.h"
#include "HRPPixelConverter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeV48R16G16B16 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_V48R16G16B16)

public:
    // Primary methods
    IMAGEPP_EXPORT                 HRPPixelTypeV48R16G16B16();
    IMAGEPP_EXPORT                 HRPPixelTypeV48R16G16B16(const HRPPixelTypeV48R16G16B16& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HRPPixelTypeV48R16G16B16();

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
