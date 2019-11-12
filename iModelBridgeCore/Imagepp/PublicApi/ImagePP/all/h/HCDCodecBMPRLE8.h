//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecBMPRLE8
//-----------------------------------------------------------------------------
// RLE8 codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecBMPRLE8 : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_BMPRLE8, HCDCodecImage)

public:

    IMAGEPP_EXPORT                 HCDCodecBMPRLE8();

    HCDCodecBMPRLE8(  size_t pi_Width,
                      size_t pi_Height);

    HCDCodecBMPRLE8(const HCDCodecBMPRLE8& pi_rObj);

    ~HCDCodecBMPRLE8();

    size_t          CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize) override;

    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize) override;

    bool           HasLineAccess() const override;

    virtual HCDCodec* Clone() const override;

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const override;

    size_t          GetSubsetMaxCompressedSize() const override;

protected:

private:
    };

END_IMAGEPP_NAMESPACE
