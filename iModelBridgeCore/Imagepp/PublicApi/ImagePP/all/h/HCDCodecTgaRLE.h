//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecTgaRLE.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"


class HCDCodecTGARLE : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1262, HCDCodecImage)

public:

    _HDLLu                 HCDCodecTGARLE();

    _HDLLu                 HCDCodecTGARLE( size_t pi_Width,
                                           size_t pi_Height,
                                           size_t pi_BitsPerPixel,
                                           Byte pi_AlphaChannelBits);

    _HDLLu                 HCDCodecTGARLE( const HCDCodecTGARLE& pi_rObj);

    _HDLLu                 ~HCDCodecTGARLE();

    size_t          CompressSubset( const void* pi_pInData,
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

    // alpha
    _HDLLu void     SetAlphaChannelBits(Byte pi_AlphaChannelBits);
    _HDLLu Byte     GetAlphaChannelBits() const;

    _HDLLu void     SetNumberOfBitsPerPixelInOutput(Byte pi_BitsPerPixel);

protected:

private:

    Byte          m_AlphaChannelBits;
    Byte          m_NumberOfBitsPerPixelInOutput;
    };


