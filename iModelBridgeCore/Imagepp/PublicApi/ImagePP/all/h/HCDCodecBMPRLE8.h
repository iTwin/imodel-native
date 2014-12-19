//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecBMPRLE8.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecBMPRLE8
//-----------------------------------------------------------------------------
// RLE8 codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

class HCDCodecBMPRLE8 : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1239, HCDCodecImage)

public:

    _HDLLu                 HCDCodecBMPRLE8();

    HCDCodecBMPRLE8(  size_t pi_Width,
                      size_t pi_Height);

    HCDCodecBMPRLE8(const HCDCodecBMPRLE8& pi_rObj);

    ~HCDCodecBMPRLE8();

    size_t          CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize);

    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize);

    bool           HasLineAccess() const;

    virtual HCDCodec* Clone() const override;

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const;

    size_t          GetSubsetMaxCompressedSize() const;

protected:

private:
    };

