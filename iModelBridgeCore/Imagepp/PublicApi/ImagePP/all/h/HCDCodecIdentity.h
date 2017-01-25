//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecIdentity.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecIdentity
//-----------------------------------------------------------------------------
// Identity codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecVector.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDCodecIdentity : public HCDCodecVector
    {
    HDECLARE_CLASS_ID(HCDCodecId_Identity, HCDCodecVector)

public:

    ~HCDCodecIdentity();

    IMAGEPP_EXPORT                 HCDCodecIdentity();

    IMAGEPP_EXPORT                 HCDCodecIdentity(size_t pi_DataSize);

    IMAGEPP_EXPORT                 HCDCodecIdentity(const HCDCodecIdentity& pi_rObj);

    size_t          CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize) override;


    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize) override;

    size_t  GetSubsetMaxCompressedSize() const override;

    virtual HCDCodec* Clone() const override;

    void            SetName(const Utf8String& pi_rName);


protected:

private:
    };

END_IMAGEPP_NAMESPACE