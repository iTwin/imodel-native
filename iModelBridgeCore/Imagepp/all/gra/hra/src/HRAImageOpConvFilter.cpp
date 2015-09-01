//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageOpConvFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRAImageOpConvFilter.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <ImagePP/all/h/HRPPixelType.h>

enum ChannelOperation
    {
    CHANNELOP_Nop,          // Use when channel doesn't exist.
    CHANNELOP_Copy,
    CHANNELOP_Convoluate,
    };

/*---------------------------------------------------------------------------------**//**
* struct ConvFilter_T                                               
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T, uint32_t ChannelCount_T, uint32_t Channel1_T, uint32_t Channel2_T=CHANNELOP_Nop, uint32_t Channel3_T=CHANNELOP_Nop, uint32_t Channel4_T=CHANNELOP_Nop>
struct ConvFilter_T : HRAImageOpConvolutionFilter::ConvFilter
{
    typedef float Sum_T;        // for now always compute in float. &&OPTIMIZATION is it faster to convolute with UInt32 when UInt8?
public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    ConvFilter_T()
        {
        HPRECONDITION(IN_RANGE(ChannelCount_T, 0, 4));
        HPRECONDITION(ChannelCount_T == 1 ? (Channel2_T==CHANNELOP_Nop && Channel3_T==CHANNELOP_Nop && Channel4_T==CHANNELOP_Nop) : true);
        HPRECONDITION(ChannelCount_T == 2 ? (Channel2_T!=CHANNELOP_Nop && Channel3_T==CHANNELOP_Nop && Channel4_T==CHANNELOP_Nop) : true);
        HPRECONDITION(ChannelCount_T == 3 ? (Channel2_T!=CHANNELOP_Nop && Channel3_T!=CHANNELOP_Nop && Channel4_T==CHANNELOP_Nop) : true);
        HPRECONDITION(ChannelCount_T == 4 ? (Channel2_T!=CHANNELOP_Nop && Channel3_T!=CHANNELOP_Nop && Channel4_T!=CHANNELOP_Nop) : true);
        }   

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  02/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImagePPStatus _Init(HRAImageOpConvolutionFilter::Kernel const& kernel, HRPPixelNeighbourhood const& pixelNeighbourdhood) override
        {
        m_Pos.clear();
        m_Factor.clear();
        m_DataPtrs.clear();

        m_originX = pixelNeighbourdhood.GetXOrigin();
        m_originY = pixelNeighbourdhood.GetYOrigin();

        Sum_T div = 0;
        //Transform  
        for(uint32_t y=0; y < kernel.size(); ++y)
            {            
            for(uint32_t x=0; x < kernel[y].size(); ++x)
                {
                if(0 != kernel[y][x])
                    {
                    m_Pos.push_back(Pos(x,y));
                    m_Factor.push_back((Sum_T)kernel[y][x]);
                    div += (Sum_T)kernel[y][x];
                    }
                }
            }

        if(div > 0.00001)
            {
            for(uint32_t i=0; i < m_Factor.size(); ++i)
                {
                m_Factor[i] /= div;
                }
            }
            
        m_DataPtrs.resize(m_Factor.size());

        return IMAGEPP_STATUS_Success;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImagePPStatus _Apply(HRAImageSampleR outData, HRAImageSampleCR inputData) override
        {
#ifndef DISABLE_BUFFER_GETDATA
        HPRECONDITION(!m_Factor.empty()); // was not _Init??
        HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
        HPRECONDITION(m_Pos.size() == m_Factor.size());
        HPRECONDITION(m_DataPtrs.size() == m_Factor.size());

        size_t inPitch, outPitch;
        Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

        uint32_t const factorCount = (uint32_t)m_Factor.size();
        uint32_t outWidth = outData.GetWidth();
        
        for(uint32_t line=0; line < outData.GetHeight(); ++line)
            {
            Byte const* pInBufferLine  = pInBuffer+line*inPitch;
            Data_T const* pOriginBufferLine = (Data_T*)(pInBuffer+((line+m_originY)*inPitch));

            Data_T* pOutBufferLine = (Data_T*)(pOutBuffer+line*outPitch);

            // Compute effective position in src.
            for(uint32_t factor=0; factor < factorCount; ++factor)
                m_DataPtrs[factor] = ((Data_T const*)(pInBufferLine + /* in bytes*/(m_Pos[factor].second*inPitch))) + /*in Data_T*/(m_Pos[factor].first*ChannelCount_T);

            for(uint32_t column=0; column < outWidth; ++column)
                {    
                Sum_T sums[4]; 
                sums[0] = sums[1] = sums[2] = sums[3] = 0;

                // OPTIMIZATION NOTES:
                // All the per-pixel ifs/elses may look strange but they evaluate to constant value so compiler
                // will remove them depending on template parameter and optimize the code as if no 'if/else' are present.
                for(uint32_t factor=0; factor < factorCount; ++factor)
                    {
                    if(Channel1_T == CHANNELOP_Convoluate)
                        sums[0] += m_DataPtrs[factor][column*ChannelCount_T+0] * m_Factor[factor];

                    if(Channel2_T == CHANNELOP_Convoluate)
                        sums[1] += m_DataPtrs[factor][column*ChannelCount_T+1] * m_Factor[factor];

                    if(Channel3_T == CHANNELOP_Convoluate)
                        sums[2] += m_DataPtrs[factor][column*ChannelCount_T+2] * m_Factor[factor];

                    if(Channel4_T == CHANNELOP_Convoluate)
                        sums[3] += m_DataPtrs[factor][column*ChannelCount_T+3] * m_Factor[factor];
                    }

                if(Channel1_T == CHANNELOP_Copy)
                    pOutBufferLine[column*ChannelCount_T+0] = pOriginBufferLine[(column+m_originX)*ChannelCount_T+0]; 
                else if(Channel1_T == CHANNELOP_Convoluate)
                    pOutBufferLine[column*ChannelCount_T+0] = clip_cast<Data_T>(sums[0]);

                if(Channel2_T == CHANNELOP_Copy)
                    pOutBufferLine[column*ChannelCount_T+1] = pOriginBufferLine[(column+m_originX)*ChannelCount_T+1]; 
                else if(Channel2_T == CHANNELOP_Convoluate)
                    pOutBufferLine[column*ChannelCount_T+1] = clip_cast<Data_T>(sums[1]);

                if(Channel3_T == CHANNELOP_Copy)
                    pOutBufferLine[column*ChannelCount_T+2] = pOriginBufferLine[(column+m_originX)*ChannelCount_T+2]; 
                else if(Channel3_T == CHANNELOP_Convoluate)
                    pOutBufferLine[column*ChannelCount_T+2] = clip_cast<Data_T>(sums[2]);

                if(Channel4_T == CHANNELOP_Copy)
                    pOutBufferLine[column*ChannelCount_T+3] = pOriginBufferLine[(column+m_originX)*ChannelCount_T+3]; 
                else if(Channel4_T == CHANNELOP_Convoluate)
                    pOutBufferLine[column*ChannelCount_T+3] = clip_cast<Data_T>(sums[3]);
                }
            }       
