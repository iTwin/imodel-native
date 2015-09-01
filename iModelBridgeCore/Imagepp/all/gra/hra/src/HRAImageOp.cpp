//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageOp.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRAImageOp.h>
#include <ImagePPInternal/gra/ImageCommon.h>
#include <ImagePPInternal/gra/HRAImageSampler.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <ImagePP/all/h/HPMPool.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HRATiledRaster.h>
#include <ImagePP/all/h/HRPPixelTypeFactory.h>
#include <ImagePPInternal/gra/ImageAllocator.h>




/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAImageBuffer----------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageBuffer::~HRAImageBuffer(){}
size_t  HRAImageBuffer::GetBufferSize() const           { return m_bufferSize; }
Byte*   HRAImageBuffer::GetDataP(size_t& pitch)         { pitch = m_pitch; return m_pData; }
HRAImageBufferRleP HRAImageBuffer::AsBufferRleP()       { return _AsBufferRleP(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageBuffer::HRAImageBuffer(Byte* pData, size_t bufferSize, size_t pitch)
   :m_pData(pData),
    m_bufferSize(bufferSize),
    m_pitch(pitch)
    {
    }


/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAImageBufferMemory--------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageBufferPtr HRAImageBufferMemory::CreateMemoryBuffer(ImagePPStatus& status, size_t sizeInBytes, size_t pitch, IImageAllocatorR allocator)
    {
    Byte* pBuffer = allocator._AllocMemory(sizeInBytes);
    if (NULL == pBuffer)
        {
        status = IMAGEPP_STATUS_OutOfMemory;
        return NULL;
        }

    status = IMAGEPP_STATUS_Success;
    return new HRAImageBufferMemory(pBuffer, sizeInBytes, pitch, allocator);
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageBufferMemory::HRAImageBufferMemory(Byte* pBuffer, size_t sizeInBytes, size_t pitch, IImageAllocatorR allocator)
    :HRAImageBuffer(pBuffer, sizeInBytes, pitch),
     m_allocator(allocator)     // Allocator that will be used to free memory.
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageBufferMemory::~HRAImageBufferMemory()
    {
    m_allocator._FreeMemory(m_pData);
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAContiguousRLEImageBuffer--------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageBufferPtr HRAImageBufferRle::CreateRleBuffer(ImagePPStatus& status, uint32_t width, uint32_t height, IImageAllocatorR allocator)
    {
    size_t lineSize = RLE_WORST_CASE_BYTES(width);

    size_t requiredSize = lineSize*height;

    Byte* pBuffer = allocator._AllocMemory(requiredSize);
    if (NULL == pBuffer)
        {
        status = IMAGEPP_STATUS_OutOfMemory;
        return NULL;
        }

    status = IMAGEPP_STATUS_Success;
    return new HRAImageBufferRle(pBuffer, requiredSize, lineSize, height, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageBufferRle::HRAImageBufferRle(Byte* pData, size_t bufferSize, size_t pitch, uint32_t height, IImageAllocatorR allocator)
:HRAImageBufferMemory(pData, bufferSize, pitch, allocator)
    {
    m_linesDataSize.resize(height, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageBufferRle::~HRAImageBufferRle()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HRAImageBufferRle::GetLineDataSize(uint32_t line) const
    {
    BeAssert(line < m_linesDataSize.size());
    return m_linesDataSize[line];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageBufferRle::SetLineDataSize(uint32_t line, size_t dataSize)
    {
    BeAssert(line < m_linesDataSize.size());
    m_linesDataSize[line] = static_cast<uint32_t>(dataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageBufferRle::ComputeDataSize(uint32_t line, uint32_t width)
    {
    BeAssert(RLE_WORST_CASE_BYTES(width) == m_pitch);
    BeAssert(line < m_linesDataSize.size());

    uint16_t const* pLine = (uint16_t const*)(m_pData + line*m_pitch);
    uint32_t pixelCount = 0;

    uint32_t index = 0;
    while (pixelCount < width)
        {
        pixelCount += pLine[index];
        ++index;
        }

    BeAssert(pixelCount == width);

    if (index & 0x1)
        {
        m_linesDataSize[line] = index*sizeof(uint16_t);
        }
    else
        {
        BeAssert(pLine[index] == 0);    // end with black

        m_linesDataSize[line] = (index + 1)*sizeof(uint16_t);
        }      
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAImageSample--------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
uint32_t             HRAImageSample::GetWidth() const                    {return m_width;}
uint32_t             HRAImageSample::GetHeight() const                   {return m_height;}
HRAImageBuffer*      HRAImageSample::GetBufferP()                        {return m_pBuffer.get();}
void                 HRAImageSample::SetBuffer(HRAImageBufferPtr& buffer) {m_pBuffer = buffer;}
HRPPixelType const&  HRAImageSample::GetPixelType() const                {return *m_pPixelType;}
HFCPtr<HRPPixelType>& HRAImageSample::GetPixelTypePtr()                 {return m_pPixelType;}
HRAImageBufferRleP   HRAImageSample::GetBufferRleP() {return GetBufferP() != NULL ? GetBufferP()->AsBufferRleP() : NULL; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAImageSample::CreateSample(ImagePPStatus& status, uint32_t width, uint32_t height, HFCPtr<HRPPixelType> pixelType, IImageAllocatorR allocator)
    {
    BeAssert(width > 0 && height > 0);

    HRAImageSamplePtr pSampler = new HRAImageSample(width, height, pixelType);

    status = pSampler->AllocateBuffer(allocator);
    
    return IMAGEPP_STATUS_Success == status ? pSampler : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAImageSample::internal_CreateSampleFromBuffer(ImagePPStatus& status, uint32_t width, uint32_t height, HFCPtr<HRPPixelType> pixelType, HRAImageBufferR buffer)
    {
    BeAssert(width > 0 && height > 0);

    HRAImageSamplePtr pSampler = new HRAImageSample(width, height, pixelType);

    HRAImageBufferPtr pBuf = &buffer;
    pSampler->SetBuffer(pBuf);
    
    status = IMAGEPP_STATUS_Success;
    return pSampler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSample::HRAImageSample(uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& pixelType)
    :m_width(width),
     m_height(height),
     m_pPixelType(pixelType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageSample::AllocateBuffer(IImageAllocatorR allocator)
    {
    // Code below would work but it is not expected to realloc a sample buffer. We should have filled the allocated memory.
    BeAssert(GetBufferCP() == NULL); 

    ImagePPStatus status = IMAGEPP_STATUS_UnknownError;

    HRAImageBufferPtr pBuffer;

    if (GetPixelType().IsCompatibleWith(HRPPixelTypeId_I1R8G8B8RLE) || GetPixelType().IsCompatibleWith(HRPPixelTypeId_I1R8G8B8A8RLE))
        {
        pBuffer = HRAImageBufferRle::CreateRleBuffer(status, GetWidth(), GetHeight(), allocator);
        }
    else
        {
        BeAssert(GetPixelType().CountPixelRawDataBits() % 8 == 0 || GetPixelType().CountPixelRawDataBits() == 1);

        size_t lineSize = (GetWidth() * GetPixelType().CountPixelRawDataBits() + 7) / 8;
        pBuffer = HRAImageBufferMemory::CreateMemoryBuffer(status, lineSize*GetHeight(), lineSize, allocator);
        }

    if (IMAGEPP_STATUS_Success != status)
        return status;

    SetBuffer(pBuffer);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAImageSample::CreateBufferReference(ImagePPStatus& status, uint32_t width, uint32_t height, uint32_t xOrigin, uint32_t yOrigin)
    {
    BeAssert(GetBufferP() != NULL);

    status = IMAGEPP_STATUS_UnknownError;

    if (GetBufferP() == NULL)
        return NULL;

    // We can reference any kind of sample if we have a perfect match
    if (GetWidth() == width && GetHeight() == height && xOrigin == 0 && yOrigin == 0)
        {
        status = IMAGEPP_STATUS_Success;
        return this;
        }

    // Non perfect match is only supported for N8
    if (GetPixelType().CountPixelRawDataBits() % 8 != 0)
        return NULL;

    // Can't reference outside
    if (xOrigin < 0 || xOrigin + GetWidth() > width || yOrigin < 0 || yOrigin + GetHeight() > height)
        return NULL;

    size_t pixelSize = GetPixelType().CountPixelRawDataBits() / 8;       

    size_t inPitch;
    Byte* pInBuffer = GetBufferP()->GetDataP(inPitch);
            
    HRAImageBufferPtr pBuffer = HRAImageBufferParasiteMemoryWithHold<HRAImageSamplePtr>::
        CreateBuffer(status,
                     pInBuffer + (yOrigin * inPitch) + (xOrigin * pixelSize),
                     (height-1)*inPitch + (width * pixelSize), 
                     inPitch,
                     this);     

    if (IMAGEPP_STATUS_Success != status)
        return NULL;

    return internal_CreateSampleFromBuffer(status, width, height, m_pPixelType, *pBuffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  1/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageSample::ValidateIntegrity() const
    {
    if(GetBufferRleCP() == NULL)
        return true;
        
    size_t pitch;
    Byte const* pData = GetBufferCP()->GetDataCP(pitch);

    for(uint32_t line=0; line < GetHeight(); ++line)
        {
        uint16_t const* pLine = (uint16_t const*)(pData + pitch*line);

        if(!Rle1Manip::ValidateLineIntegrity(pLine, GetWidth(), GetBufferRleCP()->GetLineDataSize(line)))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined __HMR_DEBUG
void HRAImageSample::MarkContour()
    {
    HRAImageBufferP pImageBuf = GetBufferP();
    if (NULL == pImageBuf)
        return;

    size_t pitch;
    Byte* pBuf = pImageBuf->GetDataP(pitch);

    if (NULL == pBuf)
        return;

    size_t samplePixelSize = (GetPixelType().CountPixelRawDataBits() + 7) / 8;
    Byte color = 100;
    memset(pBuf, color, m_width*samplePixelSize);
    memset(pBuf + (m_height - 1)*pitch, color, m_width*samplePixelSize);

    for (uint64_t y = 0; y < m_height; ++y)
        {
        Byte* pLine = pBuf + y*pitch;
        memset(pLine, color, samplePixelSize);
        memset(pLine + (m_width - 1)*samplePixelSize, color, samplePixelSize);
        }
    }
#endif
/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAImageOp--------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRPPixelType* HRAImageOp::GetInputPixelType() const     {return m_pInputPixelType.GetPtr();}
HRPPixelType* HRAImageOp::GetOutputPixelType() const    {return m_pOutputPixelType.GetPtr();}
HRPPixelNeighbourhood const& HRAImageOp::GetNeighbourhood() const 
    {
    HPRECONDITION(m_pPixelNeighbourhood != NULL);       // Must be setup at construction
    return *m_pPixelNeighbourhood;
    }

ImagePPStatus HRAImageOp::GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)  
    {return _GetAvailableInputPixelType(pixelType, index, pixelTypeToMatch);}
ImagePPStatus HRAImageOp::GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) 
    {return _GetAvailableOutputPixelType(pixelType, index, pixelTypeToMatch);}
ImagePPStatus HRAImageOp::SetInputPixelType(HFCPtr<HRPPixelType> pixelType)     {return _SetInputPixelType(pixelType);}
ImagePPStatus HRAImageOp::SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)    {return _SetOutputPixelType(pixelType);}
ImagePPStatus HRAImageOp::Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) {return _Process(out, inputData, params);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOp::HRAImageOp(HFCPtr<HRPPixelNeighbourhood> pPixelNeighbourhood)
    :m_pPixelNeighbourhood(pPixelNeighbourhood)
    {
    HPRECONDITION(pPixelNeighbourhood != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOp::HRAImageOp()
    {
    // Caller must setup m_pPixelNeighbourhood.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOp::~HRAImageOp()
    {
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------HRAImageOpPixelConverter----------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
HRAImageOpPtr HRAImageOpPixelConverter::CreatePixelConverter(bool alphaBlend)
    {
    return new HRAImageOpPixelConverter(alphaBlend);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPixelConverter::HRAImageOpPixelConverter(bool alphaBlend)
    :HRAImageOp(new HRPPixelNeighbourhood(1,1,0,0)),
    m_alphaBlend(alphaBlend)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPixelConverter::~HRAImageOpPixelConverter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPixelConverter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    return IMAGEOP_STATUS_NoMorePixelType;      // We have no default.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPixelConverter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    return IMAGEOP_STATUS_NoMorePixelType;      // We have no default.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPixelConverter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pInputPixelType = NULL;
        m_pConverter = NULL;
        return IMAGEPP_STATUS_Success;
        }

    // If OUTPUT is already provided they must be of same type.
    if(GetOutputPixelType() != NULL)
        {
        m_pConverter = GetOutputPixelType()->GetConverterFrom(pixelType);
        if(m_pConverter == NULL)
            return IMAGEOP_STATUS_InvalidPixelType;
        }

    m_pInputPixelType = pixelType;
    
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPixelConverter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pOutputPixelType = NULL;
        m_pConverter = NULL;
        return IMAGEPP_STATUS_Success;
        }

    // If INPUT is already provided they must be of same type.
    if(GetInputPixelType() != NULL)
        {
        m_pConverter = GetInputPixelType()->GetConverterTo(pixelType);
        if(m_pConverter == NULL)
            return IMAGEOP_STATUS_InvalidPixelType;
        }

    m_pOutputPixelType = pixelType;

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPixelConverter::_Process(HRAImageSampleR outData, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
    HPRECONDITION(GetInputPixelType() != NULL && GetOutputPixelType() != NULL);
    HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
    HPRECONDITION(inputData.GetWidth() == outData.GetWidth() + GetNeighbourhood().GetWidth() - 1);
    HPRECONDITION(inputData.GetHeight() == outData.GetHeight() + GetNeighbourhood().GetHeight() - 1);

    size_t inPitch, outPitch;
    Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t height = outData.GetHeight();
    uint32_t width = outData.GetWidth();

    HRAImageBufferRleP pBufferRle = outData.GetBufferRleP();

    if(m_alphaBlend && GetInputPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
        {
        for(uint32_t row=0; row < height; ++row)
            {
            m_pConverter->Compose(pInBuffer+row*inPitch, pOutBuffer+row*outPitch, width);
            if (pBufferRle != NULL)
                pBufferRle->ComputeDataSize(row, width);
            }
        }
    else
        {
        for(uint32_t row=0; row < height; ++row)
            {
            m_pConverter->Convert(pInBuffer+row*inPitch, pOutBuffer+row*outPitch, width);
            if (pBufferRle != NULL)
                pBufferRle->ComputeDataSize(row, width);
            }
        }
    
    return IMAGEPP_STATUS_Success;
    }


/*-------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------HRAImageOpPipeLine------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPipeLine::HRAImageOpPipeLine()
:m_pPixelNeighbourhood(NULL),
 m_prepared(false)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Alain.Robert  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPipeLine::~HRAImageOpPipeLine()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpPipeLine::IsEmpty() const
    {
    return m_imageOpList.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Alain.Robert  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPipeLine::AddImageOp(HRAImageOpPtr imageOp, bool atFront)
    {
    if(atFront)
        m_imageOpList.push_front(imageOp);
    else
        m_imageOpList.push_back(imageOp);

    m_pPixelNeighbourhood = NULL;
    m_prepared = false;

    return IMAGEPP_STATUS_Success;   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageOpList const& HRAImageOpPipeLine::GetImageOps() const
    {
    return m_imageOpList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Alain.Robert  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpPipeLine::Clear()
    {
    m_imageOpList.clear();

    m_prepared = false;
    m_pPixelNeighbourhood = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Alain.Robert  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HRAImageOpPipeLine::GetCount() const
    {
    return m_imageOpList.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPipeLine::AddConverterToFirstAvailableInputPixelType(ImageOpItr& whereItr, HFCPtr<HRPPixelType> pPixelTypeToMatch)
    {
    ImagePPStatus status = IMAGEOP_STATUS_CannotNegociatePixelType;

    HFCPtr<HRPPixelType> pAvailInputPixelType = NULL;
    if (IMAGEPP_STATUS_Success != (status = (*whereItr)->GetAvailableInputPixelType(pAvailInputPixelType, 0, pPixelTypeToMatch)))
        {
        // Convert indexed pixel type to their value equivalent.
        // N.B. this is only an attempt that we made to accommodate an imageOP. There is no default(v24 , v32, v64...)
        // that would accomodate every imageOP. Any other conversion must be coming from the GetAvailableInputPixelType() implementation of the imageOp. 
        if (pPixelTypeToMatch->CountIndexBits() != 0 && pPixelTypeToMatch->GetPalette().CountUsedEntries() > 0)
            {
            if(NULL != (pAvailInputPixelType = HRPPixelTypeFactory::GetInstance()->Create(pPixelTypeToMatch->GetPalette().GetChannelOrg(), 0)))
                status = IMAGEPP_STATUS_Success;
            }
        }

    if (pAvailInputPixelType != NULL &&
        IMAGEPP_STATUS_Success == (status = (*whereItr)->SetInputPixelType(pAvailInputPixelType)))
        {
        HRAImageOpPtr pConverter = HRAImageOpPixelConverter::CreatePixelConverter(false/*alphaBlend*/);
        if (IMAGEPP_STATUS_Success == pConverter->SetInputPixelType(pPixelTypeToMatch) &&
            IMAGEPP_STATUS_Success == pConverter->SetOutputPixelType((*whereItr)->GetInputPixelType()))
            {
            m_imageOpList.insert(whereItr, pConverter);
            status = IMAGEPP_STATUS_Success;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Alain.Robert  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPipeLine::Prepare(HFCPtr<HRPPixelType> pInputPixelType, HFCPtr<HRPPixelType> pOutputPixelType)
    {
    if (m_prepared)
        return IMAGEPP_STATUS_Success;

    if (m_imageOpList.size() == 0)
        {
        m_prepared = true;
        return IMAGEPP_STATUS_Success;
        }

    // Validations

    // Hand shake pixel type between operators
    // First try to match the input pixel type to first ImageOp or add a pixel converter

    // Various attempts have been made to determine how to perform pixel type handshake

    // The most important item is that we expect to preserve the input pixel type as long as possible
    // since the image operations should probably be performed as much as possible upon the native original
    // pixel type we try to maintain the input pixel type as long as we can. 
    // The prefered algorythm is thus to set as input pixel type of operations to the input image pixel type.
    // The output pixel type of operations will likewise be set to this same pixel type. 


    //
    // This strategy will not work correctly for some pixel operations such as DEM filter.
    // The DEM filter requires any monochannel pixel type on input and exits V32RGBA as output.
    // Future version will likely output V24 eventually.
    // The Alpha replacer image operation takes any value type pixel type and outputs a pixel type similar but with
    // additional Alpha channel (V24 becomes V32, V48 becomes V64, V8 becomes V8A8 ...)
    // 
    // From this can be concluded the fact that although we want to specify the input pixel type, the output
    // pixel type can be decided automatically as a result in part or completely.
    // We can possibly implement a GetAvailableInputPixelType() and GetAvailableOutputPixelType() methods that would
    // return a list of possibly supported pixel types. The list returned will depend if either input or output or both pixel types
    // of the image operation has been set. If the input pixel type has been specifically set then the availables input Pixel types will be
    // this pixel type alone. Likewise for output. If however the input has been specified then only possible outputs are returned as available that
    // fit with this constraint.
    // For example with the AlphaReplacer, setting the input to V8 will automatically limit the available outputs to V8A8. 
    // ??? Likewise if the output pixel type is set first then the input pixel type will be set to V8 automatically.
    // This strategy will allow to perform either input or output pixel type driving handshake.
    // 
    // Now for the actual operation:
    // First we have a current desired pixel type which is initially the input image pixel type.
    // - We set the input pixel type of the Op
    //   * If it fails we obtain the list of prefered inputs and add a conversion (?) Current PT is modified
    // - We set the output pixel type to current
    //   * If we fail we attempt to set the current pixel type to the final output pixel type and try again
    //   * If we fail we obtain a list of available output pixel type given previously selected input PT
    //     We set the output pixel type to first suggested ... (It cannot fail) and this becomes our current PT
    // - We loop until all ImageOps are handshaken or fail ... 
    //    * Do we backtrack in case 

    ImagePPStatus status = IMAGEOP_STATUS_CannotNegociatePixelType;

    // Need to reset what was previous set.  MT Sharing problem ahead?
    for (auto opItr : m_imageOpList)
        {
        opItr->SetInputPixelType(NULL);
        opItr->SetOutputPixelType(NULL);
        }

    ImageOpItr imagepOpItr = m_imageOpList.begin(); 

    // 1) Connect the first operation with the source pixeltype. We attempt to set it first because imageOp impl of GetAvailableInputPixelType might
    // ignore the 'pixelTypeToMatch' and return an arbitrary pixeltype but it could still be able to accept our current input.
    if (IMAGEPP_STATUS_Success != (status = (*imagepOpItr)->SetInputPixelType(pInputPixelType)))
        {
        // The imageOp cannot accept the source we need a converter.
        if (IMAGEPP_STATUS_Success != (status = AddConverterToFirstAvailableInputPixelType(imagepOpItr, pInputPixelType)))
            return status;
        }
    
    // 2) Connect imageOps together up to the last one.
    for (;imagepOpItr != m_imageOpList.end(); ++imagepOpItr)
        {
        // At this point we assumed that imagepOpItr have an inputPixelType but not an output.
        BeAssert((*imagepOpItr)->GetInputPixelType() != NULL);
        BeAssert((*imagepOpItr)->GetOutputPixelType() == NULL);

        ImageOpItr nextImageOpItr = imagepOpItr; 
        ++nextImageOpItr;
        if(nextImageOpItr == m_imageOpList.end())
            break;
                
        BeAssert((*nextImageOpItr)->GetInputPixelType() == NULL && (*nextImageOpItr)->GetOutputPixelType() == NULL);

        // Try to preserve current pixeltype
        if (IMAGEPP_STATUS_Success != (*imagepOpItr)->SetOutputPixelType((*imagepOpItr)->GetInputPixelType()))
            {
            // The imageOp cannot preserve input. Use first suggestion. EN: Negotiate that with the next OP.
            HFCPtr<HRPPixelType> pAvailableOutputPixeltype;
            if (IMAGEPP_STATUS_Success != (status = (*imagepOpItr)->GetAvailableOutputPixelType(pAvailableOutputPixeltype, 0, (*imagepOpItr)->GetInputPixelType())) ||
                IMAGEPP_STATUS_Success != (status = (*imagepOpItr)->SetOutputPixelType(pAvailableOutputPixeltype)))
                {
                BeAssert(!"HRAImageOpPipeLine::Prepare() PixelType negociation failure");
                return status;
                }
            }

        // imagepOpItr is fine or we return because we could not connect. Setup input of next imageOp.
        BeAssert((*imagepOpItr)->GetInputPixelType() != NULL && (*imagepOpItr)->GetOutputPixelType() != NULL);

        // Attempt to preserve the current pixel type.
        if (IMAGEPP_STATUS_Success != (status = (*nextImageOpItr)->SetInputPixelType((*imagepOpItr)->GetOutputPixelType())))
            {
            // Next imageOp did not accept our current output. 
            // Try to set input of current imageOp with available outputs from previous imageOp
            for (uint32_t queryIndex = 0;; ++queryIndex)
                {
                HFCPtr<HRPPixelType> pAvailableOutputPixeltype;
                if (IMAGEPP_STATUS_Success != (status = (*imagepOpItr)->GetAvailableOutputPixelType(pAvailableOutputPixeltype, queryIndex, (*imagepOpItr)->GetOutputPixelType())))
                    break;

                // We already tried this one.
                if (pAvailableOutputPixeltype->HasSamePixelInterpretation(*(*imagepOpItr)->GetOutputPixelType()))
                    continue;

                if (IMAGEPP_STATUS_Success == (status = (*nextImageOpItr)->SetInputPixelType(pAvailableOutputPixeltype)))
                    {
                    // This operation cannot failed, That pixel type came from the available list.
                    status = (*imagepOpItr)->SetOutputPixelType(pAvailableOutputPixeltype);
                    BeAssert(IMAGEPP_STATUS_Success == status);
                    break;
                    }
                }
            }

        // If we failed to connect. Add a converter.
        if ((*nextImageOpItr)->GetInputPixelType() == NULL)
            {
            if(IMAGEPP_STATUS_Success != (status = AddConverterToFirstAvailableInputPixelType(nextImageOpItr, (*imagepOpItr)->GetOutputPixelType())))
                {
                BeAssert(!"HRAImageOpPipeLine::Prepare() PixelType negociation failure");
                return status;
                }            
            BeAssert((*imagepOpItr)->GetInputPixelType() != NULL && (*imagepOpItr)->GetOutputPixelType() != NULL);
            ++imagepOpItr; // move to convert
            }

        // End loop validation before we proceed to the next op.
        BeAssert((*imagepOpItr)->GetInputPixelType() != NULL && (*imagepOpItr)->GetOutputPixelType() != NULL);
        BeAssert((*nextImageOpItr)->GetInputPixelType() != NULL && (*nextImageOpItr)->GetOutputPixelType() == NULL);
        }

    //3) Last imageOp, attempt to output destination directly.  If unsuccessful, the output merger will do the final conversion.
    if (IMAGEPP_STATUS_Success != (status = (*imagepOpItr)->SetOutputPixelType(pOutputPixelType)))
        {
        // The imageOp cannot accept the output pixeltype. Use first available output, the outputmerger will do the final conversion.
        HFCPtr<HRPPixelType> pAvailableOutputPixeltype;
        if (IMAGEPP_STATUS_Success != (status = (*imagepOpItr)->GetAvailableOutputPixelType(pAvailableOutputPixeltype, 0, (*imagepOpItr)->GetInputPixelType())) ||
            IMAGEPP_STATUS_Success != (status = (*imagepOpItr)->SetOutputPixelType(pAvailableOutputPixeltype)))
            {             
            BeAssert(!"HRAImageOpPipeLine::Prepare() PixelType negociation failure");
            return status;      
            }
        }

    m_prepared = IMAGEPP_STATUS_Success == status;

#ifdef __HMR_DEBUG
    if (m_prepared)
        {
        HFCPtr<HRPPixelType> __pInPixelType = pInputPixelType;

        for (auto _opItr : m_imageOpList)
            {
            if (_opItr->GetInputPixelType() == NULL || !_opItr->GetInputPixelType()->HasSamePixelInterpretation(*__pInPixelType) ||
                _opItr->GetOutputPixelType() == NULL) 
                {
                BeAssert(!"HRAImageOpPipeLine::Prepare() PixelType negociation failure");
                break;
                }
            __pInPixelType = _opItr->GetOutputPixelType();
            }
        }
#endif

    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAImageOpPipeLine::Process(ImagePPStatus& status, HRAImageSampleR inSample, ImagepOpParams& params, IImageAllocatorR allocator)
    {
    BeAssert(!IsEmpty());   // Avoid calling when empty?
    BeAssert(m_prepared);

    if (!m_prepared)
        {
        status = IMAGEPP_STATUS_UnknownError;
        return &inSample;
        }
        
    HRAImageSamplePtr pInSample = &inSample;

    for (auto imageOpItr : m_imageOpList)
        {
        // &&OPTIMIZATION improve allocation. Reuse sample and buffer instance??
        HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, pInSample->GetWidth()-(imageOpItr->GetNeighbourhood().GetWidth()-1), 
                                                                    pInSample->GetHeight()-(imageOpItr->GetNeighbourhood().GetHeight()-1),
                                                                    imageOpItr->GetOutputPixelType(), allocator);

        if (IMAGEPP_STATUS_Success != status)
            break;

        if (IMAGEPP_STATUS_Success != (status = imageOpItr->Process(*pOutSample, *pInSample, params)))
            break;

        params.SetOffset(params.GetOffsetX() + imageOpItr->GetNeighbourhood().GetXOrigin(),
                         params.GetOffsetY() + imageOpItr->GetNeighbourhood().GetYOrigin());

        pInSample = pOutSample;
        }

    return pInSample;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRPPixelNeighbourhood const& HRAImageOpPipeLine::GetNeighbourdhood() const
    {
    if(m_pPixelNeighbourhood == NULL)
        {
        m_pPixelNeighbourhood = new HRPPixelNeighbourhood(1,1,0,0);     //reinit unity

        for (ImageOpList::const_reverse_iterator imageOpItr = m_imageOpList.rbegin(); imageOpItr != m_imageOpList.rend(); ++imageOpItr)
            {
            // Cumulate
            *m_pPixelNeighbourhood = *m_pPixelNeighbourhood + (*imageOpItr)->GetNeighbourhood();
            }
        }

    return *m_pPixelNeighbourhood;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRPPixelType> HRAImageOpPipeLine::GetOutputPixelType() const
    {
    BeAssert(IsReady());

    return m_imageOpList.empty() ? NULL : m_imageOpList.crbegin()->get()->GetOutputPixelType();
    }
