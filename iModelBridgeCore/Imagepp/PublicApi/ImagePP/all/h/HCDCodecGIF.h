//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecGIF.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const;

    HCDCodecGIF(const HCDCodecGIF& pi_rObj);

    size_t          GetSubsetMaxCompressedSize() const;

    virtual double GetEstimatedCompressionRatio() const;

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