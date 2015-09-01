//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecJPEG.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecJPEG
//-----------------------------------------------------------------------------
// JPEG codec lass.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecJPEG : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_JPEG, HCDCodecImage)

public:

    ~HCDCodecJPEG();

    virtual bool   IsBitsPerPixelSupported(size_t pi_Bits) const;

    virtual HCDCodec* Clone() const override = 0;

protected:

    HCDCodecJPEG();

    HCDCodecJPEG(size_t pi_Width,
                 size_t pi_Height,
                 size_t pi_BitsPerPixel);
    
    HCDCodecJPEG(const HCDCodecJPEG& pi_rObj);

    };

END_IMAGEPP_NAMESPACE