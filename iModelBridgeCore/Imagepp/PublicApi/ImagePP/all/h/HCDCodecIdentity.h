//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecIdentity.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecIdentity
//-----------------------------------------------------------------------------
// Identity codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecVector.h"


class HCDCodecIdentity : public HCDCodecVector
    {
    HDECLARE_CLASS_ID(1182, HCDCodecVector)

public:

    ~HCDCodecIdentity();

    _HDLLu                 HCDCodecIdentity();

    _HDLLu                 HCDCodecIdentity(size_t pi_DataSize);

    _HDLLu                 HCDCodecIdentity(const HCDCodecIdentity& pi_rObj);

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

    void            SetName(const WString& pi_rName);


protected:

private:
    };

