//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecLRDRLE.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecLRDRLE
//-----------------------------------------------------------------------------
// RLE8 codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"


class HCDCodecLRDRLE : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1297, HCDCodecImage)

public:

    _HDLLu                 HCDCodecLRDRLE();

    _HDLLu                 HCDCodecLRDRLE(size_t pi_Width,
                                          size_t pi_Height);

    _HDLLu                 HCDCodecLRDRLE(const HCDCodecLRDRLE& pi_rObj);

    _HDLLu                 ~HCDCodecLRDRLE();

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

    void            SetOneLineMode(bool pi_Enable);

    bool           IsBitsPerPixelSupported(size_t pi_Bits) const;

    size_t          GetSubsetMaxCompressedSize() const;

protected:

private:
    bool            m_OneLineMode;
    };
