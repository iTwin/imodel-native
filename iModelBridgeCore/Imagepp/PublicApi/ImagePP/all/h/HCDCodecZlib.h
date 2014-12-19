//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecZlib.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecZlib
//-----------------------------------------------------------------------------
// Zlib codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecDeflate.h"

class HCDCodecZlib : public HCDCodecDeflate
    {
    HDECLARE_CLASS_ID(1175, HCDCodecDeflate)

public:

    _HDLLu                 HCDCodecZlib();

    _HDLLu                 HCDCodecZlib(size_t pi_DataSize);

    _HDLLu                 HCDCodecZlib(const HCDCodecZlib& pi_rObj);

    _HDLLu                 ~HCDCodecZlib();

    size_t          CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize);


    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize);

    virtual size_t  GetSubsetMaxCompressedSize() const;

    virtual HCDCodec* Clone() const override;

protected:

private:
    };

