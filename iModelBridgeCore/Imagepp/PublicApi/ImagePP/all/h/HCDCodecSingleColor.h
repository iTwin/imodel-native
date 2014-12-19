//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecSingleColor.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecSingleColor
//-----------------------------------------------------------------------------
// SingleColor codec lass.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecImage.h"

class HCDCodecSingleColor : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1155, HCDCodecImage)

public:

    ~HCDCodecSingleColor();

protected:

    HCDCodecSingleColor();

    HCDCodecSingleColor(size_t pi_Width,
                        size_t pi_Height,
                        size_t pi_BitsPerPixel);

    HCDCodecSingleColor(const HCDCodecSingleColor& pi_rObj);

    virtual HCDCodec* Clone() const override = 0;
    };

