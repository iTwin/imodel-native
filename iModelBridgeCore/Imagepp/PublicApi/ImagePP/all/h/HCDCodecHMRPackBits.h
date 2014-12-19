//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecHMRPackBits.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecHMRPackBits
//-----------------------------------------------------------------------------
// HMRPackBits codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecPackBits.h"

class HCDCodecHMRPackBits : public HCDCodecPackBits
    {
    HDECLARE_CLASS_ID(1177, HCDCodecPackBits)

public:

    _HDLLu                 HCDCodecHMRPackBits();


    _HDLLu                 HCDCodecHMRPackBits(   uint32_t pi_Width,
                                                  uint32_t pi_Height,
                                                  uint32_t pi_BitsPerPixel);

    _HDLLu                 HCDCodecHMRPackBits(const HCDCodecHMRPackBits& pi_rObj);

    _HDLLu                 ~HCDCodecHMRPackBits();

    size_t          GetSubsetMaxCompressedSize() const;

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

protected:

private:

    Byte*         CompressRun(Byte* pi_pIn,
                               size_t pi_InSize,
                               Byte* pOut);

    Byte*         DecompressRun(Byte* pi_pIn,
                                 Byte* pi_pOut,
                                 size_t pi_OutSize);
    };

