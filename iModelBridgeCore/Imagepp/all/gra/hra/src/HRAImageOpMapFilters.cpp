//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageOpMapFilters.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <ImagePP/all/h/HRAImageOpMapFilters.h>
#include <ImagePP/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRPPixelConverter.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>



/*-------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------HRAImageOpMapFilter----------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct MapChannel_T
    {
    template<class Data_T, uint32_t ChannelIndex_T, uint32_t ChannelCount_T>
    static inline void ProcessChannel(Data_T* pOutData, Data_T const* pInData, Data_T* const pMap[], uint32_t index)
        {
        pOutData[index*ChannelCount_T+ChannelIndex_T] = pMap[ChannelIndex_T][pInData[index*ChannelCount_T+ChannelIndex_T]];
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CopyChannel_T
    {
    template<class Data_T, uint32_t ChannelIndex_T, uint32_t ChannelCount_T> 
    static inline void ProcessChannel(Data_T* pOutData, Data_T const* pInData, Data_T* const pMap[], uint32_t index)
        {
        pOutData[index*ChannelCount_T+ChannelIndex_T] = pInData[index*ChannelCount_T+ChannelIndex_T];
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SkipChannel_T
    {
    template<class Data_T, uint32_t ChannelIndex_T, uint32_t ChannelCount_T> 
    static inline void ProcessChannel(Data_T* pOutData, Data_T const* pInData, Data_T* const pMap[], uint32_t index)
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* struct MapFilterMap_T                                               
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T, uint32_t ChannelCount_T, class Channel1_T, class Channel2_T=SkipChannel_T, class Channel3_T=SkipChannel_T, class Channel4_T=SkipChannel_T >
struct MapFilterMap_T : HRAImageOpMapFilter::MapFilter
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    MapFilterMap_T()
        {
        for(uint32_t channel=0; channel < ChannelCount_T; ++channel)
            {
            m_map[channel] = NULL;
            m_allocMap[channel] = NULL;
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ~MapFilterMap_T()
        {
        for(uint32_t channel=0; channel < ChannelCount_T; ++channel)
            {
            if(m_allocMap[channel] != NULL)
                delete [] m_allocMap[channel];
            }           
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void*   _AllocateChannelMap() override
        {
        for(uint32_t channel=0; channel < ChannelCount_T; ++channel)
            {
            if(m_allocMap[channel] == NULL)
                {
                m_allocMap[channel] = new Data_T[std::numeric_limits<Data_T>::max()+1];
                return m_allocMap[channel];
                }
            }

        HASSERT(!"MapFilterMap_T::AllocatedMap: Out of range");
        return NULL;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _SetMapReference(void* pMapRef, uint32_t channelIndex) override
        {
        HPRECONDITION(channelIndex < ChannelCount_T);
        m_map[channelIndex] = (Data_T*)pMapRef;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImagePPStatus _ApplyMap(HRAImageSampleR outData, HRAImageSampleCR inputData) const override
        {
#ifndef DISABLE_BUFFER_GETDATA
        size_t inPitch, outPitch;
        Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

        uint32_t outWidth = outData.GetWidth();
        uint32_t outHeight = outData.GetHeight();

        for(uint32_t line=0; line < outHeight; ++line)
            {
            Data_T const* pInBufferLine  = (Data_T const*)(pInBuffer+line*inPitch);
            Data_T*       pOutBufferLine = (Data_T*)(pOutBuffer+line*outPitch);

            for(uint32_t column=0; column < outWidth; ++column)
                {
                Channel1_T::template ProcessChannel<Data_T, 0, ChannelCount_T>(pOutBufferLine, pInBufferLine, m_map, column);
                Channel2_T::template ProcessChannel<Data_T, 1, ChannelCount_T>(pOutBufferLine, pInBufferLine, m_map, column);
                Channel3_T::template ProcessChannel<Data_T, 2, ChannelCount_T>(pOutBufferLine, pInBufferLine, m_map, column);
                Channel4_T::template ProcessChannel<Data_T, 3, ChannelCount_T>(pOutBufferLine, pInBufferLine, m_map, column);
                }
            }
#endif
        return IMAGEPP_STATUS_Success;
        }

    Data_T* m_map[ChannelCount_T];          // Shared (or not) map ref. May come from m_allocMap or user buffers.
    Data_T* m_allocMap[ChannelCount_T];     // Allocated map that we owned memory.
    };

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------HRAImageOpMapFilter--------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpMapFilter::MapFilter::MapFilter(){};
HRAImageOpMapFilter::MapFilter::~MapFilter(){};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpMapFilter::HRAImageOpMapFilter()
    :HRAImageOp(new HRPPixelNeighbourhood(1,1,0,0))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpMapFilter::~HRAImageOpMapFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpMapFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;  // No other suggestion.

    // If we have an output, input must be of same type.
    if(GetOutputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetOutputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }

    return GetMatchingPixelType(pixelType, pixelTypeToMatch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpMapFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;  // No other suggestion.

    // If we have an output, input must be of same type.
    if(GetInputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }

    return GetMatchingPixelType(pixelType, pixelTypeToMatch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpMapFilter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
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
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpMapFilter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
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
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpMapFilter::_Process(HRAImageSampleR outData, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
    HPRECONDITION(GetInputPixelType() != NULL && GetOutputPixelType() != NULL);
    HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
    HPRECONDITION(inputData.GetWidth() == outData.GetWidth() + GetNeighbourhood().GetWidth() - 1);
    HPRECONDITION(inputData.GetHeight() == outData.GetHeight() + GetNeighbourhood().GetHeight() - 1);

    return m_pFilter->_ApplyMap(outData, inputData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpMapFilter::IsSupportedPixeltype(HRPPixelType const& pixelType) const
    {
    std::unique_ptr<MapFilter> pFilter(CreateFilter(pixelType));

    return pFilter.get() != NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpMapFilter::MapFilter* HRAImageOpMapFilter::CreateFilter(HRPPixelType const& pixelType) const
    {       
    HRAImageOpMapFilter::MapFilter* pFilter = NULL;
    
    switch(pixelType.GetClassID())
        {
        case HRPPixelTypeId_V8Gray8:
        case HRPPixelTypeId_V8GrayWhite8:
            pFilter = new MapFilterMap_T<uint8_t, 1, MapChannel_T>();
            break;

        case HRPPixelTypeId_V16Gray16:
            pFilter = new MapFilterMap_T<uint16_t, 1, MapChannel_T>();
            break;
       
        case HRPPixelTypeId_V24R8G8B8:
        case HRPPixelTypeId_V24B8G8R8:
            pFilter = new MapFilterMap_T<uint8_t, 3, MapChannel_T, MapChannel_T, MapChannel_T>();
            break;

        case HRPPixelTypeId_V48R16G16B16:
            pFilter = new MapFilterMap_T<uint16_t, 3, MapChannel_T, MapChannel_T, MapChannel_T>();
            break;

        case HRPPixelTypeId_V32A8R8G8B8:
            pFilter = new MapFilterMap_T<uint8_t, 4, CopyChannel_T, MapChannel_T, MapChannel_T, MapChannel_T>();
            break;

        case HRPPixelTypeId_V32R8G8B8A8:
        case HRPPixelTypeId_V32B8G8R8X8:
        case HRPPixelTypeId_V32R8G8B8X8:
            pFilter = new MapFilterMap_T<uint8_t, 4, MapChannel_T, MapChannel_T, MapChannel_T, CopyChannel_T>();
            break;
        
        case HRPPixelTypeId_V64R16G16B16A16:
        case HRPPixelTypeId_V64R16G16B16X16:
            pFilter = new MapFilterMap_T<uint16_t, 4, MapChannel_T, MapChannel_T, MapChannel_T, CopyChannel_T>();
            break;

        // *** Not supported. ***
        // N.B. Be careful when adding support for a new pixeltype.  That will enable this pixeltype for all map filters.
        //      For now, only gray or RGB channel org are supported.
        //case HRPPixelTypeId_I1R8G8B8:
        //case HRPPixelTypeId_I1R8G8B8:
        //case HRPPixelTypeId_I1R8G8B8RLE:
        //case HRPPixelTypeId_I1R8G8B8A8:
        //case HRPPixelTypeId_I1R8G8B8A8RLE:
        //case HRPPixelTypeId_I2R8G8B8:
        //case HRPPixelTypeId_I4R8G8B8:
        //case HRPPixelTypeId_I4R8G8B8A8:
        //case HRPPixelTypeId_I8R8G8B8:
        //case HRPPixelTypeId_I8R8G8B8A8:
        //case HRPPixelTypeId_I8VA8R8G8B8:    
        //case HRPPixelTypeId_I8Gray8:
        //case HRPPixelTypeId_V1Gray1:
        //case HRPPixelTypeId_V1GrayWhite1:
        //case HRPPixelTypeId_V16Int16:      >> This is a signed pixel type.  Not sure it make sense to support this for our map filter.
        //case HRPPixelTypeId_V16B5G5R5:
        //case HRPPixelTypeId_V16R5G6B5:
        //case HRPPixelTypeId_I8R8G8B8Mask:
        //case HRPPixelTypeId_V32Float32:
        //case HRPPixelTypeId_V32CMYK:
        //case HRPPixelTypeId_V24PhotoYCC: 
        //case HRPPixelTypeId_V16PRGray8A8:          >>     Can't do Pre-mul alpha direclty
        //case HRPPixelTypeId_V32PRPhotoYCCA8:       >>     Can't do Pre-mul alpha direclty
        //case HRPPixelTypeId_V32PR8PG8PB8A8:        >>     Can't do Pre-mul alpha direclty
        //case HRPPixelTypeId_V96R32G32B32:
        default:
            pFilter = NULL;
            break;
        }

    return pFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpMapFilter::UpdateFilter()
    {
    if(GetInputPixelType() == NULL || GetOutputPixelType() == NULL)
        {
        m_pFilter.reset(NULL);
        return IMAGEPP_STATUS_UnknownError;     // Not ready.
        }
    
    m_pFilter.reset(CreateFilter(*GetInputPixelType()));

    if(m_pFilter.get() == NULL)
        return IMAGEOP_STATUS_InvalidPixelType;  
    
    if(!_SetupMapFilter(*m_pFilter, *GetInputPixelType()))
        {
        m_pFilter.reset(NULL);  
        return IMAGEPP_STATUS_UnknownError;
        }  
        
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpMapFilter::GetMatchingPixelType(HFCPtr<HRPPixelType>& pixelType, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if (pixelTypeToMatch != NULL)
        {
        // Try to find the best match for pixelTypeToMatch. Try to preserve alpha and pixel depth.
        unsigned short pixelDepth = pixelTypeToMatch->GetChannelOrg().GetChannelPtr(0)->GetSize();
        if (pixelTypeToMatch->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
            {
            // Pixel type to match has an alpha channel.
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
        // No pixel type to match
        pixelType = new HRPPixelTypeV32R8G8B8A8();
        return IMAGEPP_STATUS_Success;
        }
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------HRAImageOpGammaFilter------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpGammaFilter::CreateGammaFilter(double gamma)
    {
    HPRECONDITION(gamma != 0);
    return new HRAImageOpGammaFilter(gamma);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpGammaFilter::HRAImageOpGammaFilter(double gamma)
    :HRAImageOpMapFilter(),
     m_gamma(gamma)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpGammaFilter::~HRAImageOpGammaFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpGammaFilter::_SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType)
    {
    void* pMap = mapFilter._AllocateChannelMap();

    if(8 == pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize())
        CreateGammaMap_T<uint8_t>((uint8_t*)pMap);
    else if(16 == pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize())
        CreateGammaMap_T<uint16_t>((uint16_t*)pMap);
    else
        {
        BeAssert(!"HRAImageOpGammaFilter: Unexpected bitsPerPixel");
        return false;   // ERROR
        }

    // All channels use the same map.
    for(uint32_t index=0; index < pixelType.GetChannelOrg().CountChannels(); ++index)
        mapFilter._SetMapReference(pMap, index);
        
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T> 
void HRAImageOpGammaFilter::CreateGammaMap_T(Data_T* pMapBuffer)
    {
    BeAssert(0 != m_gamma);

    // Gamma formula:
    //  correctedValue = (pow(value/type_max_limit, 1.0 / gamma)*type_max_limit) + 0.499999
    double Power = 1.0 / m_gamma;

    uint32_t const typeMax = std::numeric_limits<Data_T>::max();

    for (uint32_t index = 0; index < typeMax+1; ++index)
        {
        double Intensity = static_cast<double>(index) / std::numeric_limits<Data_T>::max();
        pMapBuffer[index] = (Data_T)((std::numeric_limits<Data_T>::max() * pow(Intensity, Power) + 0.499999));
        }
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------HRAImageOpBrightnessFilter-------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpBrightnessFilter::CreateBrightnessFilter(double brightness)
    {
    return new HRAImageOpBrightnessFilter(brightness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpBrightnessFilter::HRAImageOpBrightnessFilter(double brightness)
    :HRAImageOpMapFilter()
    {
    m_brightness = brightness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpBrightnessFilter::~HRAImageOpBrightnessFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T> 
void HRAImageOpBrightnessFilter::CreateBrightnessMap_T(Data_T* pMapBuffer, double brightness)
    {   
    int32_t const typeMax = (int32_t)std::numeric_limits<Data_T>::max();

    int32_t variation = (int32_t)(brightness * typeMax);

    for (int32_t index = 0; index < typeMax+1; ++index)
        pMapBuffer[index] = (Data_T)BOUND(index + variation, 0, typeMax);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpBrightnessFilter::_SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType)
    {
    void* pMap = mapFilter._AllocateChannelMap();

    if(8 == pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize())
        CreateBrightnessMap_T<uint8_t>((uint8_t*)pMap, m_brightness);
    else if(16 == pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize())
        CreateBrightnessMap_T<uint16_t>((uint16_t*)pMap, m_brightness);
    else
        {
        BeAssert(!"HRAImageOpContrastFilter: Unexpected bitsPerPixel");
        return false;   // ERROR
        }

    // All channels use the same map.
    for(uint32_t index=0; index < pixelType.GetChannelOrg().CountChannels(); ++index)
        mapFilter._SetMapReference(pMap, index);
        
    return true;
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------HRAImageOpContrastFilter---------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpContrastFilter::CreateContrastFilter(double contrast)
    {
    return new HRAImageOpContrastFilter(contrast);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpContrastFilter::HRAImageOpContrastFilter(double contrast)
    :HRAImageOpMapFilter(),
     m_contrast(contrast)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpContrastFilter::~HRAImageOpContrastFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T> 
void HRAImageOpContrastFilter::CreateContrastMap_T(Data_T* pMapBuffer)
    {
    //&&Backlog EN: review contrast formula

    int32_t const typeMax = std::numeric_limits<Data_T>::max();

#if 0   // New formulas. All look almost the same
    double normalizeContrast = m_contrast;

    #if 1
        // Style: http://www.dfstudios.co.uk/articles/image-processing-algorithms-part-5/
        normalizeContrast = ((259.0/255) * (m_contrast + 1.0)) / ((259.0/255) - m_contrast);
        // C = 1.0 ->>> nc = 129.55.  129.55 / 255 == 0.508
    #else // GIMP style: http://en.wikipedia.org/wiki/Image_editing
        normalizeContrast = tan ((normalizeContrast + 1) * PI/4.0);
    #endif

    for (uint32_t index = 0; index < typeMax+1; ++index)
        {
        // newValue = (value - 0.5) * contrast + 0.5
        double normalizeValue = index/(double)typeMax;
        
        double normalizeNewValue = (normalizeValue - 0.5) * normalizeContrast + 0.5;

        double NewValueD = normalizeNewValue*typeMax;
        Data_T newVal = (Data_T)BOUND(NewValueD, 0, typeMax);

        pMapBuffer[index] = newVal;
        }

#else // HRPContrastFilter formula. Where it comes from??? == irfanview when m_contrast > 0

    // Use floating-point precision because 16bits data computation may exceed Int32 domain.
    double contrastLevel = m_contrast * (typeMax*0.5);
    
    if(contrastLevel > 0.0)
        {    
        int32_t contrastLevelInt = (int32_t)contrastLevel;
        int32_t index = 0;
        for (; index < contrastLevelInt; ++index)
            pMapBuffer[index] = 0;

        for(index = contrastLevelInt; index <= typeMax - contrastLevelInt; ++index)
            {
            int32_t newValue = (int32_t)(typeMax * ((index - contrastLevel) / (typeMax - 2.0 * contrastLevel)));   
            pMapBuffer[index] = (Data_T)BOUND(newValue, 0, typeMax);
            }

        for(index = (typeMax+1) - contrastLevelInt; index <= typeMax; ++index)
            pMapBuffer[index] = (Data_T)typeMax;
        }
    else
        {
        for(int32_t index = 0; index <= typeMax; ++index)
            {
            int32_t newValue =  (int32_t)((typeMax - (-2.0 * contrastLevel)) * index / typeMax - contrastLevel);
            pMapBuffer[index] = (Data_T)BOUND(newValue, 0, typeMax);
            }
        }
      
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpContrastFilter::_SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType)
    {
    void* pMap = mapFilter._AllocateChannelMap();

    if(8 == pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize())
        CreateContrastMap_T<uint8_t>((uint8_t*)pMap);
    else if(16 == pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize())
        CreateContrastMap_T<uint16_t>((uint16_t*)pMap);
    else
        {
        BeAssert(!"HRAImageOpContrastFilter: Unexpected bitsPerPixel");
        return false;   // ERROR
        }

    // All channels use the same map.
    for(uint32_t index=0; index < pixelType.GetChannelOrg().CountChannels(); ++index)
        mapFilter._SetMapReference(pMap, index);
        
    return true;
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------HRAImageOpContrastStretchFilter-------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpContrastStretchFilterPtr HRAImageOpContrastStretchFilter::CreateContrastStretchFilter(HRPPixelType const& intervalPixelType)
    {
    return new HRAImageOpContrastStretchFilter(intervalPixelType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpContrastStretchFilter::HRAImageOpContrastStretchFilter(HRPPixelType const& pixelType)
    :HRAImageOpMapFilter(),
     m_intervalPixeltype((HRPPixelType*)pixelType.Clone())
    {
    HPRECONDITION(pixelType.GetChannelOrg().HaveSameSize() && pixelType.GetChannelOrg().HaveSameDataType());
    HPRECONDITION(pixelType.GetChannelOrg().CountChannels() <= 4);    
    HPRECONDITION(pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize() == 8 || pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize() == 16);
    HPRECONDITION(pixelType.GetChannelOrg().GetChannelPtr(0)->GetDataType() == HRPChannelType::INT_CH || pixelType.GetChannelOrg().GetChannelPtr(0)->GetDataType() == HRPChannelType::VOID_CH);
    HPRECONDITION(IsSupportedPixeltype(pixelType));

    memset(m_minValues.values, 0, sizeof(m_minValues));
    memset(m_maxValues.values, 0xFF, sizeof(m_maxValues));
    memset(m_minContrastValues.values, 0, sizeof(m_minContrastValues));
    memset(m_maxContrastValues.values, 0xFF, sizeof(m_maxContrastValues));

    for(uint32_t i=0; i < 4; ++i)
        m_gammaFactor[i] = 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpContrastStretchFilter::~HRAImageOpContrastStretchFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpContrastStretchFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
     {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;  // We only have one possible output

    if(GetOutputPixelType() != NULL)
        {
        // input/output must be of same type.
        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        }
    else
        {
        pixelType = (HRPPixelType*)m_intervalPixeltype->Clone();        
        }

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpContrastStretchFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;  // We only have one possible output

    if(GetInputPixelType() != NULL)
        {
        // input/output must be of same type.
        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        }
    else
        {
        pixelType = (HRPPixelType*)m_intervalPixeltype->Clone();
        }

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool        HRAImageOpContrastStretchFilter::IsSupportedPixeltype(HRPPixelType const& pixeltype) const
    {
    if(!HRAImageOpMapFilter::IsSupportedPixeltype(pixeltype))
        return false;

    // Validate pixeltype against our interval pixeltype. All channels present into the interval pixeltype must be present.
    uint32_t channelCount = m_intervalPixeltype->GetChannelOrg().CountChannels();
    for(uint32_t i=0; i < channelCount; ++i)
        {
        HRPChannelType const* pChannelType = m_intervalPixeltype->GetChannelOrg().GetChannelPtr(i);
        if(HRPChannelType::FREE == pixeltype.GetChannelOrg().GetChannelIndex(pChannelType->GetRole(), pChannelType->GetId()))
            return false;   // Not supported.
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpContrastStretchFilter::GetInterval(uint16_t channelIndex, uint16_t& minValue, uint16_t& maxValue) const
    {
    HPRECONDITION(channelIndex < m_intervalPixeltype->GetChannelOrg().CountChannels());
    
    if(m_intervalPixeltype->GetChannelOrg().GetChannelPtr(0)->GetSize() == 8)
        {
        minValue = m_minValues.value8[channelIndex];
        maxValue = m_maxValues.value8[channelIndex];
        }
    else
        {
        minValue = m_minValues.value16[channelIndex];
        maxValue = m_maxValues.value16[channelIndex];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpContrastStretchFilter::SetInterval(uint16_t channelIndex, uint16_t minValue, uint16_t maxValue)
    {
    HPRECONDITION(channelIndex < m_intervalPixeltype->GetChannelOrg().CountChannels());
    HPRECONDITION(minValue <= maxValue);

    if(m_intervalPixeltype->GetChannelOrg().GetChannelPtr(0)->GetSize() == 8)
        {
        m_minValues.value8[channelIndex] = (uint8_t)minValue;
        m_maxValues.value8[channelIndex] = (uint8_t)maxValue;
        }
    else
        {
        m_minValues.value16[channelIndex] = minValue;
        m_maxValues.value16[channelIndex] = maxValue;
        }   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpContrastStretchFilter::GetContrastInterval(uint16_t channelIndex, uint16_t& minValue, uint16_t& maxValue) const
    {
    HPRECONDITION(channelIndex < m_intervalPixeltype->GetChannelOrg().CountChannels());
    
    if(m_intervalPixeltype->GetChannelOrg().GetChannelPtr(0)->GetSize() == 8)
        {
        minValue = m_minContrastValues.value8[channelIndex];
        maxValue = m_maxContrastValues.value8[channelIndex];
        }
    else
        {
        minValue = m_minContrastValues.value16[channelIndex];
        maxValue = m_maxContrastValues.value16[channelIndex];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpContrastStretchFilter::SetContrastInterval(uint16_t channelIndex, uint16_t minValue, uint16_t maxValue)
    {
    HPRECONDITION(channelIndex < m_intervalPixeltype->GetChannelOrg().CountChannels());

    if(m_intervalPixeltype->GetChannelOrg().GetChannelPtr(0)->GetSize() == 8)
        {
        m_minContrastValues.value8[channelIndex] = (uint8_t)minValue;
        m_maxContrastValues.value8[channelIndex] = (uint8_t)maxValue;
        }
    else
        {
        m_minContrastValues.value16[channelIndex] = minValue;
        m_maxContrastValues.value16[channelIndex] = maxValue;
        }   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
double HRAImageOpContrastStretchFilter::GetGammaFactor(uint16_t channelIndex) const
    {
    HPRECONDITION(channelIndex < m_intervalPixeltype->GetChannelOrg().CountChannels());

    return m_gammaFactor[channelIndex];
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpContrastStretchFilter::SetGammaFactor(uint16_t channelIndex, double gammaFactor)
    {
    HPRECONDITION(channelIndex < m_intervalPixeltype->GetChannelOrg().CountChannels());

    m_gammaFactor[channelIndex] = gammaFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T> 
void HRAImageOpContrastStretchFilter::CreateContrastStretchMap_T(Data_T* pMapBuffer, Data_T minValue, Data_T maxValue, 
                                                                 Data_T minContrast, Data_T maxContrast, double gamma)
    {
    uint32_t const entryCount = std::numeric_limits<Data_T>::max() + 1;

    uint32_t i;

    //----------------------------------------
    // Compute Input clipping if any.
    if (maxValue < std::numeric_limits<Data_T>::max() || minValue > 0)
        {
        double delta = maxValue - minValue;
        for (i = 0; i < minValue; ++i)
            pMapBuffer[i] = 0;

        for (i = minValue; i < maxValue; ++i)
            pMapBuffer[i] = (Data_T)(((i - minValue) / delta) * std::numeric_limits<Data_T>::max());

        for (i = maxValue; i < entryCount; ++i)
            pMapBuffer[i] = std::numeric_limits<Data_T>::max();
        }
    else
        {
        // init identity.
        for (i = 0; i < entryCount; ++i)
            pMapBuffer[i] = (Data_T)i;
        }

    //----------------------------------------
    // Process gamma adjustment if required.
    if (!HDOUBLE_EQUAL_EPSILON(gamma, 1.0))
        {
        for (i = 0; i < entryCount; ++i)
            pMapBuffer[i] = (Data_T)(std::numeric_limits<Data_T>::max() * pow( pMapBuffer[i] / (double)std::numeric_limits<Data_T>::max(), 1 / gamma));
        }

    //----------------------------------------
    // Compute output clipping if any.
    if (maxContrast < std::numeric_limits<Data_T>::max() || minContrast > 0)
        {
        // Take care of inverted handle.
        if (maxContrast > minContrast)
            {
            double deltaFactor = (maxContrast - minContrast) / (double)std::numeric_limits<Data_T>::max();
            for (i = 0; i < entryCount; ++i)
                pMapBuffer[i] = (Data_T)(minContrast + (pMapBuffer[i] * deltaFactor));
            }
        else
            {
            for (i = 0; i < entryCount; ++i)
                pMapBuffer[i] = (Data_T)(((std::numeric_limits<Data_T>::max() - (double)(pMapBuffer[i])) / (double)std::numeric_limits<Data_T>::max()) * (minContrast - maxContrast)) + maxContrast;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpContrastStretchFilter::_SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType)
    {
    BeAssert(pixelType.GetChannelOrg().CountChannels() <= 4);    // We limited to a max of 4 channels.

    if(8 != pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize() && 16 != pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize())
        {
        BeAssert(!"HRAImageOpContrastFilter: Unexpected bitsPerPixel");
        return false;
        }

    // Transform intervals into src pixeltype
    HFCPtr<HRPPixelConverter> pConverter = m_intervalPixeltype->GetConverterTo(GetInputPixelType());
    
    Values minValuesInSrc;
    Values maxValuesInSrc;
    pConverter->Convert(m_minValues.values, minValuesInSrc.values, 1);
    pConverter->Convert(m_maxValues.values, maxValuesInSrc.values, 1);

    Values minContrastInSrc;
    Values maxContrastInSrc;
    pConverter->Convert(m_minContrastValues.values, minContrastInSrc.values, 1);
    pConverter->Convert(m_maxContrastValues.values, maxContrastInSrc.values, 1);
    
    HRPChannelOrg const& srcChannelOrg = pixelType.GetChannelOrg();

    for(uint32_t i=0; i < srcChannelOrg.CountChannels(); ++i)
        {
        // Scale only channel role that are present in the interval pixelType
        if(HRPChannelType::FREE != m_intervalPixeltype->GetChannelOrg().GetChannelIndex(srcChannelOrg.GetChannelPtr(i)->GetRole(), srcChannelOrg.GetChannelPtr(i)->GetId()))
            {
            void* pMap = mapFilter._AllocateChannelMap();

            if(8 == srcChannelOrg.GetChannelPtr(i)->GetSize())
                CreateContrastStretchMap_T<uint8_t>((uint8_t*)pMap, minValuesInSrc.value8[i], maxValuesInSrc.value8[i], minContrastInSrc.value8[i], maxContrastInSrc.value8[i], m_gammaFactor[i]);
            else if(16 == srcChannelOrg.GetChannelPtr(i)->GetSize())
                CreateContrastStretchMap_T<uint16_t>((uint16_t*)pMap, minValuesInSrc.value16[i], maxValuesInSrc.value16[i], minContrastInSrc.value16[i], maxContrastInSrc.value16[i], m_gammaFactor[i]);
            
            mapFilter._SetMapReference(pMap, i);
            }
        }
        
    return true;
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------HRAImageOpInvertFilter---------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpInvertFilter::CreateInvertFilter()
    {
    return new HRAImageOpInvertFilter();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpInvertFilter::HRAImageOpInvertFilter()
    :HRAImageOpMapFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpInvertFilter::~HRAImageOpInvertFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T> 
void HRAImageOpInvertFilter::CreateInvertMap_T(Data_T* pMapBuffer)
    {
    //&&OPTIMIZATION this computation si so trivial that we should try doing it in real time. 
    //  1) Update: Did the test and computing on the fly is about 2.5x/3x faster for 8 and 16 bits. Need to evaluate 
    //          other trivial map filter and code them as function filter.  What we could lost is the ability 
    //          to merge map filter into one operation but it's not implemented yet.
    //  2) Need to create our own version of numeric_limits because std version is not inlined.
    uint32_t const typeMax = std::numeric_limits<Data_T>::max();
    for (uint32_t index = 0; index < typeMax+1; ++index)
        pMapBuffer[index] = (Data_T)(typeMax-index); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpInvertFilter::_SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType)
    {
    void* pMap = mapFilter._AllocateChannelMap();

    if(8 == pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize())
        CreateInvertMap_T<uint8_t>((uint8_t*)pMap);
    else if(16 == pixelType.GetChannelOrg().GetChannelPtr(0)->GetSize())
        CreateInvertMap_T<uint16_t>((uint16_t*)pMap);
    else
        {
        BeAssert(!"HRAImageOpInvertFilter: Unexpected bitsPerPixel");
        return false;   // ERROR
        }

    // All channels use the same map.
    for(uint32_t index=0; index < pixelType.GetChannelOrg().CountChannels(); ++index)
        mapFilter._SetMapReference(pMap, index);
        
    return true;
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------HRAImageOpTintFilter---------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpTintFilter::CreateTintFilter(Byte const* pRgbColor)
    {
    return new HRAImageOpTintFilter(pRgbColor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpTintFilter::HRAImageOpTintFilter(Byte const* pRgbColor)
    :HRAImageOpMapFilter()
    {
    m_rgbColor[0] = pRgbColor[0];
    m_rgbColor[1] = pRgbColor[1];
    m_rgbColor[2] = pRgbColor[2];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpTintFilter::~HRAImageOpTintFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpTintFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;  // We only have one possible output

    if(GetOutputPixelType() != NULL)
        {
        // input/output must be of same type.
        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }

    return GetMatchingPixelType(pixelType, pixelTypeToMatch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpTintFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;  // We only have one possible output

    if(GetInputPixelType() != NULL)
        {
        // input/output must be of same type.
        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }

    return GetMatchingPixelType(pixelType, pixelTypeToMatch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool        HRAImageOpTintFilter::IsSupportedPixeltype(HRPPixelType const& pixeltype) const
    {
    if(!HRAImageOpMapFilter::IsSupportedPixeltype(pixeltype))
        return false;

    // Validate that red, green blue are present.
    if(pixeltype.GetChannelOrg().GetChannelIndex(HRPChannelType::RED, 0) == HRPChannelType::FREE ||
       pixeltype.GetChannelOrg().GetChannelIndex(HRPChannelType::GREEN, 0) == HRPChannelType::FREE ||
       pixeltype.GetChannelOrg().GetChannelIndex(HRPChannelType::BLUE, 0) == HRPChannelType::FREE)
       return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T> 
void HRAImageOpTintFilter::CreateTintMap_T(Data_T* pMapBuffer, Data_T tintFactor)
    {
    uint32_t const typeMax = std::numeric_limits<Data_T>::max();
    for (uint32_t index = 0; index < typeMax+1; ++index)
        pMapBuffer[index] = (Data_T)((index * tintFactor) / typeMax);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpTintFilter::_SetupMapFilter(MapFilter& mapFilter, HRPPixelType const& pixelType)
    {
    HRPChannelOrg const& srcChannelOrg = pixelType.GetChannelOrg();

    for(uint32_t i=0; i < srcChannelOrg.CountChannels(); ++i)
        {
        uint32_t tintIndex = -1;
        switch(srcChannelOrg.GetChannelPtr(i)->GetRole())
            {
            case HRPChannelType::RED:   tintIndex = 0;  break;
            case HRPChannelType::GREEN: tintIndex = 1;  break;
            case HRPChannelType::BLUE:  tintIndex = 2;  break;
            default:                    tintIndex =-1;  break;
            }

        if(-1 != tintIndex)
            {
            void* pMap = mapFilter._AllocateChannelMap();

            if(8 == srcChannelOrg.GetChannelPtr(i)->GetSize())
                CreateTintMap_T<uint8_t>((uint8_t*)pMap, m_rgbColor[tintIndex]);
            else if(16 == srcChannelOrg.GetChannelPtr(i)->GetSize())
                CreateTintMap_T<uint16_t>((uint16_t*)pMap, m_rgbColor[tintIndex] << 8);

            mapFilter._SetMapReference(pMap, i);
            }
        }

    return true;
    }
