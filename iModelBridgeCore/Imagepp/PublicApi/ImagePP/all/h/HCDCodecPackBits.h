//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecPackBits.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    virtual bool   IsBitsPerPixelSupported(size_t pi_Bits) const;

    virtual size_t  GetSubsetMaxCompressedSize() const;
    virtual double GetEstimatedCompressionRatio() const;

protected:

    HCDCodecPackBits();

    virtual HCDCodec* Clone() const override = 0;

    HCDCodecPackBits( size_t pi_Width,
                      size_t pi_Height,
                      size_t pi_BitsPerPixel);

    HCDCodecPackBits(const HCDCodecPackBits& pi_rObj);
    };

END_IMAGEPP_NAMESPACE