//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecHMRPackBits
//-----------------------------------------------------------------------------
// HMRPackBits codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecPackBits.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecHMRPackBits : public HCDCodecPackBits
    {
    HDECLARE_CLASS_ID(HCDCodecId_PackBitsHMR, HCDCodecPackBits)

public:

    IMAGEPP_EXPORT                 HCDCodecHMRPackBits();


    IMAGEPP_EXPORT                 HCDCodecHMRPackBits(   uint32_t pi_Width,
                                                  uint32_t pi_Height,
                                                  uint32_t pi_BitsPerPixel);

    IMAGEPP_EXPORT                 HCDCodecHMRPackBits(const HCDCodecHMRPackBits& pi_rObj);

    IMAGEPP_EXPORT                 ~HCDCodecHMRPackBits();

    size_t          GetSubsetMaxCompressedSize() const override;

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

protected:

private:

    Byte*         CompressRun(Byte* pi_pIn,
                               size_t pi_InSize,
                               Byte* pOut);

    Byte*         DecompressRun(Byte* pi_pIn,
                                 Byte* pi_pOut,
                                 size_t pi_OutSize);
    };

END_IMAGEPP_NAMESPACE
