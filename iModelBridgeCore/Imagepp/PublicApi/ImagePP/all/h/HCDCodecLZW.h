//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecLZW.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecLZW
//-----------------------------------------------------------------------------
// LZW codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecLZW : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_LZW, HCDCodecImage)

public:

    IMAGEPP_EXPORT                 HCDCodecLZW();

    IMAGEPP_EXPORT                 HCDCodecLZW(size_t pi_Width,
                                       size_t pi_Height,
                                       size_t pi_BitsPerPixel,
                                       unsigned short pi_Predictor);

    HCDCodecLZW(const HCDCodecLZW& pi_rObj);

    ~HCDCodecLZW();

    virtual bool IsBitsPerPixelSupported(size_t pi_Bits) const;

    virtual size_t CompressSubset(const void* pi_pInData, size_t pi_InDataSize, void* po_pOutBuffer, size_t pi_OutBufferSize) override;
        
    virtual size_t DecompressSubset(const void* pi_pInData, size_t pi_InDataSize, void* po_pOutBuffer, size_t pi_OutBufferSize) override;

    virtual void    SetDimensions(size_t pi_Width, size_t pi_Height);

    virtual HCDCodec*Clone() const override;


protected:

private:

    unsigned short m_Predictor;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
class HCDCodecLZWPredicateExt : public HCDCodecLZW
{
       HDECLARE_CLASS_ID(HCDCodecId_LZWPredicate, HCDCodecLZW)    
public:

        IMAGEPP_EXPORT HCDCodecLZWPredicateExt();

        IMAGEPP_EXPORT HCDCodecLZWPredicateExt(size_t pi_Width,
                                               size_t pi_Height,
                                               size_t pi_BitsPerPixel,
                                               uint16_t pi_Predictor,                                     
                                               uint32_t pi_SamplesPerPixel);
                        
        HCDCodecLZWPredicateExt(const HCDCodecLZWPredicateExt& pi_rObj);

        virtual ~HCDCodecLZWPredicateExt();        

        virtual size_t CompressSubset(const void* pi_pInData, size_t pi_InDataSize, void* po_pOutBuffer, size_t pi_OutBufferSize) override;
        
        virtual size_t DecompressSubset(const void* pi_pInData, size_t pi_InDataSize, void* po_pOutBuffer, size_t pi_OutBufferSize) override;
                
        virtual HCDCodec* Clone() const override; 

protected:
     
private:

    uint16_t m_Predictor;
    uint32_t m_SamplesPerPixel;
};

END_IMAGEPP_NAMESPACE