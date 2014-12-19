//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecCCITT.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecCCITT
//-----------------------------------------------------------------------------
// CCITT codec lass.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecImage.h"
#include "ImageppLib.h"

class HCDCodecCCITT : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1180, HCDCodecImage)

public:

    ~HCDCodecCCITT();

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const;

    HCDCodecCCITT(const HCDCodecCCITT& pi_rObj);

    size_t          GetSubsetMaxCompressedSize() const;

    virtual double GetEstimatedCompressionRatio() const;

    _HDLLu void     SetPhotometric(unsigned short pi_Photo);
    unsigned short GetPhotometric() const;

    _HDLLu void     SetBitRevTable(bool pi_Reverse);

protected:
    
    virtual HCDCodec* Clone() const override = 0;

    HCDCodecCCITT();

    HCDCodecCCITT(size_t pi_Width,
                  size_t pi_Height);

    bool           m_bitrevtable;
    unsigned short m_photometric;
    };

