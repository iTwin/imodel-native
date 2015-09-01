//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecZlib.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecZlib
//-----------------------------------------------------------------------------
// Zlib codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecDeflate.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecZlib : public HCDCodecDeflate
    {
    HDECLARE_CLASS_ID(HCDCodecId_Zlib, HCDCodecDeflate)

public:

    IMAGEPP_EXPORT                 HCDCodecZlib();

    IMAGEPP_EXPORT                 HCDCodecZlib(size_t pi_DataSize);

    IMAGEPP_EXPORT                 HCDCodecZlib(const HCDCodecZlib& pi_rObj);

    IMAGEPP_EXPORT                 ~HCDCodecZlib();

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

END_IMAGEPP_NAMESPACE