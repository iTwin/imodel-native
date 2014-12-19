//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecFPXSingleColor.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecFPXSingleColor
//-----------------------------------------------------------------------------
// FPXSingleColor codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecSingleColor.h"


class HCDCodecFPXSingleColor : public HCDCodecSingleColor
    {
    HDECLARE_CLASS_ID(1164, HCDCodecSingleColor)

public:

    _HDLLu                     HCDCodecFPXSingleColor();

    _HDLLu                     HCDCodecFPXSingleColor(   size_t pi_Width,
                                                         size_t pi_Height,
                                                         size_t pi_BitsPerPixel);

    _HDLLu                     HCDCodecFPXSingleColor(const HCDCodecFPXSingleColor& pi_rObj);

    _HDLLu                     ~HCDCodecFPXSingleColor();

    
    virtual size_t          CompressSubset(const void* pi_pInData,
                                        size_t pi_InDataSize,
                                        void* po_pOutBuffer,
                                        size_t pi_OutBufferSize) override;

    virtual size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize) override;

    bool           HasLineAccess() const;

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const;

    virtual HCDCodec* Clone() const override;

protected:

private:
    };

