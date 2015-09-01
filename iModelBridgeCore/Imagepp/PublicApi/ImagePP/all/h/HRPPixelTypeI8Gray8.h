//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeI8Gray8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelTypeI8Gray8
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelTypeI8R8G8B8.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelTypeI8Gray8 : public HRPPixelTypeI8R8G8B8
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_I8Gray8)

public:
    // Primary methods
    IMAGEPP_EXPORT          HRPPixelTypeI8Gray8     ();
    HRPPixelTypeI8Gray8     (const HRPPixelTypeI8Gray8& pi_rObj);

    IMAGEPP_EXPORT virtual  ~HRPPixelTypeI8Gray8    ();

    virtual HPMPersistentObject*
    Clone() const;

protected:

    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;

private:

    };
END_IMAGEPP_NAMESPACE

