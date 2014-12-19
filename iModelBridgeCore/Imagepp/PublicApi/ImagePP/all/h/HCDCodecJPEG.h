//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecJPEG.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecJPEG
//-----------------------------------------------------------------------------
// JPEG codec lass.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecImage.h"

class HCDCodecJPEG : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1158, HCDCodecImage)

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

