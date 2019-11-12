//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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

    HPMPersistentObject*
    Clone() const override;

protected:

    const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const override;

    const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const override;

private:

    };
END_IMAGEPP_NAMESPACE

