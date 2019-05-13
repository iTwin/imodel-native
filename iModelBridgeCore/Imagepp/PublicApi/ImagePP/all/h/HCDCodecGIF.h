//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecGIF
//-----------------------------------------------------------------------------
// GIF codec class.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecGIF : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_GIF, HCDCodecImage)

public:

    ~HCDCodecGIF();

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const override;

    HCDCodecGIF(const HCDCodecGIF& pi_rObj);

    size_t          GetSubsetMaxCompressedSize() const override;

    double GetEstimatedCompressionRatio() const override;

protected:

    virtual HCDCodec* Clone() const override = 0;

    HCDCodecGIF();

    HCDCodecGIF(size_t pi_Width,
                size_t pi_Height,
                size_t pi_BitsPerPixel);

private:

    HCDCodecGIF&    operator=(const HCDCodecGIF& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
