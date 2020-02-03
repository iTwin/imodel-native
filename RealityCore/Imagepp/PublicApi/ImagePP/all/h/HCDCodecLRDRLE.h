//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecLRDRLE
//-----------------------------------------------------------------------------
// RLE8 codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecLRDRLE : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_LRDRLE, HCDCodecImage)

public:

    IMAGEPP_EXPORT                 HCDCodecLRDRLE();

    IMAGEPP_EXPORT                 HCDCodecLRDRLE(size_t pi_Width,
                                          size_t pi_Height);

    IMAGEPP_EXPORT                 HCDCodecLRDRLE(const HCDCodecLRDRLE& pi_rObj);

    IMAGEPP_EXPORT                 ~HCDCodecLRDRLE();

    size_t          CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize) override;

    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize) override;

    bool           HasLineAccess() const override;

    virtual HCDCodec* Clone() const override;

    void            SetOneLineMode(bool pi_Enable);

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const override;

    size_t          GetSubsetMaxCompressedSize() const override;

protected:

private:
    bool            m_OneLineMode;
    };

END_IMAGEPP_NAMESPACE
