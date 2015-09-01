//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageOpContrastStretchFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>
#include <Imagepp/all/h/HRAImageOpContrastStretchFilter.h>
#include <Imagepp/all/h/HGFLuvColorSpace.h>
#include <ImagePP/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T, uint32_t ChannelCount_T>
struct If_Rgba
    {
    static inline void CopyAlphaChannel(Data_T* pOutData, Data_T const* pInData, uint32_t column){}
    };

template<class Data_T>
struct If_Rgba<Data_T, 4>     // Assumed that 4 channels equals to RGBA.
    {
    static inline void CopyAlphaChannel(Data_T* pOutData, Data_T const* pInData, uint32_t column) {pOutData[column*4+3] = pInData[column*4+3];}
    };


/*---------------------------------------------------------------------------------**//**
* struct LightnessContrastStretchFilter_T                                               
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T, uint32_t ChannelCount_T>
struct LightnessContrastStretchFilter_T : HRAImageOpLightnessContrastStretchFilter::LightnessContrastStretchFilter
{
public:
    LightnessContrastStretchFilter_T(float minValue, float maxValue, float minContrastValue, float maxContrastValue, double gammaFactor) 
        :m_pColorSpaceConverter(DEFAULT_GAMMA_FACTOR, sizeof(Data_T)*8)
        {
        HPRECONDITION(minValue <= 100.0 && minValue >= 0.0);
        HPRECONDITION(maxValue <= 100.0 && maxValue >= 0.0);
        HPRECONDITION(minValue <= maxValue);
        HPRECONDITION(minContrastValue <= 100.0 && minContrastValue >= 0.0);
        HPRECONDITION(maxContrastValue <= 100.0 && maxContrastValue >= 0.0);
        HPRECONDITION(minContrastValue <= maxContrastValue);
        HPRECONDITION(gammaFactor <= 10.0 && gammaFactor > 0.0);

        m_MinValue = minValue;
        m_MaxValue = maxValue;
        m_MinContrastValue = minContrastValue;
        m_MaxContrastValue = maxContrastValue;
        m_GammaFactor = gammaFactor;
        }   

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Jonathan.Bernier  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImagePPStatus _Apply(HRAImageSampleR outData, HRAImageSampleCR inputData) override
        {
        HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);

        if (!HDOUBLE_EQUAL_EPSILON(m_GammaFactor, 1.0))
            return ProcessPixels<true>(outData, inputData);

        return ProcessPixels<false>(outData, inputData);
        }
       
private:

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<bool DoGammaAdjustment_T>
    ImagePPStatus ProcessPixels(HRAImageSampleR outData, HRAImageSampleCR inputData)
        {
        if (m_MaxContrastValue < 100 || m_MinContrastValue > 0)
            return ProcessPixels<DoGammaAdjustment_T, true>(outData, inputData);
        
        return ProcessPixels<DoGammaAdjustment_T, false>(outData, inputData);
        }

     /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Jonathan.Bernier  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<bool DoGammaAdjustment_T, bool DoOutputClipping_T>
    ImagePPStatus ProcessPixels(HRAImageSampleR outData, HRAImageSampleCR inputData)
        {
#ifndef DISABLE_BUFFER_GETDATA
        size_t inPitch, outPitch;
        Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

        double L;
        double U;
        double V;
        uint32_t Width = outData.GetWidth();
        uint32_t Height = outData.GetHeight();
              
        if (ChannelCount_T == 1)
            {
            for(uint32_t row=0; row < Height; ++row)
                {
                Data_T const* pInBufferRow  = (Data_T const*)(pInBuffer+row*inPitch);
                Data_T* pOutBufferRow = (Data_T*)(pOutBuffer+row*outPitch);

                for(uint32_t column=0; column < Width; ++column)
                    {
                    L = m_pColorSpaceConverter.LuminanceFromGray(pInBufferRow[column]);

                    //------------------------------------------
                    // Process the LigthnessContrastStretch
                    // Input clipping.
                    if (L < m_MinValue)
                        L = 0.0;
                    else if (L > m_MaxValue)
                        L = 100.0;
                    else
                        L = ((L - m_MinValue) / (double)(m_MaxValue - m_MinValue)) * 100.0;

                    //----------------------------------------
                    // Process gamma adjustment if required.
                    if (DoGammaAdjustment_T)         
                        L = (Data_T)(100.0 * pow( L / 100.0, 1 / m_GammaFactor));

                    //----------------------------------------
                    // Compute output clipping if any.
                    if (DoOutputClipping_T)
                        {
                        L = ((L /100.0) * ((m_MaxContrastValue - m_MinContrastValue))) + m_MinContrastValue;
                        }

                    pOutBufferRow[column] = (Data_T)m_pColorSpaceConverter.GrayFromLuminance(L);
                    }
                }
            }
        else
            {
            for(uint32_t row=0; row < Height; ++row)
                {
                Data_T const* pInBufferRow  = (Data_T const*)(pInBuffer+row*inPitch);
                Data_T* pOutBufferRow = (Data_T*)(pOutBuffer+row*outPitch);

                for(uint32_t column=0; column < Width; ++column)
                    {
                    m_pColorSpaceConverter.ConvertFromRGB(pInBufferRow[column*ChannelCount_T], pInBufferRow[column*ChannelCount_T+1], pInBufferRow[column*ChannelCount_T+2], &L, &U, &V);

                    //------------------------------------------
                    // Process the LigthnessContrastStretch
                    // Input clipping.
                    if (L < m_MinValue)
                        L = 0.0;
                    else if (L > m_MaxValue)
                        L = 100.0;
                    else
                        L = ((L - m_MinValue) / (double)(m_MaxValue - m_MinValue)) * 100.0;

                    //----------------------------------------
                    // Process gamma adjustment if required.
                    if (DoGammaAdjustment_T)         
                        L = (Data_T)(100.0 * pow( L / 100.0, 1 / m_GammaFactor));

                    //----------------------------------------
                    // Compute output clipping if any.
                    if (DoOutputClipping_T)
                        {
                        L = ((L /100.0) * ((m_MaxContrastValue - m_MinContrastValue))) + m_MinContrastValue;
                        }

                    m_pColorSpaceConverter.ConvertToRGB (L, U, V, &pOutBufferRow[column*ChannelCount_T], &pOutBufferRow[column*ChannelCount_T+1], &pOutBufferRow[column*ChannelCount_T+2]);
                    If_Rgba<Data_T, ChannelCount_T>::CopyAlphaChannel(pOutBufferRow, pInBufferRow, column);
                    }
                }
            }

#endif
        return IMAGEPP_STATUS_Success;
        }

    float  m_MinValue;
    float  m_MaxValue;
    float  m_MinContrastValue;
    float  m_MaxContrastValue;
    double m_GammaFactor;

    HGFLuvColorSpace m_pColorSpaceConverter;
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpLightnessContrastStretchFilterPtr HRAImageOpLightnessContrastStretchFilter::CreateLightnessContrastStretchFilter()
    {  
    return new HRAImageOpLightnessContrastStretchFilter();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpLightnessContrastStretchFilter::HRAImageOpLightnessContrastStretchFilter()
    :HRAImageOp(new HRPPixelNeighbourhood(1,1,0,0))
    {
    m_MinValue         = 0.0;
    m_MaxValue         = 100.;
    m_MinContrastValue = 0.0;
    m_MaxContrastValue = 100.0;
    m_GammaFactor      = 5.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpLightnessContrastStretchFilter::~HRAImageOpLightnessContrastStretchFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpLightnessContrastStretchFilter::GetInterval(float& MinValue, float& MaxValue) const
    {
    MinValue = m_MinValue;
    MaxValue = m_MaxValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpLightnessContrastStretchFilter::SetInterval(float MinValue, float MaxValue)
    {
    HPRECONDITION(MinValue <= MaxValue);

    m_MinValue = MinValue;
    m_MaxValue = MaxValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpLightnessContrastStretchFilter::GetContrastInterval(float& MinContrastValue, float& MaxContrastValue) const
    {
    MinContrastValue = m_MinContrastValue;
    MaxContrastValue = m_MaxContrastValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpLightnessContrastStretchFilter::SetContrastInterval(float MinContrastValue, float MaxContrastValue)
    {
    m_MinContrastValue = MinContrastValue;
    m_MaxContrastValue = MaxContrastValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpLightnessContrastStretchFilter::GetGammaFactor(double& GammaFactor) const
    {
    GammaFactor = m_GammaFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpLightnessContrastStretchFilter::SetGammaFactor(double GammaFactor)
    {
    HPRECONDITION(GammaFactor <= 100.0 && GammaFactor >= 0.0);

    m_GammaFactor = GammaFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpLightnessContrastStretchFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;  // We only have one possible input when output is set.

    // If we have an output, input must be of same type.
    if(GetOutputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetOutputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }

    if (pixelTypeToMatch != NULL)
        {
        // Try to find the best match for pixelTypeToMatch. Try to preserve alpha and pixel depth.
        unsigned short pixelDepth = pixelTypeToMatch->GetChannelOrg().GetChannelPtr(0)->GetSize();
        if (pixelTypeToMatch->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
            {
            if (pixelDepth <= 8)
                {
                pixelType = new HRPPixelTypeV32R8G8B8A8();
                return IMAGEPP_STATUS_Success;
                }
            else
                {
                pixelType = new HRPPixelTypeV64R16G16B16A16();
                return IMAGEPP_STATUS_Success;
                }
            }
        else
            {
            // Suggest an input with no alpha channel.
            if (pixelDepth <= 8)
                {
                pixelType = new HRPPixelTypeV24R8G8B8();
                return IMAGEPP_STATUS_Success;
                }
            else
                {
                pixelType = new HRPPixelTypeV48R16G16B16();
                return IMAGEPP_STATUS_Success;
                }
            }
        }
    else
        {
        // No pixel type to match. Return common pixel type.
        pixelType = new HRPPixelTypeV32R8G8B8A8();
        return IMAGEPP_STATUS_Success;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpLightnessContrastStretchFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;  // We only have one possible output when input is set.

    // If we have an input, output must be of same type.
    if(GetInputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }

    // Try to find the best match for pixelTypeToMatch. Try to preserve alpha and pixel depth.
    if (pixelTypeToMatch != NULL)
        {
        unsigned short pixelDepth = pixelTypeToMatch->GetChannelOrg().GetChannelPtr(0)->GetSize();
        if (pixelTypeToMatch->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
            {
            if (pixelDepth <= 8)
                {
                pixelType = new HRPPixelTypeV32R8G8B8A8();
                return IMAGEPP_STATUS_Success;
                }
            else
                {
                pixelType = new HRPPixelTypeV64R16G16B16A16();
                return IMAGEPP_STATUS_Success;
                }
            }
        else
            {
            // Suggest an input with no alpha channel.
            if (pixelDepth <= 8)
                {
                pixelType = new HRPPixelTypeV24R8G8B8();
                return IMAGEPP_STATUS_Success;
                }
            else
                {
                pixelType = new HRPPixelTypeV48R16G16B16();
                return IMAGEPP_STATUS_Success;
                }
            }
        }
    else
        {
        // No pixel type to match. Return common pixel type.
        pixelType = new HRPPixelTypeV32R8G8B8A8();
        return IMAGEPP_STATUS_Success;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpLightnessContrastStretchFilter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pInputPixelType = NULL;
        UpdateFilter();
        return IMAGEPP_STATUS_Success;
        }

    if (pixelType->CountIndexBits() > 0)
        // We don't process indexed pixel types (i.e. with a palette)
        return IMAGEOP_STATUS_InvalidPixelType;

    // If OUTPUT is already provided they must be of same type.
    if(GetOutputPixelType() != NULL && !GetOutputPixelType()->HasSamePixelInterpretation(*pixelType))
        return IMAGEOP_STATUS_InvalidPixelType;

    if(!IsSupportedPixeltype(*pixelType))
        return IMAGEOP_STATUS_InvalidPixelType;

    m_pInputPixelType = pixelType;
    
    UpdateFilter();

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpLightnessContrastStretchFilter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pOutputPixelType = NULL;
        UpdateFilter();
        return IMAGEPP_STATUS_Success;
        }

    if (pixelType->CountIndexBits() > 0)
        // We don't process indexed pixel types (i.e. with a palette)
        return IMAGEOP_STATUS_InvalidPixelType;

    // If INPUT is already provided they must be of same type.
    if(GetInputPixelType() != NULL && !GetInputPixelType()->HasSamePixelInterpretation(*pixelType))
        return IMAGEOP_STATUS_InvalidPixelType;
        
    if(!IsSupportedPixeltype(*pixelType))
        return IMAGEOP_STATUS_InvalidPixelType;

    m_pOutputPixelType = pixelType;

    UpdateFilter();

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpLightnessContrastStretchFilter::UpdateFilter()
    {
    // Reset filter in any case; even if inputPixelType or outputPixelType is NULL
    m_pFilter.reset(NULL);

    if(GetInputPixelType() == NULL || GetOutputPixelType() == NULL)
        return IMAGEPP_STATUS_UnknownError;     // Not ready.

    switch(GetInputPixelType()->GetClassID())
        {
        case HRPPixelTypeId_V8Gray8:
            m_pFilter.reset(new LightnessContrastStretchFilter_T<uint8_t, 1>(m_MinValue, m_MaxValue, m_MinContrastValue, m_MaxContrastValue, m_GammaFactor));
            break;

        case HRPPixelTypeId_V16Gray16:
            m_pFilter.reset(new LightnessContrastStretchFilter_T<uint16_t, 1>(m_MinValue, m_MaxValue, m_MinContrastValue, m_MaxContrastValue, m_GammaFactor));
            break;

        case HRPPixelTypeId_V24R8G8B8:
            m_pFilter.reset(new LightnessContrastStretchFilter_T<uint8_t, 3>(m_MinValue, m_MaxValue, m_MinContrastValue, m_MaxContrastValue, m_GammaFactor));
            break;

        case HRPPixelTypeId_V48R16G16B16:
            m_pFilter.reset(new LightnessContrastStretchFilter_T<uint16_t, 3>(m_MinValue, m_MaxValue, m_MinContrastValue, m_MaxContrastValue, m_GammaFactor));
            break;

        case HRPPixelTypeId_V32R8G8B8A8:
            m_pFilter.reset(new LightnessContrastStretchFilter_T<uint8_t, 4>(m_MinValue, m_MaxValue, m_MinContrastValue, m_MaxContrastValue, m_GammaFactor));
            break;

        case HRPPixelTypeId_V64R16G16B16A16:
            m_pFilter.reset(new LightnessContrastStretchFilter_T<uint16_t, 4>(m_MinValue, m_MaxValue, m_MinContrastValue, m_MaxContrastValue, m_GammaFactor));
            break;

        default:
            BeAssertOnce(!"HRAImageOpLightnessContrastStretchFilter::UpdateFilter unexpected pixeltype");
            m_pFilter.reset(NULL);
            break;
        }

    return m_pFilter.get() == NULL ? IMAGEPP_STATUS_UnknownError : IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpLightnessContrastStretchFilter::IsSupportedPixeltype(HRPPixelType const& pixeltype) const
    {
    switch(pixeltype.GetClassID())
        {
        case HRPPixelTypeId_V8Gray8:
        case HRPPixelTypeId_V16Gray16:
        case HRPPixelTypeId_V24R8G8B8:
        case HRPPixelTypeId_V48R16G16B16:
        case HRPPixelTypeId_V32R8G8B8A8:
        case HRPPixelTypeId_V64R16G16B16A16:
            return true;
        default:
            break;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpLightnessContrastStretchFilter::_Process(HRAImageSampleR outData, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
    HPRECONDITION(GetInputPixelType() != NULL && GetOutputPixelType() != NULL);
    HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
    HPRECONDITION(inputData.GetWidth() == outData.GetWidth() + GetNeighbourhood().GetWidth() - 1);
    HPRECONDITION(inputData.GetHeight() == outData.GetHeight() + GetNeighbourhood().GetHeight() - 1);

    return m_pFilter->_Apply(outData, inputData);
    }
 