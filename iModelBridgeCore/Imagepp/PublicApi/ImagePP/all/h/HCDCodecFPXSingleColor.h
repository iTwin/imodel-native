//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecFPXSingleColor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecFPXSingleColor
//-----------------------------------------------------------------------------
// FPXSingleColor codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecSingleColor.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDCodecFPXSingleColor : public HCDCodecSingleColor
    {
    HDECLARE_CLASS_ID(HCDCodecId_FlashpixSingleColor, HCDCodecSingleColor)

public:

    IMAGEPP_EXPORT                     HCDCodecFPXSingleColor();

    IMAGEPP_EXPORT                     HCDCodecFPXSingleColor(   size_t pi_Width,
                                                         size_t pi_Height,
                                                         size_t pi_BitsPerPixel);

    IMAGEPP_EXPORT                     HCDCodecFPXSingleColor(const HCDCodecFPXSingleColor& pi_rObj);

    IMAGEPP_EXPORT                     ~HCDCodecFPXSingleColor();

    
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

END_IMAGEPP_NAMESPACE
