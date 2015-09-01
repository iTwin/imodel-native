//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageOpDensitySlicingFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>
#include <Imagepp/all/h/HRAImageOpDensitySlicingFilter.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <Imagepp/all/h/HGFLightnessColorSpace.h>




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
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<size_t depth>
struct If_16bitsDepth 
    {
    static inline void ScaleColor(int32_t& color){}
    };

template<>
struct If_16bitsDepth< 16 > 
    {
    static inline void ScaleColor(int32_t& color) {color <<= 8;}
    };

/*---------------------------------------------------------------------------------**//**
* struct DensitySlicingFilter_T                                               
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T, uint32_t ChannelCount_T>
struct DensitySlicingFilter_T : HRAImageOpBaseDensitySlicingFilter::DensitySlicingFilter
{
public:
    DensitySlicingFilter_T(HRAImageOpDensitySlicingFilter::SliceList const& sliceList, double desaturationFactor)
        :m_SliceList(sliceList),      // a const reference to sliceList 
        m_DesaturationFactor(desaturationFactor)
        {
        }   

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Jonathan.Bernier  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImagePPStatus _Apply(HRAImageSampleR outData, HRAImageSampleCR inputData) override
        {
        HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);

#ifndef DISABLE_BUFFER_GETDATA

        size_t inPitch, outPitch;
        Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

        uint32_t Width = outData.GetWidth();
        uint32_t Height = outData.GetHeight();

        for(uint32_t row=0; row < Height; ++row)
            {
            Data_T const* pInBufferRow  = (Data_T const*)(pInBuffer+row*inPitch);
            Data_T* pOutBufferRow = (Data_T*)(pOutBuffer+row*outPitch);
            int32_t RedValue = 0;
            int32_t GreenValue = 0;
            int32_t BlueValue = 0;
            double Opacity= 0.0;

            for(uint32_t column=0; column < Width; ++column)
                {
                uint32_t GrayValue = (uint32_t)(pInBufferRow[column*ChannelCount_T]*REDFACTOR + pInBufferRow[column*ChannelCount_T+1]*GREENFACTOR + pInBufferRow[column*ChannelCount_T+2]*BLUEFACTOR);

                bool IsSliced = false;
                HRAImageOpDensitySlicingFilter::SliceList::const_iterator SliceItr = m_SliceList.begin();
                while (!IsSliced && SliceItr != m_SliceList.end())
                    {
                    if (GrayValue >= (*SliceItr).m_StartValue && GrayValue <= (*SliceItr).m_EndValue)
                        {
                        double GradientFactor = (GrayValue - (*SliceItr).m_StartValue) / (double)(MAX((*SliceItr).m_EndValue - (*SliceItr).m_StartValue, 1.0));

                        RedValue = (int32_t)(((((*SliceItr).m_StartColor& 0x00FF0000) >> 16)*(1 - GradientFactor)) +
                                           ((((*SliceItr).m_EndColor& 0x00FF0000) >> 16)*GradientFactor));

                        GreenValue = (int32_t)(((((*SliceItr).m_StartColor& 0x0000FF00) >> 8)*(1 - GradientFactor)) +
                                             ((((*SliceItr).m_EndColor& 0x0000FF00) >> 8)*GradientFactor));

                        BlueValue = (int32_t)((((*SliceItr).m_StartColor& 0x000000FF)*(1 - GradientFactor)) +
                                            (((*SliceItr).m_EndColor& 0x000000FF)*GradientFactor));

                        If_16bitsDepth<sizeof(Data_T)*8>::ScaleColor(RedValue);
                        If_16bitsDepth<sizeof(Data_T)*8>::ScaleColor(GreenValue);
                        If_16bitsDepth<sizeof(Data_T)*8>::ScaleColor(BlueValue);

                        BeAssert(RedValue <= std::numeric_limits<Data_T>::max());
                        BeAssert(GreenValue <= std::numeric_limits<Data_T>::max());
                        BeAssert(BlueValue <= std::numeric_limits<Data_T>::max());

                        Opacity = (*SliceItr).m_Opacity;

                        // Indicate the current pixel is in this slide, and break the loop.
                        IsSliced = true;
                        }
                    else
                        SliceItr++;
                    }

                if (IsSliced)
                    {
                    pOutBufferRow[column*ChannelCount_T]   = (Data_T)(pInBufferRow[column*ChannelCount_T]*(1-Opacity) + RedValue*Opacity);
                    pOutBufferRow[column*ChannelCount_T+1] = (Data_T)(pInBufferRow[column*ChannelCount_T+1]*(1-Opacity) + GreenValue*Opacity);
                    pOutBufferRow[column*ChannelCount_T+2] = (Data_T)(pInBufferRow[column*ChannelCount_T+2]*(1-Opacity) + BlueValue*Opacity);
                    }
                else
                    {
                    pOutBufferRow[column*ChannelCount_T]   = (Data_T)(pInBufferRow[column*ChannelCount_T]*(1.0 - m_DesaturationFactor) + (GrayValue*m_DesaturationFactor));
                    pOutBufferRow[column*ChannelCount_T+1] = (Data_T)(pInBufferRow[column*ChannelCount_T+1]*(1.0 - m_DesaturationFactor) + (GrayValue*m_DesaturationFactor));
                    pOutBufferRow[column*ChannelCount_T+2] = (Data_T)(pInBufferRow[column*ChannelCount_T+2]*(1.0 - m_DesaturationFactor) + (GrayValue*m_DesaturationFactor));
                    }

                // Recopy or skip alpha channel.
                If_Rgba<Data_T, ChannelCount_T>::CopyAlphaChannel(pOutBufferRow, pInBufferRow, column);
                }
            }
#endif

        return IMAGEPP_STATUS_Success;
        }

    double m_DesaturationFactor;
    HRAImageOpDensitySlicingFilter::SliceList const& m_SliceList;       // A ref to the list held by HRAImageOpDensitySlicingFilter
};


/*---------------------------------------------------------------------------------**//**
* struct LightnessDensitySlicingFilter_T                                               
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T, uint32_t ChannelCount_T>
struct LightnessDensitySlicingFilter_T : HRAImageOpBaseDensitySlicingFilter::DensitySlicingFilter
{
public:
    LightnessDensitySlicingFilter_T(HRAImageOpLightnessDensitySlicingFilter::SliceList const& sliceList, double desaturationFactor)
        :m_ColorSpaceConverter(DEFAULT_GAMMA_FACTOR, sizeof(Data_T)*8),
         m_DesaturationFactor(desaturationFactor),
         m_SliceList(sliceList)  // take a const ref to it.
        {
        }   

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Jonathan.Bernier  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImagePPStatus _Apply(HRAImageSampleR outData, HRAImageSampleCR inputData) override
        {
#ifndef DISABLE_BUFFER_GETDATA
        HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);

        size_t inPitch, outPitch;
        Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

        uint32_t Width = outData.GetWidth();
        uint32_t Height = outData.GetHeight();

        // desaturation of lightness[0,100] to color [0, dataTypeMax]
        double lightnessToColorDesaturation = m_DesaturationFactor*std::numeric_limits<Data_T>::max() / 100.0;

        for(uint32_t row=0; row < Height; ++row)
            {
            Data_T const* pInBufferRow  = (Data_T*)(pInBuffer+row*inPitch);
            Data_T* pOutBufferRow = (Data_T*)(pOutBuffer+row*outPitch);
            int32_t RedValue = 0;
            int32_t GreenValue = 0;
            int32_t BlueValue = 0;
            double Opacity= 0.0;

            for(uint32_t column=0; column < Width; ++column)
                {
                float lightnessValue = (float)m_ColorSpaceConverter.ConvertFromRGB(pInBufferRow[column*ChannelCount_T], pInBufferRow[column*ChannelCount_T+1], pInBufferRow[column*ChannelCount_T+2]);
                BeAssertOnce(IN_RANGE(lightnessValue, 0.0, 100.0));

                bool IsSliced = false;
                HRAImageOpLightnessDensitySlicingFilter::SliceList::const_iterator SliceItr = m_SliceList.begin();
                while (!IsSliced && SliceItr != m_SliceList.end())
                    {
                    if (lightnessValue >= (*SliceItr).m_StartValue && lightnessValue <= (*SliceItr).m_EndValue)
                        {
                        double GradientFactor = (lightnessValue - (*SliceItr).m_StartValue) / (double)(MAX((*SliceItr).m_EndValue - (*SliceItr).m_StartValue, 1.0));

                        RedValue = (int32_t)(((((*SliceItr).m_StartColor& 0x00FF0000) >> 16)*(1 - GradientFactor)) +
                                            ((((*SliceItr).m_EndColor& 0x00FF0000) >> 16)*GradientFactor));

                        GreenValue = (int32_t)(((((*SliceItr).m_StartColor& 0x0000FF00) >> 8)*(1 - GradientFactor)) +
                                             ((((*SliceItr).m_EndColor& 0x0000FF00) >> 8)*GradientFactor));

                        BlueValue = (int32_t)((((*SliceItr).m_StartColor& 0x000000FF)*(1 - GradientFactor)) +
                                             (((*SliceItr).m_EndColor& 0x000000FF)*GradientFactor));

                        If_16bitsDepth<sizeof(Data_T)*8>::ScaleColor(RedValue);
                        If_16bitsDepth<sizeof(Data_T)*8>::ScaleColor(GreenValue);
                        If_16bitsDepth<sizeof(Data_T)*8>::ScaleColor(BlueValue);

                        BeAssert(RedValue <= std::numeric_limits<Data_T>::max());
                        BeAssert(GreenValue <= std::numeric_limits<Data_T>::max());
                        BeAssert(BlueValue <= std::numeric_limits<Data_T>::max());

                        Opacity = (*SliceItr).m_Opacity;

                        // Indicate the current pixel is in this slide, and break the loop.
                        IsSliced = true;
                        }
                    else
                        SliceItr++;
                    }

                if (IsSliced)
                    {
                    pOutBufferRow[column*ChannelCount_T]   = (Data_T)(pInBufferRow[column*ChannelCount_T]  *(1-Opacity) + RedValue*Opacity);
                    pOutBufferRow[column*ChannelCount_T+1] = (Data_T)(pInBufferRow[column*ChannelCount_T+1]*(1-Opacity) + GreenValue*Opacity);
                    pOutBufferRow[column*ChannelCount_T+2] = (Data_T)(pInBufferRow[column*ChannelCount_T+2]*(1-Opacity) + BlueValue*Opacity);
                    }
                else
                    {
                    pOutBufferRow[column*ChannelCount_T]   = (Data_T)(pInBufferRow[column*ChannelCount_T]  *(1.0 - m_DesaturationFactor) + lightnessValue*lightnessToColorDesaturation);
                    pOutBufferRow[column*ChannelCount_T+1] = (Data_T)(pInBufferRow[column*ChannelCount_T+1]*(1.0 - m_DesaturationFactor) + lightnessValue*lightnessToColorDesaturation);
                    pOutBufferRow[column*ChannelCount_T+2] = (Data_T)(pInBufferRow[column*ChannelCount_T+2]*(1.0 - m_DesaturationFactor) + lightnessValue*lightnessToColorDesaturation);
                    }

                // Recopy or skip alpha channel.
                If_Rgba<Data_T, ChannelCount_T>::CopyAlphaChannel(pOutBufferRow, pInBufferRow, column);
                }
            }
         
#endif
        return IMAGEPP_STATUS_Success;
        }

    double m_DesaturationFactor;
    HGFLightnessColorSpace m_ColorSpaceConverter;
    HRAImageOpLightnessDensitySlicingFilter::SliceList const& m_SliceList; // A ref to the list held by HRAImageOpLightnessDensitySlicingFilter
};


/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------HRAImageOpBaseDensitySlicingFilter---------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpBaseDensitySlicingFilter::HRAImageOpBaseDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PixelDepth depth)
    :HRAImageOp(new HRPPixelNeighbourhood(1,1,0,0)),
    m_DesaturationFactor(0.0),
    m_pixelDepth(depth)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpBaseDensitySlicingFilter::~HRAImageOpBaseDensitySlicingFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpBaseDensitySlicingFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;  // We only have one possible input.

    // If we have an output, input must be of same type.
    if(GetOutputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetOutputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }

    // Choice is made according to m_pixelDepth and pixelTypeToMatch.
    if (pixelTypeToMatch != NULL)
        {
        if (pixelTypeToMatch->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
            {
            // Pixel type to match has an alpha channel
            if(m_pixelDepth <= PIXELDEPTH_8bits)
                pixelType = new HRPPixelTypeV32R8G8B8A8();
            else
                pixelType = new HRPPixelTypeV64R16G16B16A16();
    
            return IMAGEPP_STATUS_Success;
            }
        else
            {
            // Pixel type to match has no alpha channel
            if(m_pixelDepth <= PIXELDEPTH_8bits)
                pixelType = new HRPPixelTypeV24R8G8B8();
            else
                pixelType = new HRPPixelTypeV48R16G16B16();
    
            return IMAGEPP_STATUS_Success;
            }
        }
    else
        {
        // No pixel type to match
        if(m_pixelDepth <= PIXELDEPTH_8bits)
            pixelType = new HRPPixelTypeV32R8G8B8A8();
        else
            pixelType = new HRPPixelTypeV64R16G16B16A16();
        return IMAGEPP_STATUS_Success;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpBaseDensitySlicingFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;  // We only have one possible output.

    // If we have an input, output must be of same type.
    if(GetInputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }

    // Choice is made according to m_pixelDepth and pixelTypeToMatch.
    if (pixelTypeToMatch != NULL)
        {
        if (pixelTypeToMatch->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
            {
            // Pixel type to match has an alpha channel
            if(m_pixelDepth <= PIXELDEPTH_8bits)
                pixelType = new HRPPixelTypeV32R8G8B8A8();
            else
                pixelType = new HRPPixelTypeV64R16G16B16A16();
    
            return IMAGEPP_STATUS_Success;
            }
        else
            {
            // Pixel type to match has no alpha channel
            if(m_pixelDepth <= PIXELDEPTH_8bits)
                pixelType = new HRPPixelTypeV24R8G8B8();
            else
                pixelType = new HRPPixelTypeV48R16G16B16();
    
            return IMAGEPP_STATUS_Success;
            }
        }
    else
        {
        // No pixel type to match
        if(m_pixelDepth <= PIXELDEPTH_8bits)
            pixelType = new HRPPixelTypeV32R8G8B8A8();
        else
            pixelType = new HRPPixelTypeV64R16G16B16A16();
        return IMAGEPP_STATUS_Success;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpBaseDensitySlicingFilter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pInputPixelType = NULL;
        _UpdateFilter();
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
    
    _UpdateFilter();

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpBaseDensitySlicingFilter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pOutputPixelType = NULL;
        _UpdateFilter();
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

    _UpdateFilter();

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpBaseDensitySlicingFilter::IsSupportedPixeltype(HRPPixelType const& pixeltype) const
    {
    switch(pixeltype.GetClassID())
        {
        case HRPPixelTypeId_V24R8G8B8:
        case HRPPixelTypeId_V48R16G16B16:
        case HRPPixelTypeId_V32R8G8B8A8:
        case HRPPixelTypeId_V64R16G16B16A16:
            // Accept if pixel type have same depth
            return pixeltype.GetChannelOrg().GetChannelPtr(0)->GetSize() == m_pixelDepth;
        default:
            break;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpBaseDensitySlicingFilter::_Process(HRAImageSampleR outData, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
    HPRECONDITION(GetInputPixelType() != NULL && GetOutputPixelType() != NULL);
    HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
    HPRECONDITION(inputData.GetWidth() == outData.GetWidth() + GetNeighbourhood().GetWidth() - 1);
    HPRECONDITION(inputData.GetHeight() == outData.GetHeight() + GetNeighbourhood().GetHeight() - 1);

    return m_pFilter->_Apply(outData, inputData);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpBaseDensitySlicingFilter::SetDesaturationFactor(double DesaturationFactor)
    {
    HPRECONDITION(IN_RANGE(DesaturationFactor, 0.0, 1.0));

    m_DesaturationFactor = BOUND(DesaturationFactor, 0.0, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
double HRAImageOpBaseDensitySlicingFilter::GetDesaturationFactor() const
    {
    return m_DesaturationFactor;
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------HRAImageOpDensitySlicingFilter------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpDensitySlicingFilterPtr HRAImageOpDensitySlicingFilter::CreateDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PixelDepth depth)
    {  
    HRAImageOpDensitySlicingFilter* pFilter = new HRAImageOpDensitySlicingFilter(depth);
    return pFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpDensitySlicingFilter::HRAImageOpDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PixelDepth depth) 
    :HRAImageOpBaseDensitySlicingFilter(depth)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpDensitySlicingFilter::~HRAImageOpDensitySlicingFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t HRAImageOpDensitySlicingFilter::AddSlice(uint32_t StartValue, uint32_t EndValue, uint32_t StartColor, uint32_t EndColor, double Opacity)
    {
    HPRECONDITION(StartValue <= EndValue);
    HPRECONDITION(IN_RANGE(Opacity, 0, 1.0));
    HPRECONDITION(StartValue >= 0 && m_pixelDepth == PIXELDEPTH_8bits ? EndValue <= 0xFF : EndValue <= 0xFFFF);

    SliceInfos NewSlice;

    NewSlice.m_StartValue = StartValue;
    NewSlice.m_EndValue = EndValue;
    NewSlice.m_StartColor = StartColor;
    NewSlice.m_EndColor = EndColor;
    NewSlice.m_Opacity = Opacity;

    m_SliceList.push_back(NewSlice);

    return (uint32_t)m_SliceList.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpDensitySlicingFilter::_UpdateFilter()
    {
    // Reset filter in any case; even if inputPixelType or outputPixelType is NULL
    m_pFilter.reset(NULL);

    if(GetInputPixelType() == NULL || GetOutputPixelType() == NULL)
        return IMAGEPP_STATUS_UnknownError;     // Not ready.

    switch(GetInputPixelType()->GetClassID())
        {
        case HRPPixelTypeId_V24R8G8B8:
            m_pFilter.reset(new DensitySlicingFilter_T<uint8_t, 3>(m_SliceList, m_DesaturationFactor));
            break;

        case HRPPixelTypeId_V48R16G16B16:
            m_pFilter.reset(new DensitySlicingFilter_T<uint16_t, 3>(m_SliceList, m_DesaturationFactor));
            break;

        case HRPPixelTypeId_V32R8G8B8A8:
            m_pFilter.reset(new DensitySlicingFilter_T<uint8_t, 4>(m_SliceList, m_DesaturationFactor));
            break;

        case HRPPixelTypeId_V64R16G16B16A16:
            m_pFilter.reset(new DensitySlicingFilter_T<uint16_t, 4>(m_SliceList, m_DesaturationFactor));
            break;

        default:
            BeAssertOnce(!"HRAImageOpDensitySlicingFilter::UpdateFilter unexpected pixeltype");
            m_pFilter.reset(NULL);
            break;
        }
    
    return m_pFilter.get() == NULL ? IMAGEPP_STATUS_UnknownError : IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpDensitySlicingFilter::SliceList const& HRAImageOpDensitySlicingFilter::GetSlices() const
    {
    return m_SliceList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpDensitySlicingFilter::ClearSlices()
    {
    m_SliceList.clear();
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------HRAImageOpLightnessDensitySlicingFilter------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpLightnessDensitySlicingFilterPtr HRAImageOpLightnessDensitySlicingFilter::CreateLightnessDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PixelDepth depth)
    {  
    return new HRAImageOpLightnessDensitySlicingFilter(depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpLightnessDensitySlicingFilter::HRAImageOpLightnessDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PixelDepth depth)
    :HRAImageOpBaseDensitySlicingFilter(depth)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpLightnessDensitySlicingFilter::~HRAImageOpLightnessDensitySlicingFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t HRAImageOpLightnessDensitySlicingFilter::AddSlice(float StartValue, float EndValue, uint32_t StartColor, uint32_t EndColor, double Opacity)
    {
    HPRECONDITION(StartValue <= EndValue);
    HPRECONDITION(StartValue >= 0.0 && EndValue <= 100.0);
    HPRECONDITION(Opacity >= 0 && Opacity <= 1.0);

    SliceInfos NewSlice;

    NewSlice.m_StartValue = StartValue;
    NewSlice.m_EndValue = EndValue;
    NewSlice.m_StartColor = StartColor;
    NewSlice.m_EndColor = EndColor;
    NewSlice.m_Opacity = Opacity;

    m_SliceList.push_back(NewSlice);

    return (uint32_t)m_SliceList.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpLightnessDensitySlicingFilter::SliceList const& HRAImageOpLightnessDensitySlicingFilter::GetSlices() const
    {
    return m_SliceList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpLightnessDensitySlicingFilter::ClearSlices()
    {
    m_SliceList.clear();
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpLightnessDensitySlicingFilter::_UpdateFilter()
    {
    // Reset filter in any case; even if inputPixelType or outputPixelType is NULL
    m_pFilter.reset(NULL);

    if(GetInputPixelType() == NULL || GetOutputPixelType() == NULL)
        return IMAGEPP_STATUS_UnknownError;     // Not ready.

    switch(GetInputPixelType()->GetClassID())
        {
        case HRPPixelTypeId_V24R8G8B8:
            m_pFilter.reset(new LightnessDensitySlicingFilter_T<uint8_t, 3>(m_SliceList, m_DesaturationFactor));
            break;

        case HRPPixelTypeId_V48R16G16B16:
            m_pFilter.reset(new LightnessDensitySlicingFilter_T<uint16_t, 3>(m_SliceList, m_DesaturationFactor));
            break;

        case HRPPixelTypeId_V32R8G8B8A8:
            m_pFilter.reset(new LightnessDensitySlicingFilter_T<uint8_t, 4>(m_SliceList, m_DesaturationFactor));
            break;

        case HRPPixelTypeId_V64R16G16B16A16:
            m_pFilter.reset(new LightnessDensitySlicingFilter_T<uint16_t, 4>(m_SliceList, m_DesaturationFactor));
            break;

        default:
            BeAssertOnce(!"HRAImageOpDensitySlicingFilter::UpdateFilter unexpected pixeltype");
            m_pFilter.reset(NULL);
            break;
        }
    
    return m_pFilter.get() == NULL ? IMAGEPP_STATUS_UnknownError : IMAGEPP_STATUS_Success;
    }