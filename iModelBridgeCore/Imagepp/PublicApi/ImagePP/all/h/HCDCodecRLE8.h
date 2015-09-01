//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecRLE8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecRLE8
//-----------------------------------------------------------------------------
// RLE8 codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDCodecRLE8 : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_RLE8, HCDCodecImage)

public:

    IMAGEPP_EXPORT                 HCDCodecRLE8();

    IMAGEPP_EXPORT                 HCDCodecRLE8(size_t pi_Width, size_t pi_Height, size_t pi_BitsPerPixel);

    IMAGEPP_EXPORT                 HCDCodecRLE8(const HCDCodecRLE8& pi_rObj);

    IMAGEPP_EXPORT                 ~HCDCodecRLE8();

    size_t          CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize) override;

    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize) override;

    bool           HasLineAccess() const;

    virtual HCDCodec* Clone() const override;

    IMAGEPP_EXPORT void            SetLineHeader(bool pi_Enable);

    void            SetOneLineMode(bool pi_Enable);

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const;

    size_t          GetSubsetMaxCompressedSize() const;


protected:

private:

    size_t CompressSubsetN8(const void* pi_pInData,
                            size_t      pi_InDataSize,
                            void*       po_pOutBuffer,
                            size_t      po_OutBufferSize);

    size_t DecompressSubsetN8(const void* pi_pInData,
                              size_t pi_InDataSize,
                              void* po_pOutBuffer,
                              size_t pi_OutBufferSize);

    bool           m_LineHeader;

    bool            m_OneLineMode;
    };

END_IMAGEPP_NAMESPACE
