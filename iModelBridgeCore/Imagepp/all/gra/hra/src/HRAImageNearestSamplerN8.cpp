//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageNearestSamplerN8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <ImagePPInternal/gra/HRAImageNearestSamplerN8.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/HGF2DCoordSys.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <ImagePPInternal/gra/HRAImageSurface.h>



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSampler* HRAImageSampler::CreateNearestN8(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType)
    {
    if (pixelType.CountPixelRawDataBits() % 8 != 0)
        return NULL;

    unique_ptr<HRAImageSampler> pSampler;

    size_t samplePixelSize = pixelType.CountPixelRawDataBits() / 8;
    switch (samplePixelSize)
        {
        case 1:
            pSampler.reset(new HRAImageNearestSamplerN8<1>(destToSrcTransfo));
            break;
        case 2:
            pSampler.reset(new HRAImageNearestSamplerN8<2>(destToSrcTransfo));
            break;
        case 3:
            pSampler.reset(new HRAImageNearestSamplerN8<3>(destToSrcTransfo));
            break;
        case 4:
            pSampler.reset(new HRAImageNearestSamplerN8<4>(destToSrcTransfo));
            break;
        case 6:
            pSampler.reset(new HRAImageNearestSamplerN8<6>(destToSrcTransfo));
            break;
        case 8:
            pSampler.reset(new HRAImageNearestSamplerN8<8>(destToSrcTransfo));
            break;
        case 12:
            pSampler.reset(new HRAImageNearestSamplerN8<12>(destToSrcTransfo));
            break;
        default:
            pSampler.reset();
            break;
        }

    return  pSampler.release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t PixelSize_T>
template<class Surface_T>
ImagePPStatus HRAImageNearestSamplerN8<PixelSize_T>::Stretch_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
    {
    size_t inPitch;
    Byte const* pInBuffer = inData.GetDataCP(inPitch);

    size_t outPitch;
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t inWidth = inData.GetWidth();
    uint32_t inHeight = inData.GetHeight();

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();

    double deltaX = m_scaleX;
    double deltaY = m_scaleY;

    RawBuffer_T<uint32_t, decltype(m_allocator)> offsets(outWidth, m_allocator);
    uint32_t* pPixelsOffset = &offsets.At(0);

    // Build the pre-computed offset array. This is the fastest technique we have so far.
    for(uint32_t w=0; w < outWidth; ++w)
        {
        int32_t columnInSrc = (int32_t)((m_offsetX + ((w+outOffset.x+0.5)*deltaX)) - inOffset.x);
        pPixelsOffset[w] = (uint32_t)((MAX(0, MIN(columnInSrc, (int32_t)(inWidth-1))))*PixelSize_T);
        }

    // Define a lambda to process a line.
    // Can be called by several threads.
    auto lineProcessor = [&](uint32_t row)
        {
        int32_t rowInSrc = (int32_t)((m_offsetY + ((row+outOffset.y+0.5)*deltaY)) - inOffset.y);
        uint32_t effectiveRowInSrc = MAX(0, MIN(rowInSrc, (int32_t)(inHeight-1)));

        Byte*       pOutBufferLine = pOutBuffer+row*outPitch;
        Byte const* pInBufferLine  = pInBuffer+effectiveRowInSrc*inPitch;

        for(uint32_t column=0; column < outWidth; ++column)
            memcpy(pOutBufferLine + column*PixelSize_T, pInBufferLine + pPixelsOffset[column], PixelSize_T);
        };
    
    // Process all lines.
#if defined (_WIN32)
    if (!m_enableMultiThreading)
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
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t PixelSize_T>
template<class Surface_T>
ImagePPStatus HRAImageNearestSamplerN8<PixelSize_T>::Warp_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
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
* @bsimethod                                    Stephane.Poulin                 12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t PixelSize_T>
template<class Surface_T, class Allocator_T>
ImagePPStatus HRAImageNearestSamplerN8<PixelSize_T>::Warp_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset, Allocator_T& allocator)
    {
    size_t inPitch;
    Byte const* pInBuffer = inData.GetDataCP(inPitch);
    
    size_t outPitch;
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t inWidth = inData.GetWidth();
    uint32_t inHeight = inData.GetHeight();

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();
  
    // Define a lambda to process a line.
    // Can be called by several threads.
    auto lineProcessor = [&](uint32_t row)
        {
        RawBuffer_T<double, Allocator_T> positions(outWidth * 2, allocator);
        double* pXPositions = &positions.At(0);
        double* pYPositions = &positions.At(outWidth);

        // convert the destination run into the source coordinates
        // N.B. The Y component must be pass before X. How intuitive is that!
        m_pDestToSrcTransfo->ConvertDirect(row + outOffset.y + 0.5, outOffset.x + 0.5, outWidth, 1.0, pXPositions, pYPositions);

        Byte* pOutBufferLine = pOutBuffer + row*outPitch;

        for (uint32_t column = 0; column < outWidth; ++column)
            {
            int32_t rowInSrc = MAX(0, MIN((int32_t)(pYPositions[column] - inOffset.y), (int32_t)(inHeight - 1)));
            int32_t columnInSrc = MAX(0, MIN((int32_t)(pXPositions[column] - inOffset.x), (int32_t)(inWidth - 1)));

            memcpy(pOutBufferLine + column*PixelSize_T, pInBuffer + rowInSrc*inPitch + columnInSrc*PixelSize_T, PixelSize_T);
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
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t PixelSize_T>
HRAImageNearestSamplerN8<PixelSize_T>::HRAImageNearestSamplerN8(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo)
    :HRAImageSampler(HRPPixelNeighbourhood(1, 1, 0, 0), destToSrcTransfo)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t PixelSize_T>
HRAImageNearestSamplerN8<PixelSize_T>::~HRAImageNearestSamplerN8(){}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t PixelSize_T>
ImagePPStatus HRAImageNearestSamplerN8<PixelSize_T>::_Stretch(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() == inData.GetPixelType().CountPixelRawDataBits());
    HPRECONDITION(outData.GetBufferP() != NULL);
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() % 8 == 0);

    SamplerSurfaceVisitor<HRAImageNearestSamplerN8, true/*IsStretch*/> visitor(*this, outData, outOffset, inOffset);

    return inData.Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t PixelSize_T>
ImagePPStatus HRAImageNearestSamplerN8<PixelSize_T>::_Warp(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() == inData.GetPixelType().CountPixelRawDataBits());
    HPRECONDITION(outData.GetBufferP() != NULL);
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() % 8 == 0);

    //&&OPTIMIZATION: 
    //          - can we optimize m_destToSrcTransfo.ConvertDirect? ex: step could be hardcoded to 1.0 instead of being variable
    //            We should introduce SSE into transfo model and while we are at it we could do the min max in SEE too.
    //          - keep pointer a WarpNearest_T<T> for direct access.
    SamplerSurfaceVisitor<HRAImageNearestSamplerN8, false/*IsStretch*/> visitor(*this, outData, outOffset, inOffset);

    return inData.Accept(visitor);
    }
