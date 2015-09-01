//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageSurface.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <ImagePPInternal/gra/HRAImageSurface.h>
#include <ImagePPInternal/gra/HRAImageEditor.h>
#include <Imagepp/all/h/HRAImageOp.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRPPixelConverter.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>

#include <ImagePP/all/h/HCDPacket.h>    
#include <ImagePP/all/h/HCDPacketRLE.h> 
#include <ImagePP/all/h/HRABitmapRLE.h> 
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSurface::HRAImageSurface(uint32_t width, uint32_t height, HFCPtr<HRPPixelType> const& pPixelType)
:m_width(width),
 m_height(height),
 m_pixelType(pPixelType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSurface::~HRAImageSurface(){}
uint32_t HRAImageSurface::GetWidth() const { return m_width; }
uint32_t HRAImageSurface::GetHeight() const{ return m_height; }
HRPPixelType const& HRAImageSurface::GetPixelType() const { return *m_pixelType; }
HFCPtr<HRPPixelType> HRAImageSurface::GetPixelTypePtr() { return m_pixelType; }
HRAImageSamplePtr HRAImageSurface::CopyToSample(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset, IImageAllocatorR allocator) const { return _CopyToSample(status, width, height, offset, allocator); }
ImagePPStatus HRAImageSurface::Accept(SurfaceVisitor& visitor) { return _Accept(visitor); }
HRAImageSamplePtr HRAImageSurface::CreateBufferReference(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset) const { return _CreateBufferReference(status, width, height, offset); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSurfacePtr HRAPacketSurface::Create(uint32_t width, uint32_t height, HFCPtr<HRPPixelType> const& pPixelType, HFCPtr<HCDPacket>& packet, size_t pitch)
    {
    BeAssert(NULL == packet->GetCodec().GetPtr() || packet->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID));

    if (packet->GetBufferAddress() == NULL)        
        return NULL;

    if (pPixelType->CountPixelRawDataBits() % 8 == 0) 
        return new HRAPacketN8Surface(width, height, pPixelType, packet, pitch);
        
    if (pPixelType->CountPixelRawDataBits() == 1)
        return new HRAPacketN1Surface(width, height, pPixelType, packet, pitch);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAPacketSurface::HRAPacketSurface(uint32_t width, uint32_t height, HFCPtr<HRPPixelType> const& pPixelType, HFCPtr<HCDPacket>& packet)
:HRAImageSurface(width, height, pPixelType),
m_packet(packet)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HCDPacket> HRAPacketSurface::GetPacketPtr() const { return m_packet;}
HCDPacket const& HRAPacketSurface::GetPacket() const { return *m_packet;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Byte* HRAPacketN1Surface::GetDataP(size_t& pitch)
    {
    pitch = m_pitch;
    return m_packet->GetBufferAddress();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAPacketN1Surface::HRAPacketN1Surface(uint32_t width, uint32_t height, HFCPtr<HRPPixelType> const& pPixelType, HFCPtr<HCDPacket>& packet, size_t pitch)
:HRAPacketSurface(width, height, pPixelType, packet), m_pitch(pitch)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAPacketN1Surface::_CopyToSample(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset, IImageAllocatorR allocator) const
    {
    HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, width, height, m_pixelType, allocator);
    if(IMAGEPP_STATUS_Success != status)
        return NULL;

    size_t outPitch;
    Byte* pOutBuffer = pOutSample->GetBufferP()->GetDataP(outPitch);

    size_t inPitch;
    Byte const* pInBuffer = GetDataCP(inPitch);

    CopyPixelsN1(pOutSample->GetWidth(), pOutSample->GetHeight(), pOutBuffer, outPitch, GetWidth(), GetHeight(), pInBuffer, inPitch, offset);

    return pOutSample;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAPacketN1Surface::_CreateBufferReference(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset) const
    {
    // For now accept only perfect match.
    if (NULL != GetPacket().GetBufferAddress() && GetWidth() == width && GetHeight() == height && offset.x == 0 && offset.y == 0)
        {
        HRAImageBufferPtr  pBuffer = HRAImageBufferParasiteMemoryWithHold<HFCPtr<HCDPacket>>::
            CreateBuffer(status, GetPacketPtr()->GetBufferAddress(), GetPacket().GetDataSize(), m_pitch, GetPacketPtr());

        if (IMAGEPP_STATUS_Success != status)
            return NULL;

        return HRAImageSample::internal_CreateSampleFromBuffer(status, width, height, m_pixelType, *pBuffer);
        }

    status = IMAGEPP_STATUS_UnknownError;
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* HRAPacketN8Surface
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAPacketN8Surface::HRAPacketN8Surface(uint32_t width, uint32_t height, HFCPtr<HRPPixelType> const& pPixelType, HFCPtr<HCDPacket>& packet, size_t pitch)
:HRAPacketSurface(width, height, pPixelType, packet),
 m_pitch(pitch)
    {
    }
        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Byte* HRAPacketN8Surface::GetDataP(size_t& pitch)
    {
    pitch = m_pitch;
    return m_packet->GetBufferAddress();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAPacketN8Surface::_CopyToSample(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset, IImageAllocatorR allocator) const
    {
    HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, width, height, m_pixelType, allocator);
    if(IMAGEPP_STATUS_Success != status)
        return NULL;   

    uint32_t samplePixelSize = GetPixelType().CountPixelRawDataBits() / 8;
    size_t outPitch;
    Byte* pOutBuffer = pOutSample->GetBufferP()->GetDataP(outPitch);

    status = CopyPixelsN8(pOutSample->GetWidth(), pOutSample->GetHeight(), pOutBuffer, outPitch, 
                          GetWidth(), GetHeight(), GetPacket().GetBufferAddress(), m_pitch, 
                          samplePixelSize, offset);

    return pOutSample;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAPacketN8Surface::_CreateBufferReference(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset) const
    {
    // Can't reference outside
    if (offset.x < 0 || offset.x + GetWidth() > width || 
        offset.y < 0 || offset.y + GetHeight() > height)
        {
        status = IMAGEPP_STATUS_UnknownError;
        return NULL;
        }
    
    const size_t bytePerPixel = GetPixelType().CountPixelRawDataBits() / 8;

    HRAImageBufferPtr  pBuffer = HRAImageBufferParasiteMemoryWithHold<HFCPtr<HCDPacket>>::
        CreateBuffer(status, GetPacketPtr()->GetBufferAddress() + (offset.y * m_pitch) + (offset.x * bytePerPixel), 
                             (height-1)*m_pitch + (width * bytePerPixel), m_pitch, GetPacketPtr());

    if (IMAGEPP_STATUS_Success != status)
        return NULL;
    
    return HRAImageSample::internal_CreateSampleFromBuffer(status, width, height, m_pixelType, *pBuffer);
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------HRASampleSurface------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRASampleSurface::HRASampleSurface(HRAImageSampleR sample)
:HRAImageSurface(sample.GetWidth(), sample.GetHeight(), sample.GetPixelTypePtr()),
 m_pSample(&sample) // add a ref.
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRASampleSurfacePtr HRASampleSurface::Create(HRAImageSampleR sample)
    {
    if (sample.GetBufferP() == NULL)        
        return NULL;

    if (sample.GetPixelType().CountPixelRawDataBits() % 8 == 0) 
        {
        return new HRASampleN8Surface(sample);
        }

    if (sample.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || 
        sample.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID))
        return new HRASampleRleSurface(sample);

    if (sample.GetPixelType().CountPixelRawDataBits() == 1)
        return new HRASampleN1Surface(sample);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Byte const* HRASampleSurface::GetDataCP(size_t& pitch) const
    {
    return m_pSample->GetBufferCP()->GetDataCP(pitch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRASampleSurface::_CreateBufferReference(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset) const
    {
    return m_pSample->CreateBufferReference(status, width, height, (uint32_t)offset.x, (uint32_t)offset.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRASampleN8Surface::HRASampleN8Surface(HRAImageSampleR sample)
:HRASampleSurface(sample)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRASampleN8Surface::_CopyToSample(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset, IImageAllocatorR allocator) const
    {
    HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, width, height, m_pixelType, allocator);
    if(IMAGEPP_STATUS_Success != status)
        return NULL;   

    uint32_t samplePixelSize = GetPixelType().CountPixelRawDataBits() / 8;
    size_t outPitch;
    Byte* pOutBuffer = pOutSample->GetBufferP()->GetDataP(outPitch);

    size_t inPitch;
    Byte const* pInBuffer = GetDataCP(inPitch);

    status = CopyPixelsN8(pOutSample->GetWidth(), pOutSample->GetHeight(), pOutBuffer, outPitch, 
                          GetWidth(), GetHeight(), pInBuffer, inPitch, 
                          samplePixelSize, offset);

    return pOutSample;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HRASampleN8Surface::_Clear()
    {
    Byte const* rawData = static_cast<Byte const*>(GetPixelType().GetDefaultRawData());
    const uint32_t bytesPerPixel = GetPixelType().CountPixelRawDataBits() / 8;

    size_t pitch;
    Byte* pBuf = m_pSample->GetBufferP()->GetDataP(pitch);

    ImageEditor::Clear(m_width, m_height, pBuf, pitch, bytesPerPixel, rawData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRASampleN1Surface::HRASampleN1Surface(HRAImageSampleR sample)
:HRASampleSurface(sample)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRASampleN1Surface::_CopyToSample(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset, IImageAllocatorR allocator) const
    {
    HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, width, height, m_pixelType, allocator);
    if(IMAGEPP_STATUS_Success != status)
        return NULL;   
    
    size_t outPitch;
    Byte* pOutBuffer = pOutSample->GetBufferP()->GetDataP(outPitch);

    size_t inPitch;
    Byte const* pInBuffer = GetDataCP(inPitch);

    CopyPixelsN1(pOutSample->GetWidth(), pOutSample->GetHeight(), pOutBuffer, outPitch, GetWidth(), GetHeight(), pInBuffer, inPitch, offset);

    return pOutSample;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Byte* HRASampleN1Surface::GetDataP(size_t& pitch)
    {
    return GetSampleR().GetBufferP()->GetDataP(pitch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HRASampleN1Surface::_Clear()
    {
    size_t pitch;
    Byte* pBuf = m_pSample->GetBufferP()->GetDataP(pitch);

    Byte const* rawData = static_cast<Byte const*>(GetPixelType().GetDefaultRawData());
    const bool state = (0 != (rawData[0] & 0x80));

    ImageEditor::Clear(m_width, m_height, pBuf, pitch, state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRASampleRleSurface::HRASampleRleSurface(HRAImageSampleR sample)
:HRASampleSurface(sample)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRASampleRleSurface::_CopyToSample(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset, IImageAllocatorR allocator) const
    {
    HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, width, height, m_pixelType, allocator);
    if(IMAGEPP_STATUS_Success != status)
        return NULL;   
     
    HRAImageBufferRleP pOutRleBuf = pOutSample->GetBufferRleP();
    if (NULL == pOutRleBuf)
        return NULL;

    LineReader lineReader(*this);
    CopyPixelsRle_T(pOutSample->GetWidth(), pOutSample->GetHeight(), *pOutRleBuf, GetWidth(), GetHeight(), lineReader, offset);

    return pOutSample;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HRASampleRleSurface::_Clear()
    {
    // compute the byte to copy
    Byte const* rawData = static_cast<Byte const*>(GetPixelType().GetDefaultRawData());
    const bool state = (false != (rawData[0] & 0x80));

    ImageEditor::Clear(m_width, m_height, *m_pSample->GetBufferP()->AsBufferRleP(), state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAPacketRleSurfacePtr HRAPacketRleSurface::CreateSurface(uint32_t width, uint32_t height, HFCPtr<HRPPixelType> const& pPixelType, uint32_t stripHeight)
    {
    return new HRAPacketRleSurface(width, height, pPixelType, stripHeight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAPacketRleSurface::HRAPacketRleSurface(uint32_t width, uint32_t height, HFCPtr<HRPPixelType> const& pPixelType, uint32_t stripHeight)
:HRAImageSurface(width, height, pPixelType),
 m_stripHeight(stripHeight)
    {
    BeAssert(pPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAPacketRleSurface::_CopyToSample(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset64 const& offset, IImageAllocatorR allocator) const
    {
    HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, width, height, m_pixelType, allocator);
    if(IMAGEPP_STATUS_Success != status)
        return NULL;    
   
    HRAImageBufferRleP pOutRleBuf = pOutSample->GetBufferRleP();
    if (NULL == pOutRleBuf)
        {
        status = IMAGEPP_STATUS_UnknownError;
        return NULL;
        }

    LineReader lineReader(*this);
    CopyPixelsRle_T(pOutSample->GetWidth(), pOutSample->GetHeight(), *pOutRleBuf, GetWidth(), GetHeight(), lineReader, offset);

    return pOutSample;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAPacketRleSurface::AppendStrip(HFCPtr<HRABitmapRLE>& pStrip)
    {
    BeAssert(m_strips.size()*m_stripHeight < GetHeight());
    BeAssert(pStrip->GetPacket()->GetCodec()->GetWidth() == GetWidth());

    m_strips.push_back(StripsVector::value_type(pStrip, pStrip->GetPacket().GetPtr()));
    }


    