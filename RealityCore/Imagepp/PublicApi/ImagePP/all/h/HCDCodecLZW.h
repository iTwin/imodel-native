//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecLZW
//-----------------------------------------------------------------------------
// LZW codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecLZW : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_LZW, HCDCodecImage)

public:
    
    HCDCodecLZW();

    HCDCodecLZW(size_t width, size_t height, size_t bitsPerPixel, uint16_t predictor, uint32_t samplesPerPixel);

    HCDCodecLZW(const HCDCodecLZW& pi_rObj);

    ~HCDCodecLZW();

    bool IsBitsPerPixelSupported(size_t pi_Bits) const override;

    virtual size_t CompressSubset(const void* pi_pInData, size_t pi_InDataSize, void* po_pOutBuffer, size_t pi_OutBufferSize) override;
        
    virtual size_t DecompressSubset(const void* pi_pInData, size_t pi_InDataSize, void* po_pOutBuffer, size_t pi_OutBufferSize) override;

    void    SetDimensions(size_t pi_Width, size_t pi_Height) override;

    virtual HCDCodec*Clone() const override;

private:

    void DecodeHorizontalPredicate(Byte* po_pOutBuffer, size_t dataSize);

    uint16_t m_Predictor = 1;   // 1 = None, 2 = horizontal, 3 = floatingPoint (not supported)
    uint32_t   m_samplePerPixels = 0;
    };


END_IMAGEPP_NAMESPACE
