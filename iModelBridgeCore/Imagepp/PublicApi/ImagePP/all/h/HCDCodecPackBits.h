//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecPackBits.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecPackBits
//-----------------------------------------------------------------------------
// PackBits codec lass.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecPackBits : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_PackBits, HCDCodecImage)

public:

    ~HCDCodecPackBits();

    bool   IsBitsPerPixelSupported(size_t pi_Bits) const override;

    size_t  GetSubsetMaxCompressedSize() const override;
    double GetEstimatedCompressionRatio() const override;

protected:

    HCDCodecPackBits();

    virtual HCDCodec* Clone() const override = 0;

    HCDCodecPackBits( size_t pi_Width,
                      size_t pi_Height,
                      size_t pi_BitsPerPixel);

    HCDCodecPackBits(const HCDCodecPackBits& pi_rObj);
    };

END_IMAGEPP_NAMESPACE