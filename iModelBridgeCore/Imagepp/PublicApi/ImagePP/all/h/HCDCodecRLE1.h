//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecRLE1.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecRLE1
//-----------------------------------------------------------------------------
// RLE1 codec lass.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecImage.h"

class HCDCodecRLE1 : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1151, HCDCodecImage)

public:

    ~HCDCodecRLE1();

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const;

    size_t          GetSubsetMaxCompressedSize() const;

    _HDLLu virtual double GetEstimatedCompressionRatio() const;


protected:

    virtual HCDCodec* Clone() const override = 0;

    HCDCodecRLE1();
    
    HCDCodecRLE1(const HCDCodecRLE1& pi_rObj);

    HCDCodecRLE1(size_t pi_Width,
                 size_t pi_Height);
    };

