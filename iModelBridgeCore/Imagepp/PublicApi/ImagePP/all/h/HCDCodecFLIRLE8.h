//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecFLIRLE8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecFLIRLE8
//-----------------------------------------------------------------------------
// RLE8 codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecFLIRLE8 : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_FLIRLE8, HCDCodecImage)

public:

    IMAGEPP_EXPORT                 HCDCodecFLIRLE8();

    IMAGEPP_EXPORT                 HCDCodecFLIRLE8(  size_t pi_Width,
                                             size_t pi_Height);

    IMAGEPP_EXPORT                 HCDCodecFLIRLE8(const HCDCodecFLIRLE8& pi_rObj);

    IMAGEPP_EXPORT                 ~HCDCodecFLIRLE8();

    size_t          CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize);

    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize);

    bool           HasLineAccess() const;

    virtual HCDCodec* Clone() const override;

    IMAGEPP_EXPORT void            SetLineHeader(bool pi_Enable);

    void            SetOneLineMode(bool pi_Enable);

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const;

    size_t          GetSubsetMaxCompressedSize() const;


protected:

private:

    bool           m_LineHeader;

    bool            m_OneLineMode;
    };

END_IMAGEPP_NAMESPACE