//:>--------------------------------------------------------------------------------------+
    //:>
    //:>     $Source: all/gra/hra/src/HRAImageBilinearSamplerN8.cpp $
    //:>
    //:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
    //:>
    //:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <ImagePPInternal/gra/HRAImageBilinearSamplerN8.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <ImagePPInternal/gra/ImageCommon.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/HGF2DCoordSys.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <ImagePPInternal/gra/HRAImageSurface.h>



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSampler* HRAImageSampler::CreateBilinear(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType)
    {
    unique_ptr<HRAImageSampler> pSampler;

    switch (pixelType.GetClassID())
        {
        case HRPPixelTypeId_V8Gray8:
        case HRPPixelTypeId_V8GrayWhite8:
            pSampler.reset(new HRAImageBilinearSamplerN8<1, uint8_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V16Gray16:
            pSampler.reset(new HRAImageBilinearSamplerN8<1, uint16_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V16Int16:
            pSampler.reset(new HRAImageBilinearSamplerN8<1, int16_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V32Float32:
            pSampler.reset(new HRAImageBilinearSamplerN8<1, float>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V16PRGray8A8:
            pSampler.reset(new HRAImageBilinearSamplerN8<2, uint8_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V24R8G8B8:
        case HRPPixelTypeId_V24B8G8R8:
        case HRPPixelTypeId_V24PhotoYCC:
            pSampler.reset(new HRAImageBilinearSamplerN8<3, uint8_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V48R16G16B16:
            pSampler.reset(new HRAImageBilinearSamplerN8<3, uint16_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V96R32G32B32:
            pSampler.reset(new HRAImageBilinearSamplerN8<3, uint32_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V32PRPhotoYCCA8:
        case HRPPixelTypeId_V32PR8PG8PB8A8:
        case HRPPixelTypeId_V32CMYK:
        case HRPPixelTypeId_V32B8G8R8X8:
        case HRPPixelTypeId_V32R8G8B8A8:
        case HRPPixelTypeId_V32R8G8B8X8:
        case HRPPixelTypeId_V32A8R8G8B8:
            pSampler.reset(new HRAImageBilinearSamplerN8<4, uint8_t>(destToSrcTransfo));
            break;

        case HRPPixelTypeId_V64R16G16B16A16:
        case HRPPixelTypeId_V64R16G16B16X16:
            pSampler.reset(new HRAImageBilinearSamplerN8<4, uint16_t>(destToSrcTransfo));
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
HRAImageBilinearSamplerN8<ChannelCount_T, Data_T>::HRAImageBilinearSamplerN8(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo)
 :HRAImageSampler(HRPPixelNeighbourhood(3,3,1,1), destToSrcTransfo)
    { 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
HRAImageBilinearSamplerN8<ChannelCount_T, Data_T>::~HRAImageBilinearSamplerN8(){}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
float* HRAImageBilinearSamplerN8<ChannelCount_T, Data_T>::GetRealPixelOffSetBuffer(size_t size)
    {
    if (size > m_realPixelOffSet.size())
        m_realPixelOffSet.resize(size);

    return m_realPixelOffSet.data();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
ImagePPStatus HRAImageBilinearSamplerN8<ChannelCount_T, Data_T>::_Stretch(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() == inData.GetPixelType().CountPixelRawDataBits());

    SamplerSurfaceVisitor<HRAImageBilinearSamplerN8, true/*IsStretch*/> visitor(*this, outData, outOffset, inOffset);

    return inData.Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
ImagePPStatus HRAImageBilinearSamplerN8<ChannelCount_T, Data_T>::_Warp(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() == inData.GetPixelType().CountPixelRawDataBits());

    SamplerSurfaceVisitor<HRAImageBilinearSamplerN8, false/*IsStretch*/> visitor(*this, outData, outOffset, inOffset);

    return inData.Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
template<typename Surface_T>
void HRAImageBilinearSamplerN8<ChannelCount_T, Data_T>::Stretch_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
    {
    if(outData.GetPixelType().GetChannelOrg().GetChannelPtr(0)->GetNoDataValue() != NULL)
        Stretch_T<Surface_T, true/*AsNoData*/>(outData, outOffset, inData, inOffset);
    else
        Stretch_T<Surface_T, false/*AsNoData*/>(outData, outOffset, inData, inOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
template<typename Surface_T>
void HRAImageBilinearSamplerN8<ChannelCount_T, Data_T>::Warp_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
    {
    if(outData.GetPixelType().GetChannelOrg().GetChannelPtr(0)->GetNoDataValue() != NULL)
        {
        if (!m_pDestToSrcTransfo->IsConvertDirectThreadSafe())
            {
            return Warp_T<Surface_T, true/*AsNoData*/, SingleBlockAllocator>(outData, outOffset, inData, inOffset, m_allocator);
            }
        else
            {
            ConcurrencyAllocator allocator;
            return Warp_T<Surface_T, true/*AsNoData*/, ConcurrencyAllocator>(outData, outOffset, inData, inOffset, allocator);
            }
        }
    else
        {
        if (!m_pDestToSrcTransfo->IsConvertDirectThreadSafe())
            {
            return Warp_T<Surface_T, false/*AsNoData*/, SingleBlockAllocator>(outData, outOffset, inData, inOffset, m_allocator);
            }
        else
            {
            ConcurrencyAllocator allocator;
            return Warp_T<Surface_T, false/*AsNoData*/, ConcurrencyAllocator>(outData, outOffset, inData, inOffset, allocator);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Nicolas.Marquis 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
template<typename Surface_T, bool HasNoData_T>
void HRAImageBilinearSamplerN8<ChannelCount_T, Data_T>::Stretch_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
    {
    //&&Backlog We need to implement a proper bilinear when the scale factor is higher then 1:2.  
    //          Todo: use average to compute the 4 pixels value to the closest 1:N and then bilinear the remainder. 
    //          also bicubic.
    size_t inPitch, outPitch;
    Byte const* pInBuffer = inData.GetDataCP(inPitch);
    Byte* const pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t inWidth = inData.GetWidth();
    uint32_t inHeight = inData.GetHeight();

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();

    Data_T noDataValue;
    if (HasNoData_T)
        noDataValue = (Data_T)*outData.GetPixelType().GetChannelOrg().GetChannelPtr(0)->GetNoDataValue();

    float* pPixelsfx = GetRealPixelOffSetBuffer(outWidth);
   
    RawBuffer_T<int32_t, decltype(m_allocator)> offsets(outWidth, m_allocator);
    int32_t* pPixelsOffsetInt = &offsets.At(0);

    for (uint32_t w = 0; w < outWidth; ++w)
        {
        // The -0.5 is to select a window of the 4 closest pixels.
        float pixelOffset = (float)((m_offsetX + ((w + outOffset.x + 0.5) * m_scaleX) - inOffset.x) - 0.5);
        pPixelsOffsetInt[w] = (int)floor(pixelOffset);

        pPixelsfx[w] = pixelOffset - pPixelsOffsetInt[w];
        }

    // Define a lambda to process a line.
    // Can be called by several threads.
    auto lineProcessor = [&](uint32_t row)
        {
        // The -0.5 is to select a window of the 4 closest pixels.
        float rowInSrcRaw = (float)((m_offsetY + ((row + outOffset.y + 0.5) * m_scaleY) - inOffset.y) - 0.5);

        int rowInSrcInt = (int)floor(rowInSrcRaw);

        Data_T* pOutBufferLine = (Data_T*)(pOutBuffer + row * outPitch);

        Data_T const* pInBufferLine1 = NULL;
        Data_T const* pInBufferLine2 = NULL;
        if (rowInSrcInt < 0)
            {
            pInBufferLine1 = pInBufferLine2 = (Data_T const*)pInBuffer;
            }
        else
            {
            pInBufferLine1 = (Data_T const*)(pInBuffer + MIN(rowInSrcInt, (int)inHeight - 1) * inPitch);
            pInBufferLine2 = (Data_T const*)(pInBuffer + MIN(rowInSrcInt + 1, (int)inHeight - 1) * inPitch);
            }

        float fy = rowInSrcRaw - rowInSrcInt;
        float fy1 = 1.0f - fy;

        for (uint32_t column = 0; column < outWidth; ++column)
            {
            uint32_t pxData, pxData2;
            if (pPixelsOffsetInt[column] < 0)
                {
                pxData = pxData2 = 0;
                }
            else
                {
                pxData = MIN(pPixelsOffsetInt[column], (int)inWidth - 1);
                pxData2 = MIN(pxData + 1, inWidth - 1);
                }

            float fx = pPixelsfx[column];
            float fx1 = 1.0f - pPixelsfx[column];

            // pixels weights  [ 1, 2 ]
            //                 [ 3, 4 ]
            float w1 = fx1 * fy1;
            float w2 = fx * fy1;
            float w3 = fx1 * fy;
            float w4 = fx * fy;

            for (uint32_t channel = 0; channel < ChannelCount_T; ++channel)
                {
                // *** HasNoData_T is resolved at compile time.
                if (HasNoData_T &&
                    (pInBufferLine1[pxData*ChannelCount_T + channel] == noDataValue ||
                    pInBufferLine1[pxData2*ChannelCount_T + channel] == noDataValue ||
                    pInBufferLine2[pxData*ChannelCount_T + channel] == noDataValue ||
                    pInBufferLine2[pxData2*ChannelCount_T + channel] == noDataValue))
                    {
                    // For now we simply average the valid pixels, if none, the output is set to noDataValue.
                    float sum = 0;
                    uint32_t divisor = 0;

                    if (pInBufferLine1[pxData*ChannelCount_T + channel] != noDataValue)
                        {
                        sum += pInBufferLine1[pxData*ChannelCount_T + channel];
                        ++divisor;
                        }

                    if (pInBufferLine1[pxData2*ChannelCount_T + channel] != noDataValue)
                        {
                        sum += pInBufferLine1[pxData2*ChannelCount_T + channel];
                        ++divisor;
                        }

                    if (pInBufferLine2[pxData*ChannelCount_T + channel] != noDataValue)
                        {
                        sum += pInBufferLine2[pxData*ChannelCount_T + channel];
                        ++divisor;
                        }

                    if (pInBufferLine2[pxData2*ChannelCount_T + channel] != noDataValue)
                        {
                        sum += pInBufferLine2[pxData2*ChannelCount_T + channel];
                        ++divisor;
                        }

                    if (divisor)
                        pOutBufferLine[column * ChannelCount_T + channel] = static_cast<Data_T>(sum / divisor);
                    else
                        pOutBufferLine[column * ChannelCount_T + channel] = noDataValue;
                    }
                else
                    {
                    float result = pInBufferLine1[pxData*ChannelCount_T + channel] * w1 + pInBufferLine1[pxData2*ChannelCount_T + channel] * w2 +
                        pInBufferLine2[pxData*ChannelCount_T + channel] * w3 + pInBufferLine2[pxData2*ChannelCount_T + channel] * w4;

                    //It's faster without clip_cast and since the weights have a total of 1.0. it should not be required to clip_cast<Data_T>(result)
                    // unless we have precision error?!
                    pOutBufferLine[column * ChannelCount_T + channel] = static_cast<Data_T>(result);
                    BeAssert(pOutBufferLine[column * ChannelCount_T + channel] == clip_cast<Data_T>(result));
                    }
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nicolas.Marquis 06/2014
 +---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t ChannelCount_T, typename Data_T>
template<typename Surface_T, bool HasNoData_T, class Allocator_T>
void HRAImageBilinearSamplerN8<ChannelCount_T, Data_T>::Warp_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset, Allocator_T& allocator)
    {
    size_t inPitch, outPitch;
    Byte const* pInBuffer = inData.GetDataCP(inPitch);
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t inWidth = inData.GetWidth();
    uint32_t inHeight = inData.GetHeight();

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();

    Data_T noDataValue;
    if (HasNoData_T)
        noDataValue = (Data_T)*outData.GetPixelType().GetChannelOrg().GetChannelPtr(0)->GetNoDataValue();

    // Define a lambda to process a line.
    // Can be called by several threads.
    auto lineProcessor = [&](uint32_t row)
        {
        RawBuffer_T<double, Allocator_T> positions(outWidth * 2, allocator);
        double* pXPositions = &positions.At(0);
        double* pYPositions = &positions.At(outWidth);

        // convert the destination run into the source coordinates
        m_pDestToSrcTransfo->ConvertDirect(row + outOffset.y + 0.5, outOffset.x + 0.5, outWidth, 1.0, pXPositions, pYPositions);

        Data_T* pOutBufferLine = (Data_T*)(pOutBuffer + row * outPitch);

        for (uint32_t column = 0; column < outWidth; ++column)
            {
            int pxData, pyData;
            int pxData2, pyData2;
            int px, py;

            // The -0.5 is to select a window of the 4 closest pixels.
            float x = (float)((pXPositions[column] - inOffset.x) - 0.5);
            float y = (float)((pYPositions[column] - inOffset.y) - 0.5);

            px = (int)floor(x);
            py = (int)floor(y);

            if (px < 0)
                {
                pxData = pxData2 = 0;
                }
            else
                {
                pxData = MIN(px, (int)inWidth - 1);
                pxData2 = MIN(pxData + 1, (int)inWidth - 1);
                }

            if (py < 0)
                {
                pyData = pyData2 = 0;
                }
            else
                {
                pyData = MIN(py, (int)inHeight - 1);
                pyData2 = MIN(pyData + 1, (int)inHeight - 1);
                }

            float fx = x - px;
            float fy = y - py;
            float fx1 = 1.0f - fx;
            float fy1 = 1.0f - fy;

            float w1 = fx1 *  fy1;
            float w2 = fx *  fy1;
            float w3 = fx1 *  fy;
            float w4 = fx *  fy;

            //for each channels
            for (uint32_t channel = 0; channel < ChannelCount_T; ++channel)
                {
                Data_T p1 = *((Data_T const*)(pInBuffer + pyData * inPitch) + pxData*ChannelCount_T + channel);
                Data_T p2 = *((Data_T const*)(pInBuffer + pyData * inPitch) + pxData2*ChannelCount_T + channel);
                Data_T p3 = *((Data_T const*)(pInBuffer + pyData2 * inPitch) + pxData*ChannelCount_T + channel);
                Data_T p4 = *((Data_T const*)(pInBuffer + pyData2 * inPitch) + pxData2*ChannelCount_T + channel);

                // *** HasNoData_T is resolved at compile time.
                if (HasNoData_T &&
                    (p1 == noDataValue || p2 == noDataValue || p3 == noDataValue || p4 == noDataValue))
                    {
                    // For now we simply average the valid pixels, if none, the output is set to noDataValue.
                    float sum = 0;
                    uint32_t divisor = 0;

                    if (p1 != noDataValue)
                        {
                        sum += p1;
                        ++divisor;
                        }

                    if (p2 != noDataValue)
                        {
                        sum += p2;
                        ++divisor;
                        }

                    if (p3 != noDataValue)
                        {
                        sum += p3;
                        ++divisor;
                        }

                    if (p4 != noDataValue)
                        {
                        sum += p4;
                        ++divisor;
                        }

                    if (divisor)
                        pOutBufferLine[column * ChannelCount_T + channel] = static_cast<Data_T>(sum / divisor);
                    else
                        pOutBufferLine[column * ChannelCount_T + channel] = noDataValue;
                    }
                else
                    {
                    float result = p1*w1 + p2 * w2 + p3 * w3 + p4 * w4; result;

                    //It's faster without clip_cast and since the weights have a total of 1.0. it should not be required to clip_cast<Data_T>(result)
                    // unless we have precision error?!
                    pOutBufferLine[column * ChannelCount_T + channel] = static_cast<Data_T>(result);
                    BeAssert(pOutBufferLine[column * ChannelCount_T + channel] == clip_cast<Data_T>(result));
                    }
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
    }




