//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecCCITT.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const;

    HCDCodecCCITT(const HCDCodecCCITT& pi_rObj);

    size_t          GetSubsetMaxCompressedSize() const;

    virtual double GetEstimatedCompressionRatio() const;

    IMAGEPP_EXPORT void     SetPhotometric(unsigned short pi_Photo);
    unsigned short GetPhotometric() const;

    IMAGEPP_EXPORT void     SetBitRevTable(bool pi_Reverse);

protected:
    
    virtual HCDCodec* Clone() const override = 0;

    HCDCodecCCITT();

    HCDCodecCCITT(size_t pi_Width,
                  size_t pi_Height);

    bool           m_bitrevtable;
    unsigned short m_photometric;
    };

END_IMAGEPP_NAMESPACE