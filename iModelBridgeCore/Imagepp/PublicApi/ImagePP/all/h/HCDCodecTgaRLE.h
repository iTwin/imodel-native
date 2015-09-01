//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecTgaRLE.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecTGARLE : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_TGARLE, HCDCodecImage)

public:

    IMAGEPP_EXPORT                 HCDCodecTGARLE();

    IMAGEPP_EXPORT                 HCDCodecTGARLE( size_t pi_Width,
                                           size_t pi_Height,
                                           size_t pi_BitsPerPixel,
                                           Byte pi_AlphaChannelBits);

    IMAGEPP_EXPORT                 HCDCodecTGARLE( const HCDCodecTGARLE& pi_rObj);

    IMAGEPP_EXPORT                 ~HCDCodecTGARLE();

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
    IMAGEPP_EXPORT void     SetAlphaChannelBits(Byte pi_AlphaChannelBits);
    IMAGEPP_EXPORT Byte     GetAlphaChannelBits() const;

    IMAGEPP_EXPORT void     SetNumberOfBitsPerPixelInOutput(Byte pi_BitsPerPixel);

protected:

private:

    Byte          m_AlphaChannelBits;
    Byte          m_NumberOfBitsPerPixelInOutput;
    };


END_IMAGEPP_NAMESPACE