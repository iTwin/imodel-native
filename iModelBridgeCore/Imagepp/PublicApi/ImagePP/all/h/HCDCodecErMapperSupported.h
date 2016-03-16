//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecErMapperSupported.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecECW
//-----------------------------------------------------------------------------
// ECW codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecErMapperSupported : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_ErMapperSupported, HCDCodecImage)

public:

    // primary methods
    IMAGEPP_EXPORT          HCDCodecErMapperSupported();
    IMAGEPP_EXPORT          HCDCodecErMapperSupported(const HCDCodecErMapperSupported& pi_rObj);
    IMAGEPP_EXPORT virtual  ~HCDCodecErMapperSupported();

    IMAGEPP_EXPORT void     SetCompressionRatio(uint16_t pi_Ratio);
    IMAGEPP_EXPORT uint16_t GetCompressionRatio() const;

    // overriden methods
    virtual HCDCodec* Clone() const override;

    virtual size_t  CompressSubset(const void* pi_pInData,
                                   size_t      pi_InDataSize,
                                   void*       po_pOutBuffer,
                                   size_t      pi_OutBufferSize);
    virtual size_t  DecompressSubset(const void* pi_pInData,
                                     size_t      pi_InDataSize,
                                     void*       po_pOutBuffer,
                                     size_t      pi_OutBufferSize);
    virtual bool   HasLineAccess() const;

    virtual bool   IsBitsPerPixelSupported(size_t pi_Bits) const;

protected :

    HCDCodecErMapperSupported(const WString& pi_rCodecName);

private :

    uint16_t m_CompressionRatio;

    };

END_IMAGEPP_NAMESPACE