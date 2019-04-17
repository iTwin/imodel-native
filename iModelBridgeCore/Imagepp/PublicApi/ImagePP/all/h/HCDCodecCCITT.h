//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecCCITT
//-----------------------------------------------------------------------------
// CCITT codec lass.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecCCITT : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_CCITT, HCDCodecImage)

public:

    ~HCDCodecCCITT();

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const override;

    HCDCodecCCITT(const HCDCodecCCITT& pi_rObj);

    size_t          GetSubsetMaxCompressedSize() const override;

    double GetEstimatedCompressionRatio() const override;

    IMAGEPP_EXPORT void     SetPhotometric(uint16_t pi_Photo);
    uint16_t GetPhotometric() const;

    IMAGEPP_EXPORT void     SetBitRevTable(bool pi_Reverse);

protected:
    
    virtual HCDCodec* Clone() const override = 0;

    HCDCodecCCITT();

    HCDCodecCCITT(size_t pi_Width,
                  size_t pi_Height);

    bool           m_bitrevtable;
    uint16_t m_photometric;
    };

END_IMAGEPP_NAMESPACE
