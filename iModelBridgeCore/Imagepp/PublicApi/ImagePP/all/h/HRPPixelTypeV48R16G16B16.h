//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeV48R16G16B16.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

class HRPPixelTypeV48R16G16B16 : public HRPPixelTypeRGB
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1269)

public:
    // Primary methods
    _HDLLg                 HRPPixelTypeV48R16G16B16();
    _HDLLg                 HRPPixelTypeV48R16G16B16(const HRPPixelTypeV48R16G16B16& pi_rObj);

    _HDLLg virtual         ~HRPPixelTypeV48R16G16B16();

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