#endif        
        return IMAGEPP_STATUS_Success;
        }    

    typedef pair<uint32_t,uint32_t> Pos;
    vector<Pos>     m_Pos;
    vector<Sum_T>   m_Factor;
    vector<Data_T const*>  m_DataPtrs;
    uint32_t        m_originX;   // pixel neighbourhood origin
    uint32_t        m_originY;
};


/*-------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------HRAImageOpBlurFilter----------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpConvolutionFilter::ConvFilter::ConvFilter(){};
HRAImageOpConvolutionFilter::ConvFilter::~ConvFilter(){};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpConvolutionFilter::HRAImageOpConvolutionFilter()
    :HRAImageOp()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpConvolutionFilter::~HRAImageOpConvolutionFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpConvolutionFilter::CreateCustomConvolutionFilter(Kernel const& kernel, HRPPixelNeighbourhood const& pixelNeighbourhood)
    {
    HPRECONDITION(kernel.size() == pixelNeighbourhood.GetHeight());
    HPRECONDITION(kernel[0].size() == pixelNeighbourhood.GetWidth());
    
    HRAImageOpConvolutionFilter* pFilter = new HRAImageOpConvolutionFilter();
    pFilter->SetKernel(kernel, pixelNeighbourhood);

    return pFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpConvolutionFilter::CreateAverageFilter()
    {
    // Average kernel.
    // [ 1, 1 ]
    // [ 1, 1 ]
    Kernel kernel(2);
    kernel[0].resize(2, 1);
    kernel[1].resize(2, 1);
            
    return CreateCustomConvolutionFilter(kernel, HRPPixelNeighbourhood(2, 2, 0, 0));  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpConvolutionFilter::CreateBlurFilter(uint8_t intensity)
    {
    // Blur kernel. 
    // [ 1, 1, 1, 1, 1]
    // [ 1, 0, 0, 0, 1]
    // [ 1, 0, X, 0, 1]
    // [ 1, 0, 0, 0, 1]
    // [ 1, 1, 1, 1, 1]
    Kernel kernel(5);
    kernel[0].resize(5, 1);
    kernel[1].resize(5, 0); kernel[1][0] = kernel[1][4] = 1;
    kernel[2] = kernel[1];
    kernel[3] = kernel[1];
    kernel[4].resize(5, 1);

    // I don't understand this kernel, it is a port from the old blur filter. MM.
    // we can see "blur" effect with a center weight of 31 to 0 (observation)
    kernel[2][2] = 31 - intensity * 31 / 255;

    return CreateCustomConvolutionFilter(kernel, HRPPixelNeighbourhood(5, 5, 2, 2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpConvolutionFilter::CreateDetailFilter()
    {
    // Detail kernel.
    // [  0, -1,  0 ]
    // [ -1, 10, -1 ]
    // [  0, -1,  0 ]
    Kernel kernel(3);
    kernel[0].resize(3, 0);  kernel[0][1] = -1;
    kernel[1].resize(3, -1); kernel[1][1] = 10;
    kernel[2].resize(3, 0);  kernel[2][1] = -1;
            
    return CreateCustomConvolutionFilter(kernel, HRPPixelNeighbourhood(3, 3, 1, 1));  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpConvolutionFilter::CreateEdgeEnhanceFilter()
    {
    // Edge enhance kernel.
    // [ -1, -1, -1 ]
    // [ -1, 10, -1 ]
    // [ -1, -1, -1 ]
    Kernel kernel(3);
    kernel[0].resize(3, -1);
    kernel[1].resize(3, -1); kernel[1][1] = 10;
    kernel[2].resize(3, -1);
            
    return CreateCustomConvolutionFilter(kernel, HRPPixelNeighbourhood(3, 3, 1, 1));  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpConvolutionFilter::CreateFindEdgeFilter()
    {
    // Find edge kernel.
    // [ -1, -1, -1 ]
    // [ -1,  8, -1 ]
    // [ -1, -1, -1 ]
    Kernel kernel(3);
    kernel[0].resize(3, -1);
    kernel[1].resize(3, -1); kernel[1][1] = 8;
    kernel[2].resize(3, -1);
            
    return CreateCustomConvolutionFilter(kernel, HRPPixelNeighbourhood(3, 3, 1, 1));  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpConvolutionFilter::CreateSharpenFilter(uint8_t intensity)
    {
    // Sharpen kernel.
    // [ -2, -2, -2 ]
    // [ -2,  X, -2 ]
    // [ -2, -2, -2 ]
    Kernel kernel(3);
    kernel[0].resize(3, -2);
    kernel[1].resize(3, -2);
    kernel[2].resize(3, -2);

    // I don't understand this kernel, it is a port from the old sharpen filter. MM.
    // we can see "sharpen" effect with a center weight of 48 to 17 (observation)
    kernel[1][1] = 31 - intensity * 31 / 255 + 17;
            
    return CreateCustomConvolutionFilter(kernel, HRPPixelNeighbourhood(3, 3, 1, 1));  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpConvolutionFilter::CreateSmoothFilter()
    {
    // Smooth kernel.
    // [ 1, 1, 1 ]
    // [ 1, 5, 1 ]
    // [ 1, 1, 1 ]
    Kernel kernel(3);
    kernel[0].resize(3, 1);
    kernel[1].resize(3, 1); kernel[1][1] = 5;
    kernel[2].resize(3, 1);
            
    return CreateCustomConvolutionFilter(kernel, HRPPixelNeighbourhood(3, 3, 1, 1));  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpConvolutionFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    // If we have an output, input must be of same type.
    if(GetOutputPixelType() != NULL)
        {
        if(index > 0)
            return IMAGEOP_STATUS_NoMorePixelType;  // We only have one possible input when output is set.

        pixelType = (HRPPixelType*)GetOutputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }
    
    return IMAGEOP_STATUS_NoMorePixelType;      // We have no default.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpConvolutionFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    // If we have an input, output must be of same type.
    if(GetInputPixelType() != NULL)
        {
        if(index > 0)
            return IMAGEOP_STATUS_NoMorePixelType;  // We only have one possible output when input is set.

        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }
    
    return IMAGEOP_STATUS_NoMorePixelType;      // We have no default.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpConvolutionFilter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pInputPixelType = NULL;
        UpdateFilter();
        return IMAGEPP_STATUS_Success;
        }

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
ImagePPStatus HRAImageOpConvolutionFilter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pOutputPixelType = NULL;
        UpdateFilter();
        return IMAGEPP_STATUS_Success;
        }

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
void HRAImageOpConvolutionFilter::SetKernel(HRAImageOpConvolutionFilter::Kernel const& kernel, HRPPixelNeighbourhood const& neighborhood)
    {
    HPRECONDITION(kernel.size() == neighborhood.GetHeight());
    HPRECONDITION(kernel[0].size() == neighborhood.GetWidth());

    m_kernel = kernel;
    m_pPixelNeighbourhood = new HRPPixelNeighbourhood(neighborhood);

    UpdateFilter();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpConvolutionFilter::IsSupportedPixeltype(HRPPixelType const& pixelType) const
    {
    std::unique_ptr<ConvFilter> pFilter(CreateFilter(pixelType));

    return pFilter.get() != NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpConvolutionFilter::ConvFilter* HRAImageOpConvolutionFilter::CreateFilter(HRPPixelType const& pixelType) const
    {       
    HRAImageOpConvolutionFilter::ConvFilter* pFilter = NULL;
    
    switch(pixelType.GetClassID())
        {
        case HRPPixelTypeId_V8Gray8:
        case HRPPixelTypeId_V8GrayWhite8:
            pFilter = new ConvFilter_T<uint8_t, 1, CHANNELOP_Convoluate>();
            break;

        case HRPPixelTypeId_V16Gray16:
        case HRPPixelTypeId_V16Int16:
            pFilter = new ConvFilter_T<uint16_t, 1, CHANNELOP_Convoluate>();
            break;

        case HRPPixelTypeId_V32Float32:
            pFilter = new ConvFilter_T<float, 1, CHANNELOP_Convoluate>();
            break;

        // Premultiplied alpha can be process directly including the alpha channel.
        case HRPPixelTypeId_V16PRGray8A8:
            pFilter = new ConvFilter_T<uint8_t, 2, CHANNELOP_Convoluate, CHANNELOP_Convoluate>();
            break;
       
        case HRPPixelTypeId_V24R8G8B8:
        case HRPPixelTypeId_V24B8G8R8:
        case HRPPixelTypeId_V24PhotoYCC:
            pFilter = new ConvFilter_T<uint8_t, 3, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Convoluate>();
            break;

        case HRPPixelTypeId_V48R16G16B16:
            pFilter = new ConvFilter_T<uint16_t, 3, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Convoluate>();
            break;

        case HRPPixelTypeId_V96R32G32B32:
            pFilter = new ConvFilter_T<uint32_t, 3, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Convoluate >();
            break;

        case HRPPixelTypeId_V32A8R8G8B8:
            pFilter = new ConvFilter_T<uint8_t, 4, CHANNELOP_Copy, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Convoluate>();
            break;

        // &&Backlog EN: Processing pixeltype with Alpha channel could lead to black shadows because transparent color will be used for convolution.
        // see "Alpha Channels": http://www.jhlabs.com/ip/blurring.html
        // review all non Premul alpha pixel type.
        case HRPPixelTypeId_V32B8G8R8X8:
        case HRPPixelTypeId_V32R8G8B8A8:
        case HRPPixelTypeId_V32R8G8B8X8:
            pFilter = new ConvFilter_T<uint8_t, 4, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Copy>();
            break;
        
        case HRPPixelTypeId_V64R16G16B16A16:
        case HRPPixelTypeId_V64R16G16B16X16:
            pFilter = new ConvFilter_T<uint16_t, 4, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Copy>();
            break;

        // Premultiplied alpha can be process directly including the alpha channel.
        case HRPPixelTypeId_V32PRPhotoYCCA8:
        case HRPPixelTypeId_V32PR8PG8PB8A8:
            pFilter = new ConvFilter_T<uint8_t, 4, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Convoluate>();
            break;

        case HRPPixelTypeId_V32CMYK:
            pFilter = new ConvFilter_T<uint8_t, 4, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Convoluate, CHANNELOP_Convoluate>();
            break;

        // not supported.
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
        //case HRPPixelTypeId_V16B5G5R5:
        //case HRPPixelTypeId_V16R5G6B5:
        //case HRPPixelTypeId_I8R8G8B8Mask:
        default:
            pFilter = NULL;
            break;
        }

    return pFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpConvolutionFilter::UpdateFilter()
    {
    m_pFilter.reset(NULL);

    if(GetInputPixelType() == NULL || GetOutputPixelType() == NULL)
        return IMAGEPP_STATUS_UnknownError;     // Not ready.

    if(m_pPixelNeighbourhood == NULL || m_kernel.empty())
        return IMAGEPP_STATUS_UnknownError;     // Not ready.
    
    m_pFilter.reset(CreateFilter(*GetInputPixelType()));

    if(m_pFilter.get() == NULL)
        return IMAGEOP_STATUS_InvalidPixelType;  
    
    return m_pFilter->_Init(m_kernel, GetNeighbourhood());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpConvolutionFilter::_Process(HRAImageSampleR outData, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
    HPRECONDITION(GetInputPixelType() != NULL && GetOutputPixelType() != NULL);
    HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
    HPRECONDITION(inputData.GetWidth() == outData.GetWidth() + GetNeighbourhood().GetWidth() - 1);
    HPRECONDITION(inputData.GetHeight() == outData.GetHeight() + GetNeighbourhood().GetHeight() - 1);

    return m_pFilter->_Apply(outData, inputData);
    }






