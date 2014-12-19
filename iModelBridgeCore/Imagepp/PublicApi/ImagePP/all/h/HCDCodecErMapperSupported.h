//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecErMapperSupported.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecECW
//-----------------------------------------------------------------------------
// ECW codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

class HCDCodecErMapperSupported : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1670, HCDCodecImage)

public:

    // primary methods
    _HDLLu          HCDCodecErMapperSupported();
    _HDLLu          HCDCodecErMapperSupported(const HCDCodecErMapperSupported& pi_rObj);
    _HDLLu virtual  ~HCDCodecErMapperSupported();

    _HDLLu void     SetCompressionRatio(unsigned short pi_Ratio);
    _HDLLu unsigned short GetCompressionRatio() const;

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

    unsigned short m_CompressionRatio;

    };