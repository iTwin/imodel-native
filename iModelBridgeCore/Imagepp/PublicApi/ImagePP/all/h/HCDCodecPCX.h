//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecPCX.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecPCX
//-----------------------------------------------------------------------------
// General class for CodecPCXs.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecVector.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDCodecPCX : public HCDCodecVector
    {
    HDECLARE_CLASS_ID(HCDCodecId_PCX, HCDCodecVector)

public:

    // constructors

    HCDCodecPCX();

    HCDCodecPCX(size_t pi_DataSize);

    HCDCodecPCX(const HCDCodecPCX& pi_rObj);

    // destructor

    virtual         ~HCDCodecPCX();




    size_t          CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize);


    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize);

    virtual HCDCodec* Clone() const override;

    virtual size_t  GetSubsetMaxCompressedSize() const;


protected:

private:
    };

END_IMAGEPP_NAMESPACE

