//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecIJGAltaPhoto.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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