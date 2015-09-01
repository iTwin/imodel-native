/*--------------------------------------------------------------------------------------+
|
|     $Source: all/gra/hra/src/ImageCommon.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ImagePPInternal/hstdcpp.h>
#include <ImagePPInternal/gra/ImageCommon.h>




#ifndef NDEBUG
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void debugOnly_ValidateCopyPixels(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch,
                                uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch,
                                size_t pixelSize, PixelOffset64 const& offset)
    {
    HDEBUGCODE(bool __asserted = false;)

    for (uint32_t row = 0; row < outHeight; ++row)
        {
        uint32_t effectiveRowInSrc = MAX(0, MIN((int32_t)(row + offset.y), (int32_t)inHeight - 1));

        Byte*       pOutBufferLine = pOutData + row*outPitch;
        Byte const* pInBufferLine = pInData + (effectiveRowInSrc*inPitch);

        for (uint32_t column = 0; column < outWidth; ++column)
            {
            uint32_t effectiveColumnInSrc = MAX(0, MIN((int32_t)(column + offset.x), (int32_t)inWidth - 1));

            HDEBUGCODE
                (
                if (!__asserted && 0 != memcmp(pOutBufferLine + column*pixelSize, pInBufferLine + effectiveColumnInSrc*pixelSize, pixelSize))
                    {
                    BeAssert(!"HRAImageSampler, CopyPixels_TEST failed");
                    __asserted = true; // assert once per loop.
                    }
                );
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  1/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Rle1Manip::ValidateLineIntegrity(uint16_t const* pLine, uint32_t width, size_t dataSize)
    {
    // Must end on black state, odd size, 1, 3, 5...
    if(!(dataSize / 2) & 0x1)
        return false;

    uint32_t pixelCount = pLine[0];
    uint32_t index=1;
    while(pixelCount < width)
        {
        // Detect zero len run that should not exits.
        if(pLine[index] == 0 && pLine[index-1] != 32767)
            {
            //WString result;
            //result.Sprintf(L"Inward zero len run at line %i", line);
            return false;
            }

        pixelCount +=  pLine[index];
        ++index;
        }

    if(pixelCount != width)
        {
        //result.Sprintf(L"Invalid pixel count at line %i", line);
        return false;
        }    

    // If the next run is black, it must have a len of 0.
    if(!(index & 0x1))
        {
        if(pLine[index] != 0)                
            {
            //result.Sprintf(L"End of line(%i) is not on black state", line);
            return false;
            }

        ++index;    // skip empty black.
        }

    if(index*2 != dataSize)
        {
        //result.Sprintf(L"Invalid line data size at line %i", line);
        return false;
        }       

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus ImagePP::CopyPixelsN8(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch,
                           uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch,
                           uint32_t pixelSizeBytes, PixelOffset64 const& offset)
    {
    switch (pixelSizeBytes)
        {
        case 1:
            CopyPixelsN8_T<1>(outWidth, outHeight, pOutData, outPitch, inWidth, inHeight, pInData, inPitch, offset);
            break;
        case 2:
            CopyPixelsN8_T<2>(outWidth, outHeight, pOutData, outPitch, inWidth, inHeight, pInData, inPitch, offset);
            break;
        case 3:
            CopyPixelsN8_T<3>(outWidth, outHeight, pOutData, outPitch, inWidth, inHeight, pInData, inPitch, offset);
            break;
        case 4:
            CopyPixelsN8_T<4>(outWidth, outHeight, pOutData, outPitch, inWidth, inHeight, pInData, inPitch, offset);
            break;
        case 6:
            CopyPixelsN8_T<6>(outWidth, outHeight, pOutData, outPitch, inWidth, inHeight, pInData, inPitch, offset);
            break;
        case 8:
            CopyPixelsN8_T<8>(outWidth, outHeight, pOutData, outPitch, inWidth, inHeight, pInData, inPitch, offset);
            break;
        case 12:
            CopyPixelsN8_T<12>(outWidth, outHeight, pOutData, outPitch, inWidth, inHeight, pInData, inPitch, offset);
            break;
        default:
            BeAssert(!"CopyPixels unsupported bit depth.");
            return IMAGEPP_STATUS_NoImplementation;
        }
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<uint32_t InOffset_T> void CopyBits_T(uint32_t outWidth, Byte* pOutData, Byte const* pInData)
    {
    uint32_t fullBytesCount = outWidth / BITSPERPIXEL_N1;
    uint64_t x = 0;
    for (; x < fullBytesCount; ++x) // dst
        {
        pOutData[x] = (0xFF & (pInData[x] << InOffset_T)) | (pInData[x + 1] >> (BITSPERPIXEL_N1 - InOffset_T));
        }

    uint32_t remainder = outWidth % BITSPERPIXEL_N1; // bits to write

    if (remainder > 0)
        {
        // fill last Byte
        if (remainder > (BITSPERPIXEL_N1 - InOffset_T))
            {
            const uint32_t bitsFromFirstByte = BITSPERPIXEL_N1 - InOffset_T;
            const uint32_t bitsFromSecondByte = remainder - bitsFromFirstByte;
            // Copy from 2 Bytes
            pOutData[x] = (0xFF & (pInData[x] << InOffset_T)) | ((pInData[x + 1] & N1Manip::GetLeftBitPattern(bitsFromSecondByte)) >> bitsFromFirstByte) | (pOutData[x] & N1Manip::GetRightBitPattern(BITSPERPIXEL_N1 - remainder));
            }
        else
            {
            // Copy from first Bytes
            pOutData[x] = (pInData[x] << InOffset_T) & N1Manip::GetLeftBitPattern(remainder) | (pOutData[x] & N1Manip::GetRightBitPattern(BITSPERPIXEL_N1 - remainder));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Template specialization for offset==0
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<> void CopyBits_T<0>(uint32_t outWidth, Byte* pOutData, Byte const* pInData)
    {
    uint32_t fullBytesCount = outWidth / BITSPERPIXEL_N1;
    memcpy(pOutData, pInData, fullBytesCount);
    uint64_t x = fullBytesCount;

    uint32_t remainder = outWidth % BITSPERPIXEL_N1; // bits to write

    if (remainder > 0)
        {
        // fill last Byte
        pOutData[x] = pInData[x] & N1Manip::GetLeftBitPattern(remainder) | (pOutData[x] & N1Manip::GetRightBitPattern(BITSPERPIXEL_N1 - remainder));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ImagePP::CopyLineN1(Byte* pOutData, uint32_t posX, uint32_t pixelCount, Byte const* pInData)
    {
    uint32_t offset = posX % BITSPERPIXEL_N1;
    Byte const* pSrcStart = pInData + (posX / BITSPERPIXEL_N1);

    switch (offset)
        {
        case 0:
            CopyBits_T<0>(pixelCount, pOutData, pSrcStart);
            break;
        case 1:
            CopyBits_T<1>(pixelCount, pOutData, pSrcStart);
            break;
        case 2:
            CopyBits_T<2>(pixelCount, pOutData, pSrcStart);
            break;
        case 3:
            CopyBits_T<3>(pixelCount, pOutData, pSrcStart);
            break;
        case 4:
            CopyBits_T<4>(pixelCount, pOutData, pSrcStart);
            break;
        case 5:
            CopyBits_T<5>(pixelCount, pOutData, pSrcStart);
            break;
        case 6:
            CopyBits_T<6>(pixelCount, pOutData, pSrcStart);
            break;
        case 7:
            CopyBits_T<7>(pixelCount, pOutData, pSrcStart);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ImagePP::CopyPixelsN1(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch,
                  uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch,
                  PixelOffset64 const& offset)
    {
    // Source coords
    uint32_t srcOffsetX = (offset.x < 0) ? 0 : (uint32_t)offset.x;
    uint32_t srcOffsetY = (offset.y < 0) ? 0 : (uint32_t)offset.y;

    // Origin\end of the src in dst coords
    // > if we were receiving offset in double. Kept as reference.
    //     Int64 inBeginX = (offset.x < 0) ? (Int64)(-(floor(offset.x))) : (Int64)(-offset.x);
    //     Int64 inBeginY = (offset.y < 0) ? (Int64)(-(floor(offset.y))) : (Int64)(-offset.y);
    int64_t inBeginX = -offset.x;
    int64_t inBeginY = -offset.y;
    int64_t inEndX = inBeginX + inWidth;
    int64_t inEndY = inBeginY + inHeight;

    int32_t srcRight = inWidth - 1;
    int32_t srcBottom = inHeight - 1;

    // Intersection part
    uint32_t beginX = (uint32_t)MIN(MAX(inBeginX, 0), outWidth);
    uint32_t beginY = (uint32_t)MIN(MAX(inBeginY, 0), outHeight);
    uint32_t endX = (uint32_t)MAX(0, MIN(inEndX, outWidth));
    uint32_t endY = (uint32_t)MAX(0, MIN(inEndY, outHeight));

    uint32_t y = 0;

    // Top, (offset.y < 0)
    // Build first line
    if (y < beginY)
        {
        Byte*       pOutLine = pOutData + y*outPitch;
        Byte const* pInLine = pInData; // First line
        uint32_t x = 0;

        // Left, copy first pixel
        N1Manip::SetBitsRange(pOutLine, 0/*startPos*/, beginX/*endPos*/, N1Manip::GetBitValue(pInLine, 0));
        x = beginX;

        // Center
        N1Manip::CopyBits(pOutLine, beginX/*outPos*/, (endX - x)/*pixelCount*/, pInLine, srcOffsetX/*inPos*/);
        x = endX;

        // Right, copy last pixel
        N1Manip::SetBitsRange(pOutLine, x/*startPos*/, outWidth/*endPos*/, N1Manip::GetBitValue(pInLine, srcRight)/*state*/);

        ++y;
        }

    // recopy first line into subsequent lines.
    for (; y < beginY; ++y)
        {
        Byte*       pOutLine = pOutData + y*outPitch;
        Byte const* pPrevOutLine = pOutData + (y - 1)*outPitch;
        CopyLineN1(pOutLine, 0, outWidth, pPrevOutLine);
        }

    // Mid (Intersecting part)
    uint64_t srcY = srcOffsetY;
    for (; y < endY; ++y, ++srcY)
        {
        Byte* pOutLine = pOutData + y*outPitch;
        Byte const* pInLine = pInData + srcY*inPitch;
        uint32_t x = 0;

        // Left, copy first pixel
        N1Manip::SetBitsRange(pOutLine, 0/*startPos*/, beginX/*endPos*/, N1Manip::GetBitValue(pInLine, 0));
        x = beginX;

        // Center
        N1Manip::CopyBits(pOutLine, x/*outPos*/, (endX - x)/*pixelCount*/, pInLine, srcOffsetX/*inPos*/);
        x = endX;

        // Right, copy last pixel
        N1Manip::SetBitsRange(pOutLine, x/*startPos*/, outWidth/*endPos*/, N1Manip::GetBitValue(pInLine, srcRight)/*state*/);
        }

    // Bottom (offset.y > 0)
    // Build first line
    if (y < outHeight)
        {
        Byte*       pOutLine = pOutData + y*outPitch;
        Byte const* pInLine = pInData + srcBottom*inPitch;
        uint32_t x = 0;

        // Left, copy bottom left pixel
        N1Manip::SetBitsRange(pOutLine, 0/*startPos*/, beginX/*endPos*/, N1Manip::GetBitValue(pInLine, 0));
        x = beginX;

        // Center
        N1Manip::CopyBits(pOutLine, beginX/*outPos*/, (endX - x)/*pixelCount*/, pInLine, srcOffsetX/*inPos*/);
        x = endX;

        // Right, copy last pixel
        N1Manip::SetBitsRange(pOutLine, x/*startPos*/, outWidth/*endPos*/, N1Manip::GetBitValue(pInLine, srcRight)/*state*/);

        ++y;
        }

    // recopy first line into subsequent lines.
    for (; y < outHeight; ++y)
        {
        Byte*       pOutLine = pOutData + y*outPitch;
        Byte const* pPrevOutLine = pOutData + (y - 1)*outPitch;
        CopyLineN1(pOutLine, 0, outWidth, pPrevOutLine);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImagePP::EvaluateCopyClampPixelsMethods(CopyPixelsMethodN8_T& copyMethod, ClampPixelsMethodN8_T& clampMethod, uint32_t bytePerPixel)
    {
    switch(bytePerPixel)
        {
        case 1:
            copyMethod = &CopyPixelsN8_T<1>;
            clampMethod = &ClampPixels_T<1>;
            break;
        case 2:
            copyMethod = &CopyPixelsN8_T<2>;
            clampMethod = &ClampPixels_T<2>;
            break;
        case 3:
            copyMethod = &CopyPixelsN8_T<3>;
            clampMethod = &ClampPixels_T<3>;
            break;
        case 4:
            copyMethod = &CopyPixelsN8_T<4>;
            clampMethod = &ClampPixels_T<4>;
            break;
        case 6:
            copyMethod = &CopyPixelsN8_T<6>;
            clampMethod = &ClampPixels_T<6>;
            break;
        case 8:
            copyMethod = &CopyPixelsN8_T<8>;
            clampMethod = &ClampPixels_T<8>;
            break;
        case 12:
            copyMethod = &CopyPixelsN8_T<12>;
            clampMethod = &ClampPixels_T<12>;
            break;
        default:
            BeAssert(!"EvaluateCopyClampPixelsMethods unsupported bytePerPixel");
            copyMethod = NULL;
            clampMethod = NULL;
            break;
        }
    return copyMethod != NULL || clampMethod != NULL;
    }
