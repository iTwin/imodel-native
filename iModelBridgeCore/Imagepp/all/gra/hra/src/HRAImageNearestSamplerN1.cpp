//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageNearestSamplerN1.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <ImagePPInternal/gra/HRAImageNearestSamplerN1.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <ImagePPInternal/gra/HRAImageSurface.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8RLE.h>


    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSampler* HRAImageSampler::CreateNearestN1(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType)
    {
    if (pixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || pixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
        return NULL;

    if (pixelType.CountPixelRawDataBits() == 1)
        return new HRAImageNearestSamplerN1(destToSrcTransfo);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Surface_T>
ImagePPStatus HRAImageNearestSamplerN1::Stretch_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
    {
    size_t inPitch;
    Byte const* pInBuffer = inData.GetDataCP(inPitch);

    size_t outPitch;
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t inWidth = inData.GetWidth();
    uint32_t inHeight = inData.GetHeight();

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();

    RawBuffer_T<uint32_t, decltype(m_allocator)> offsets(outWidth, m_allocator);
    uint32_t* pPixelsOffset = &offsets.At(0);

    // Build the pre-computed offset array. This is the fastest technique we have so far.
    for (uint32_t w = 0; w < outWidth; ++w)
        {
        int32_t columnInSrc = (int32_t)((m_offsetX + ((w + outOffset.x + 0.5)*m_scaleX)) - inOffset.x);
        pPixelsOffset[w] = (uint32_t)((MAX(0, MIN(columnInSrc, (int32_t)(inWidth - 1)))));
        }

    // Process all lines.
    for (uint64_t row = 0; row < outHeight; ++row)
        {
        int32_t rowInSrc = (int32_t)((m_offsetY + ((row + outOffset.y + 0.5)*m_scaleY)) - inOffset.y);
        uint32_t effectiveRowInSrc = MAX(0, MIN(rowInSrc, (int32_t)(inHeight - 1)));

        Byte* pOutBufferLine = pOutBuffer + row*outPitch;
        Byte const* pInBufferLine = pInBuffer + effectiveRowInSrc*inPitch;
        const uint32_t fullBytes = outWidth / 8;
        for (uint32_t x = 0; x < fullBytes; ++x)
            {
            const uint32_t column = x*8;

            pOutBufferLine[x] =
                (((pInBufferLine[pPixelsOffset[column + 0] / 8] << (pPixelsOffset[column + 0] % 8)) & 0x80) >> 0) |
                (((pInBufferLine[pPixelsOffset[column + 1] / 8] << (pPixelsOffset[column + 1] % 8)) & 0x80) >> 1) |
                (((pInBufferLine[pPixelsOffset[column + 2] / 8] << (pPixelsOffset[column + 2] % 8)) & 0x80) >> 2) |
                (((pInBufferLine[pPixelsOffset[column + 3] / 8] << (pPixelsOffset[column + 3] % 8)) & 0x80) >> 3) |
                (((pInBufferLine[pPixelsOffset[column + 4] / 8] << (pPixelsOffset[column + 4] % 8)) & 0x80) >> 4) |
                (((pInBufferLine[pPixelsOffset[column + 5] / 8] << (pPixelsOffset[column + 5] % 8)) & 0x80) >> 5) |
                (((pInBufferLine[pPixelsOffset[column + 6] / 8] << (pPixelsOffset[column + 6] % 8)) & 0x80) >> 6) |
                (((pInBufferLine[pPixelsOffset[column + 7] / 8] << (pPixelsOffset[column + 7] % 8)) & 0x80) >> 7);
            }

        // Process remainder
        for (uint32_t column = fullBytes*8; column < outWidth; ++column)
            {
            const uint32_t outIdx = column / 8;
            const uint32_t outBit = column % 8;
            const uint32_t inIdx = pPixelsOffset[column] / 8;
            const uint32_t inBit = pPixelsOffset[column] % 8;

            pOutBufferLine[outIdx] = (pInBufferLine[inIdx] & (0x80 >> inBit)) ?
                pOutBufferLine[outIdx] | (0x80 >> outBit) : /* SetBit*/
                pOutBufferLine[outIdx] & ~(0x80 >> outBit); /* ClearBit*/
            }
        }

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Surface_T>
ImagePPStatus HRAImageNearestSamplerN1::Warp_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
    {
    if (!m_pDestToSrcTransfo->IsConvertDirectThreadSafe())
        {
        return Warp_T<Surface_T, SingleBlockAllocator>(outData, outOffset, inData, inOffset, m_allocator);
        }
    else
        {
        ConcurrencyAllocator allocator;
        return Warp_T<Surface_T, ConcurrencyAllocator>(outData, outOffset, inData, inOffset, allocator);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Surface_T, class Allocator_T>
ImagePPStatus HRAImageNearestSamplerN1::Warp_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset, Allocator_T& allocator)
    {
    size_t inPitch;
    Byte const* pInBuffer = inData.GetDataCP(inPitch);
    
    size_t outPitch;
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t inWidth = inData.GetWidth();
    uint32_t inHeight = inData.GetHeight();

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();

    // Process all lines.
    auto lineProcessor = [&](uint32_t row)
        {
        RawBuffer_T<double, Allocator_T> positions(outWidth * 2, allocator);
        double* pXPositions = &positions.At(0);
        double* pYPositions = &positions.At(outWidth);

        // convert the destination run into the source coordinates
        // N.B. The Y component must be pass before X. How intuitive is that!
        m_pDestToSrcTransfo->ConvertDirect(row + outOffset.y + 0.5, outOffset.x + 0.5, outWidth, 1.0, pXPositions, pYPositions);

        Byte* pOutBufferLine = pOutBuffer+row*outPitch;

        for(uint32_t column=0; column < outWidth; ++column)
            {
            const int32_t rowInSrc = MAX(0, MIN((int32_t)(pYPositions[column] - inOffset.y), (int32_t)(inHeight-1)));
            const int32_t columnInSrc = MAX(0, MIN((int32_t)(pXPositions[column] - inOffset.x), (int32_t)(inWidth-1)));

            const uint32_t outIdx = column / 8;
            const uint32_t outBit = column % 8;
            const uint32_t inIdx = columnInSrc / 8;
            const uint32_t inBit = columnInSrc % 8;

            pOutBufferLine[outIdx] =
                ((pInBuffer + rowInSrc*inPitch)[inIdx] & (0x80 >> inBit)) ?
                pOutBufferLine[outIdx] | (0x80 >> outBit) : /* SetBit*/
                pOutBufferLine[outIdx] & ~(0x80 >> outBit); /* ClearBit*/
            }
        };

    // Process all lines.
#if defined (_WIN32)
    if (!m_enableMultiThreading || !m_pDestToSrcTransfo->IsConvertDirectThreadSafe())
#endif
        {
        for (uint32_t row = 0; row < outHeight; ++row)
            lineProcessor(row);
        }
#if defined (_WIN32)
    else
        {
        Concurrency::parallel_for<uint32_t>(0, outHeight, lineProcessor);
        }
#endif
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageNearestSamplerN1::HRAImageNearestSamplerN1(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo)
    :HRAImageSampler(HRPPixelNeighbourhood(1, 1, 0, 0), destToSrcTransfo)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageNearestSamplerN1::~HRAImageNearestSamplerN1(){}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageNearestSamplerN1::_Stretch(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() == inData.GetPixelType().CountPixelRawDataBits());
    HPRECONDITION(outData.GetBufferP() != NULL);
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() % 8 == 1);

    SamplerSurfaceVisitor<HRAImageNearestSamplerN1, true/*IsStretch*/> visitor(*this, outData, outOffset, inOffset);

    return inData.Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageNearestSamplerN1::_Warp(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() == inData.GetPixelType().CountPixelRawDataBits());
    HPRECONDITION(outData.GetBufferP() != NULL);
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() % 8 == 1);

    SamplerSurfaceVisitor<HRAImageNearestSamplerN1, false/*IsStretch*/> visitor(*this, outData, outOffset, inOffset);

    return inData.Accept(visitor);
    }
