//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecLZW.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecLZW
//-----------------------------------------------------------------------------
// LZW codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

class HCDCodecLZW : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1306, HCDCodecImage)

public:

    _HDLLu                 HCDCodecLZW();

    _HDLLu                 HCDCodecLZW(size_t pi_Width,
                                       size_t pi_Height,
                                       size_t pi_BitsPerPixel,
                                       unsigned short pi_Predictor);

    HCDCodecLZW(const HCDCodecLZW& pi_rObj);

    ~HCDCodecLZW();

    virtual bool IsBitsPerPixelSupported(size_t pi_Bits) const;

    size_t          CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize);

    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize);

    virtual void    SetDimensions(size_t pi_Width, size_t pi_Height);

    virtual HCDCodec*Clone() const override;


protected:

private:

    unsigned short m_Predictor;
    };

