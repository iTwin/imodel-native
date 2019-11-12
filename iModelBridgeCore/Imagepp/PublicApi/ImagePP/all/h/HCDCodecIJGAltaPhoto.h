//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Class : HCDCodecIJGAltaPhoto
//--------------------------------------------------------------------------


#pragma once

#include "HCDCodecIJG.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecIJGAltaPhoto : public HCDCodecIJG
    {
public:

    HCDCodecIJGAltaPhoto();
    HCDCodecIJGAltaPhoto(uint32_t pi_Width,
                         uint32_t pi_Height,
                         uint32_t pi_BitsPerPixel);
    HCDCodecIJGAltaPhoto(const HCDCodecIJGAltaPhoto& pi_rObj);
    virtual         ~HCDCodecIJGAltaPhoto();

protected:

private:

    void            SetAltaPhotoCodecParameters();
    };


END_IMAGEPP_NAMESPACE
