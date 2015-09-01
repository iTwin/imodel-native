//:>--------------------------------------------------------------------------------------+
    //:>
    //:>     $Source: all/gra/hra/src/HRAImageBicubicSamplerN8.cpp $
    //:>
    //:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
    //:>
    //:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
#include <ImagePPInternal/gra/HRAImageBicubicSamplerN8.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <ImagePPInternal/gra/ImageCommon.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/HGF2DCoordSys.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <ImageppInternal/gra/HRAImageSurface.h>



static const float s_a = -0.5;
static const float s_2a = -1;     // 2 * s_a
static const float s_2aP3 = 2.0;   // s_2a + 3.0
static const float s_aP2 = 1.5;   // s_a + 2.0
static const float s_aP3 = 2.5;   // s_a + 3.0

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSampler* HRAImageSampler::CreateBicubic(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType)
    {
    // if we have no data values, we cannot use cubic convolution because it will average the NoDataValue.
    if (pixelType.GetChannelOrg().GetChannelPtr(0)->GetNoDataValue() != NULL)
        return NULL;

    unique_ptr<HRAImageSampler> pSampler;

    switch (pixelType.GetClassID())
        {
        case HRPPixelTypeId_V8Gray8:
        case HRPPixelTypeId_V8GrayWhite8:
            pSampler.reset(new HRAImageBicubicSamplerN8<1, uint8_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V16Gray16:
            pSampler.reset(new HRAImageBicubicSamplerN8<1, uint16_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V16Int16:
            pSampler.reset(new HRAImageBicubicSamplerN8<1, int16_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V32Float32:
            pSampler.reset(new HRAImageBicubicSamplerN8<1, float>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V16PRGray8A8:
            pSampler.reset(new HRAImageBicubicSamplerN8<2, uint8_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V24R8G8B8:
        case HRPPixelTypeId_V24B8G8R8:
        case HRPPixelTypeId_V24PhotoYCC:
            pSampler.reset(new HRAImageBicubicSamplerN8<3, uint8_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V48R16G16B16:
            pSampler.reset(new HRAImageBicubicSamplerN8<3, uint16_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V96R32G32B32:
            pSampler.reset(new HRAImageBicubicSamplerN8<3, uint32_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V32PRPhotoYCCA8:
        case HRPPixelTypeId_V32PR8PG8PB8A8:
        case HRPPixelTypeId_V32CMYK:
        case HRPPixelTypeId_V32B8G8R8X8:
        case HRPPixelTypeId_V32R8G8B8A8:
        case HRPPixelTypeId_V32R8G8B8X8:
        case HRPPixelTypeId_V32A8R8G8B8:
            pSampler.reset(new HRAImageBicubicSamplerN8<4, uint8_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V64R16G16B16A16:
        case HRPPixelTypeId_V64R16G16B16X16:
            pSampler.reset(new HRAImageBicubicSamplerN8<4, uint16_t>(destToSrcTransfo));
            break;

            // not supported.
        case HRPPixelTypeId_I8R8G8B8:
        case HRPPixelTypeId_I8R8G8B8A8:
        case HRPPixelTypeId_I8Gray8:
        case HRPPixelTypeId_I8R8G8B8Mask:
        case HRPPixelTypeId_I8VA8R8G8B8:
        case HRPPixelTypeId_I1R8G8B8:
        case HRPPixelTypeId_I1R8G8B8RLE:
        case HRPPixelTypeId_I1R8G8B8A8:
        case HRPPixelTypeId_I1R8G8B8A8RLE:
        case HRPPixelTypeId_I2R8G8B8:
        case HRPPixelTypeId_I4R8G8B8:
        case HRPPixelTypeId_I4R8G8B8A8:
        case HRPPixelTypeId_V1Gray1:
        case HRPPixelTypeId_V1GrayWhite1:
        case HRPPixelTypeId_V16B5G5R5:
        case HRPPixelTypeId_V16R5G6B5:
        default:
            pSampler.reset();
            break;
        }

    return pSampler.release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
HRAImageBicubicSamplerN8<ChannelCount_T, Data_T>::HRAImageBicubicSamplerN8(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo)
   :HRAImageSampler(HRPPixelNeighbourhood(5,5,2,2), destToSrcTransfo)
    { 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
HRAImageBicubicSamplerN8<ChannelCount_T, Data_T>::~HRAImageBicubicSamplerN8(){}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
ImagePPStatus HRAImageBicubicSamplerN8<ChannelCount_T, Data_T>::_Stretch(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    SamplerSurfaceVisitor<HRAImageBicubicSamplerN8, true/*IsStretch*/> visitor(*this, outData, outOffset, inOffset);

    return inData.Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
ImagePPStatus HRAImageBicubicSamplerN8<ChannelCount_T, Data_T>::_Warp(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    SamplerSurfaceVisitor<HRAImageBicubicSamplerN8, false/*IsStretch*/> visitor(*this, outData, outOffset, inOffset);

    return inData.Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
float* HRAImageBicubicSamplerN8<ChannelCount_T, Data_T>::GetRealPixelOffSetBuffer(size_t size)
    {
    if (size > m_realPixelOffSet.size())
        m_realPixelOffSet.resize(size);

    return m_realPixelOffSet.data();
    }


/*---------------------------------------------------------------------------------**//**
* Note 1. (AG 11/2014) Implementation of Stretch_T (and Warp_T)
*  We have to add + 0.5 to the offsets and round at the max value of the image
*  to make sure that if we get the case marked with the x, we get the right neighbourhood
*  (outside of the image)
*  ___ ___        ___ ___
* | 1 | 2 | -0.5 | 1 | 2 |   We get the index "0" which is out of boundaries, but its value
* |___|___| ---> |___|___|   is required to place the 3 other input lines correctly.
* |x  |   |     x|   |   |   The same situation can happen with +0.5 at the other edge of the image
* |___|___|      |___|___|
* Calculations on the Y axis have to be done the same way
*
*   Neighborhood for bicubic, "x" being the pixel. There are 4 cases to be considered
* ==========================   ==========================   ==========================   ==========================
* |0000|0000|0000|0000|    |   |    |0000|0000|0000|0000|   |    |    |    |    |    |   |    |    |    |    |    |
* |0000|0000|0000|0000|    |   |    |0000|0000|0000|0000|   |    |    |    |    |    |   |    |    |    |    |    |
* ==========================   ==========================   ==========================   ==========================
* |0000|0000|0000|0000|    |   |    |0000|0000|0000|0000|   |    |0000|0000|0000|0000|   |0000|0000|0000|0000|    |
* |0000|0000|0000|0000|    |   |    |0000|0000|0000|0000|   |    |0000|0000|0000|0000|   |0000|0000|0000|0000|    |
* ==========================   ==========================   ==========================   ==========================
* |0000|0000|x   |0000|    |   |    |0000|   x|0000|0000|   |    |0000|    |0000|0000|   |0000|0000|    |0000|    |
* |0000|0000|    |0000|    |   |    |0000|    |0000|0000|   |    |0000|   x|0000|0000|   |0000|0000|x   |0000|    |
* ==========================   ==========================   ==========================   ==========================
* |0000|0000|0000|0000|    |   |    |0000|0000|0000|0000|   |    |0000|0000|0000|0000|   |0000|0000|0000|0000|    |
* |0000|0000|0000|0000|    |   |    |0000|0000|0000|0000|   |    |0000|0000|0000|0000|   |0000|0000|0000|0000|    |
* ==========================   ==========================   ==========================   ==========================
* |    |    |    |    |    |   |    |    |    |    |    |   |    |0000|0000|0000|0000|   |0000|0000|0000|0000|    |
* |    |    |    |    |    |   |    |    |    |    |    |   |    |0000|0000|0000|0000|   |0000|0000|0000|0000|    |
* ==========================   ==========================   ==========================   ==========================
*
*   ===> To make sure we cover all cases, we need a 5,5 neighbourhood centered on 2,2
*
*
* @bsimethod                                                  Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/



template<uint32_t ChannelCount_T, typename Data_T>
template<typename Surface_T> 
ImagePPStatus HRAImageBicubicSamplerN8<ChannelCount_T, Data_T>::Stretch_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
    {
    size_t inPitch, outPitch;
    Byte const* pInBuffer = inData.GetDataCP(inPitch);
    Byte* const pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t inWidth = inData.GetWidth();
    uint32_t inHeight = inData.GetHeight();

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();

    //get offset array first
    float* pPixelsDeltas = GetRealPixelOffSetBuffer(outWidth);
    RawBuffer_T<uint32_t, decltype(m_allocator)> offsets(outWidth, m_allocator);
    uint32_t* pPixelsOffsetInt = &offsets.At(0);

    for(uint32_t w=0; w < outWidth; ++w)
        {
        pPixelsDeltas[w] = (float)(m_offsetX + ((w + outOffset.x + 0.5) * m_scaleX) - inOffset.x - 0.5);
        pPixelsOffsetInt[w] = (int32_t)floor(pPixelsDeltas[w]);
        pPixelsDeltas[w] -= (int32_t)pPixelsOffsetInt[w]; //Precalculation of Dx
        }

    auto lineProcessor = [&](uint32_t row)
        {
        float rowInSrc = (float)(m_offsetY + ((row + outOffset.y + 0.5) * m_scaleY) - inOffset.y - 0.5);
        int32_t rowInSrcInt = (int32_t)floor(rowInSrc);

        Data_T* pOutBufferLine = (Data_T*)(pOutBuffer + row * outPitch);

        // Bound reference point in case we are way outside the src area. We use the bounded value to compute our neighborhood.
        int32_t py_ref = BOUND(rowInSrcInt, -1, (int)inHeight);
        uint32_t pyData[4];            
        pyData[1] = BOUND(py_ref,     0, (int)inHeight-1);
        pyData[0] = BOUND(py_ref - 1, 0, (int)inHeight-1);
        pyData[2] = BOUND(py_ref + 1, 0, (int)inHeight-1);
        pyData[3] = BOUND(py_ref + 2, 0, (int)inHeight-1);

        Data_T const* pInBufferFirstLine  =  (Data_T const*)(pInBuffer + pyData[0] * inPitch);
        Data_T const* pInBufferSecondLine  = (Data_T const*)(pInBuffer + pyData[1] * inPitch);
        Data_T const* pInBufferThirdLine  =  (Data_T const*)(pInBuffer + pyData[2] * inPitch);
        Data_T const* pInBufferFourthLine  = (Data_T const*)(pInBuffer + pyData[3] * inPitch);

        float Dy = rowInSrc - rowInSrcInt;
        float T2y = Dy * Dy;
        float T3y = Dy * T2y;
        float CoefY0 = T3y * s_a - T2y * s_2a + s_a * Dy;
        float CoefY1 = T3y * s_aP2 - T2y * s_aP3 + 1;
        float CoefY2 = T2y * s_2aP3 - T3y * s_aP2 - s_a * Dy;
        float CoefY3 = s_a * (T2y - T3y);

        for(uint32_t column=0; column < outWidth; ++column)
            {
            // Bound reference point in case we are way outside the src area. We use the bounded value to compute our neighborhood.
            int32_t px_ref = BOUND((int32_t)pPixelsOffsetInt[column], -1, (int)inWidth);
            uint32_t pxData[4];
            pxData[1] = BOUND(px_ref,     0, (int)inWidth-1);
            pxData[0] = BOUND(px_ref - 1, 0, (int)inWidth-1);
            pxData[2] = BOUND(px_ref + 1, 0, (int)inWidth-1);
            pxData[3] = BOUND(px_ref + 2, 0, (int)inWidth-1);

            float Dx = pPixelsDeltas[column];
            float T2x = Dx * Dx;
            float T3x = Dx * T2x;
            float CoefX0 = T3x * s_a - T2x * s_2a + s_a * Dx;
            float CoefX1 = T3x * s_aP2 - T2x * s_aP3 + 1;
            float CoefX2 = T2x * s_2aP3 - T3x * s_aP2 - s_a * Dx;
            float CoefX3 = s_a * (T2x - T3x);

            //for each channels
            for (uint32_t channel = 0; channel < ChannelCount_T; ++channel)
                {
                float Horiz0 = pInBufferFirstLine[pxData[0]*ChannelCount_T + channel] * CoefX0 +
                               pInBufferFirstLine[pxData[1]*ChannelCount_T + channel] * CoefX1 +
                               pInBufferFirstLine[pxData[2]*ChannelCount_T + channel] * CoefX2 +
                               pInBufferFirstLine[pxData[3]*ChannelCount_T + channel] * CoefX3;

                float Horiz1 = pInBufferSecondLine[pxData[0]*ChannelCount_T + channel] * CoefX0 +
                               pInBufferSecondLine[pxData[1]*ChannelCount_T + channel] * CoefX1 +
                               pInBufferSecondLine[pxData[2]*ChannelCount_T + channel] * CoefX2 +
                               pInBufferSecondLine[pxData[3]*ChannelCount_T + channel] * CoefX3;

                float Horiz2 = pInBufferThirdLine[pxData[0]*ChannelCount_T + channel] * CoefX0 +
                               pInBufferThirdLine[pxData[1]*ChannelCount_T + channel] * CoefX1 +
                               pInBufferThirdLine[pxData[2]*ChannelCount_T + channel] * CoefX2 +
                               pInBufferThirdLine[pxData[3]*ChannelCount_T + channel] * CoefX3;

                float Horiz3 = pInBufferFourthLine[pxData[0]*ChannelCount_T + channel] * CoefX0 +
                               pInBufferFourthLine[pxData[1]*ChannelCount_T + channel] * CoefX1 +
                               pInBufferFourthLine[pxData[2]*ChannelCount_T + channel] * CoefX2 +
                               pInBufferFourthLine[pxData[3]*ChannelCount_T + channel] * CoefX3;

                float channelResult = Horiz0 * CoefY0 + Horiz1 * CoefY1 + Horiz2 * CoefY2 + Horiz3 * CoefY3;

                pOutBufferLine[(column * ChannelCount_T) + channel] = clip_cast<Data_T>(channelResult);
                }
            }
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
* @bsimethod                                    Stephane.Poulin                 12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
template<class Surface_T>
ImagePPStatus HRAImageBicubicSamplerN8<ChannelCount_T, Data_T>::Warp_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
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
* @bsimethod                                                    Nicolas.Marquis 06/2014
 +---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
template<typename Surface_T, class Allocator_T>
ImagePPStatus HRAImageBicubicSamplerN8<ChannelCount_T, Data_T>::Warp_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset, Allocator_T& allocator)
    {
    // Refer to Note 1 above for explanations
    size_t inPitch, outPitch;
    Byte const* pInBuffer = inData.GetDataCP(inPitch);
    Byte* const pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t inWidth = inData.GetWidth();
    uint32_t inHeight = inData.GetHeight();

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();

    auto lineProcessor = [&](uint32_t row)
        {
        RawBuffer_T<double, Allocator_T> positions(outWidth * 2, allocator);
        double* pXPositions = &positions.At(0);
        double* pYPositions = &positions.At(outWidth);

        // convert the destination run into the source coordinates
        m_pDestToSrcTransfo->ConvertDirect(row + outOffset.y + 0.5, outOffset.x + 0.5, outWidth, 1.0, pXPositions, pYPositions);

        Data_T* pOutBufferLine = (Data_T*)(pOutBuffer + row * outPitch);

        for(uint32_t column=0; column < outWidth; ++column)
            {
            // The -0.5 is to select a window of the 4x4 closest pixels.
            float x  = (float)((pXPositions[column] - inOffset.x) - 0.5);
            float y  = (float)((pYPositions[column] - inOffset.y) - 0.5);

            int px = (int)floor(x);
            int py = (int)floor(y);

            // Bound reference point in case we are way outside the src area. We use the bounded value to compute our neighborhood.
            int px_ref = BOUND(px, -1, (int)inWidth);   // Ref point can be outside by 1.
            uint32_t pxData[4];
            pxData[1] = BOUND(px_ref,     0, (int)inWidth-1);
            pxData[0] = BOUND(px_ref - 1, 0, (int)inWidth-1);
            pxData[2] = BOUND(px_ref + 1, 0, (int)inWidth-1);
            pxData[3] = BOUND(px_ref + 2, 0, (int)inWidth-1);

            // Bound reference point in case we are way outside the src area. We use the bounded value to compute our neighborhood.
            int py_ref = BOUND(py, -1, (int)inHeight);  // Ref point can be outside by 1.
            uint32_t pyData[4];            
            pyData[1] = BOUND(py_ref,     0, (int)inHeight-1);
            pyData[0] = BOUND(py_ref - 1, 0, (int)inHeight-1);
            pyData[2] = BOUND(py_ref + 1, 0, (int)inHeight-1);
            pyData[3] = BOUND(py_ref + 2, 0, (int)inHeight-1);

            float Dy = y - py;
            float T2y = Dy * Dy;
            float T3y = Dy * T2y;
            float CoefY0 = T3y * s_a - T2y * s_2a + s_a * Dy;
            float CoefY1 = T3y * s_aP2 - T2y * s_aP3 + 1;
            float CoefY2 = T2y * s_2aP3 - T3y * s_aP2 - s_a * Dy;
            float CoefY3 = s_a * (T2y - T3y);

            float Dx = x - px;
            float T2x = Dx * Dx;
            float T3x = Dx * T2x;
            float CoefX0 = T3x * s_a - T2x * s_2a + s_a * Dx;
            float CoefX1 = T3x * s_aP2 - T2x * s_aP3 + 1;
            float CoefX2 = T2x * s_2aP3 - T3x * s_aP2 - s_a * Dx;
            float CoefX3 = s_a * (T2x - T3x);
            
            Data_T const* pInBufferFirstLine  =  (Data_T const*)(pInBuffer + pyData[0] * inPitch);
            Data_T const* pInBufferSecondLine  = (Data_T const*)(pInBuffer + pyData[1] * inPitch);
            Data_T const* pInBufferThirdLine  =  (Data_T const*)(pInBuffer + pyData[2] * inPitch);
            Data_T const* pInBufferFourthLine  = (Data_T const*)(pInBuffer + pyData[3] * inPitch);

            for (uint32_t channel = 0; channel < ChannelCount_T; ++channel)
                {
                float Horiz0 = pInBufferFirstLine[pxData[0]*ChannelCount_T + channel] * CoefX0 +
                               pInBufferFirstLine[pxData[1]*ChannelCount_T + channel] * CoefX1 +
                               pInBufferFirstLine[pxData[2]*ChannelCount_T + channel] * CoefX2 +
                               pInBufferFirstLine[pxData[3]*ChannelCount_T + channel] * CoefX3;

                float Horiz1 = pInBufferSecondLine[pxData[0]*ChannelCount_T + channel] * CoefX0 +
                               pInBufferSecondLine[pxData[1]*ChannelCount_T + channel] * CoefX1 +
                               pInBufferSecondLine[pxData[2]*ChannelCount_T + channel] * CoefX2 +
                               pInBufferSecondLine[pxData[3]*ChannelCount_T + channel] * CoefX3;

                float Horiz2 = pInBufferThirdLine[pxData[0]*ChannelCount_T + channel] * CoefX0 +
                               pInBufferThirdLine[pxData[1]*ChannelCount_T + channel] * CoefX1 +
                               pInBufferThirdLine[pxData[2]*ChannelCount_T + channel] * CoefX2 +
                               pInBufferThirdLine[pxData[3]*ChannelCount_T + channel] * CoefX3;

                float Horiz3 = pInBufferFourthLine[pxData[0]*ChannelCount_T + channel] * CoefX0 +
                               pInBufferFourthLine[pxData[1]*ChannelCount_T + channel] * CoefX1 +
                               pInBufferFourthLine[pxData[2]*ChannelCount_T + channel] * CoefX2 +
                               pInBufferFourthLine[pxData[3]*ChannelCount_T + channel] * CoefX3;

                float channelResult = Horiz0 * CoefY0 + Horiz1 * CoefY1 + Horiz2 * CoefY2 + Horiz3 * CoefY3;

                pOutBufferLine[(column * ChannelCount_T) + channel] = clip_cast<Data_T>(channelResult);
                }
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

