//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecBMPRLE8
//-----------------------------------------------------------------------------
// RLE8 codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecBMPRLE4 : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_BMPRLE4, HCDCodecImage)

public:

    IMAGEPP_EXPORT                 HCDCodecBMPRLE4();

    HCDCodecBMPRLE4(size_t pi_Width,
                    size_t pi_Height);

    HCDCodecBMPRLE4(const HCDCodecBMPRLE4& pi_rObj);

    ~HCDCodecBMPRLE4();

    size_t          CompressSubset(const void* pi_pInData,
                                   size_t     pi_InDataSize,
                                   void*      po_pOutBuffer,
                                   size_t     pi_OutBufferSize) override;

    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t       pi_InDataSize,
                                     void*        po_pOutBuffer,
                                     size_t       pi_OutBufferSize) override;

    bool           HasLineAccess() const override;

    virtual HCDCodec* Clone() const override;

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const override;

    size_t          GetSubsetMaxCompressedSize() const override;

protected:

private:
    };

END_IMAGEPP_NAMESPACE
