//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PrivateApi/ImagePPInternal/gra/HRAImageNearestSamplerRLE.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRAImageSampler.h" //<ImagePPInternal/gra/HRAImageSampler.h>

BEGIN_IMAGEPP_NAMESPACE

class HCDPacketRLE;

/*----------------------------------------------------------------------------+
|struct HRAImageNearestSamplerRLE
| This class is not thread safe and thus cannot be use by multiple thread at the 
| same time. See m_pixelOffSet members.  
+----------------------------------------------------------------------------*/
struct HRAImageNearestSamplerRLE : HRAImageSampler
    {
public:
    // Use HRAImageSampler::CreateNearestRle(...) to instanciate this class.
    friend HRAImageSampler;

    struct RLEBufferPosition
        {
        uint32_t m_IndexInBuffer;
        uint32_t m_StartPosition;
        };

    virtual ~HRAImageNearestSamplerRLE();

protected:
    virtual ImagePPStatus _Stretch(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset) override;
    virtual ImagePPStatus _Warp(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset) override;

private:
    enum BiasMode
        {
        BIAS_EqualWeight,       // a pixel must fill the center to be taken.
        BIAS_Black,             // bias toward zeroes.
        BIAS_White              // bias toward ones
        };

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename Sampler_T, bool IsStretch_T>
    struct SamplerSurfaceVisitor : public SurfaceVisitor
        {
        SamplerSurfaceVisitor(Sampler_T& sampler, HRAImageSampleR outData, PixelOffset const& outOffset, PixelOffset const& inOffset)
        :m_sampler(sampler), m_outData(outData), m_outOffset(outOffset), m_inOffset(inOffset){}

        virtual ~SamplerSurfaceVisitor(){};

        virtual ImagePPStatus _Visit(HRAPacketRleSurface& surface) override { return Process(surface); }
        virtual ImagePPStatus _Visit(HRASampleRleSurface& surface) override { return Process(surface); }

        virtual ImagePPStatus _Visit(HRASampleN1Surface& surface) override { return IMAGEPP_STATUS_UnknownError; }
        virtual ImagePPStatus _Visit(HRAPacketN1Surface& surface) override { return IMAGEPP_STATUS_UnknownError; }
        virtual ImagePPStatus _Visit(HRAPacketCodecRleSurface& surface) override { return IMAGEPP_STATUS_UnknownError; }
        virtual ImagePPStatus _Visit(HRAPacketN8Surface& surface) override {return IMAGEPP_STATUS_UnknownError;}
        virtual ImagePPStatus _Visit(HRASampleN8Surface& surface) override {return IMAGEPP_STATUS_UnknownError;}

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                   Mathieu.Marchand  10/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        template<typename Surface_T>
        ImagePPStatus Process(Surface_T& surface)
            {
            if (IsStretch_T)    // resolved at compile time.
                m_sampler.Stretch_T(m_outData, m_outOffset, surface, m_inOffset);
            else
                m_sampler.Warp_T(m_outData, m_outOffset, surface, m_inOffset);

            return IMAGEPP_STATUS_Success;
            }
    
        Sampler_T& m_sampler;
        HRAImageSampleR m_outData;
        PixelOffset const& m_outOffset;
        PixelOffset const& m_inOffset;
        };

    template<class Surface_T>
    ImagePPStatus Stretch_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset);

    template<class Surface_T>
    ImagePPStatus Warp_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset);
    
    HRAImageNearestSamplerRLE(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);

    BiasMode m_biasMode;

    // ***** Begin Not-thread safe *****
    vector<RLEBufferPosition> m_bufferPosition;
    // ***** End Not-thread safe *****

    };

END_IMAGEPP_NAMESPACE