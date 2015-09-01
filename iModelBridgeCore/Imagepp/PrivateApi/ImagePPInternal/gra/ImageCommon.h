/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/ImagePPInternal/gra/ImageCommon.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ImagePP/all/h/HRAImageOp.h>
#include <ImagePP/all/h/HRPPixelType.h>

#define RLE_RUN_LIMIT 32767
#define RLE_WORST_CASE_BYTES(width) ((width+2)*sizeof(uint16_t))

#ifdef NDEBUG
    #define VALIDATE_COPY_PIXELS
#else 
    #define VALIDATE_COPY_PIXELS    debugOnly_ValidateCopyPixels
    void debugOnly_ValidateCopyPixels(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch,
                                    uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch,
                                    size_t pixelSize, ImagePP::PixelOffset64 const& offset);
#endif

BEGIN_IMAGEPP_NAMESPACE

class HRABitmapRLE;

/*---------------------------------------------------------------------------------**//**
* @bsimethod               
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus CopyPixelsN8(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch,
                           uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch,
                           uint32_t pixelSizeBytes, PixelOffset64 const& offset);

void CopyLineN1(Byte* pOutData, uint32_t posX, uint32_t pixelCount, Byte const* pInData);
void CopyPixelsN1(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch,
                  uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch,
                  PixelOffset64 const& offset);

/*----------------------------------------------------------------------------+
|struct HRAImageBufferParasiteMemory
|This class implements a memory buffer that wraps upon an existing memory.
|typically the buffer will attach to an existing memory buffer such as an
|already allocated tile of bitmap.
|Of course there is no notion of allocation or disallocation involved.
+----------------------------------------------------------------------------*/
struct HRAImageBufferParasiteMemory : public HRAImageBuffer
{
public:
    static HRAImageBufferPtr CreateBuffer(ImagePPStatus& status, Byte* memoryBuffer, size_t sizeInBytes, size_t pitch)
        {
        status = IMAGEPP_STATUS_Success;
        return new HRAImageBufferParasiteMemory(memoryBuffer, sizeInBytes, pitch);
        }

protected:
    HRAImageBufferParasiteMemory(Byte* pBuffer, size_t sizeInBytes, size_t pitch)
       :HRAImageBuffer(pBuffer, sizeInBytes, pitch){}

    virtual ~HRAImageBufferParasiteMemory(){};
    };

/*----------------------------------------------------------------------------+
|struct HRAImageBufferParasiteMemoryWithHold
|This class implements a memory buffer that wraps upon an existing memory.
|typically the buffer will attach to an existing memory buffer such as an
|already allocated tile of bitmap.
|Of course there is no notion of allocation or deallocation involved.
|Additionally the present class template will maintain a hold through any
|smart pointer (RefCounted or HFCShareable) upon some object for his existence. 
|This feature is useful for shared objects upon which we want to intrude
|upon their existing memory
+----------------------------------------------------------------------------*/
template <class POINTER_T> struct HRAImageBufferParasiteMemoryWithHold : public HRAImageBufferParasiteMemory
{
public:
    static HRAImageBufferPtr CreateBuffer(ImagePPStatus& status, Byte* memoryBuffer, size_t sizeInBytes, size_t pitch, POINTER_T pointerHold)
        {
        status = IMAGEPP_STATUS_Success;
        return new HRAImageBufferParasiteMemoryWithHold(memoryBuffer, sizeInBytes, pitch, pointerHold);
        }

protected:
    HRAImageBufferParasiteMemoryWithHold(Byte* pBuffer, size_t sizeInBytes, size_t pitch, POINTER_T pointerHold)
        : HRAImageBufferParasiteMemory(pBuffer, sizeInBytes, pitch),
         m_pointerHold(pointerHold)
        {
        }


    virtual ~HRAImageBufferParasiteMemoryWithHold()
        {
        m_pointerHold = NULL;
        }
    
private:
    POINTER_T m_pointerHold;
};


/*---------------------------------------------------------------------------------**//**
* ! @param[in]   availGrid    The available src grid
* ! @param[in]   reqGrid      The requested dst grid
* @bsimethod                                    Stephane.Poulin                 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<size_t PixelSize_T>
inline void ClampPixels_T(HFCInclusiveGrid const& availGrid, HFCInclusiveGrid const& reqGrid, uint32_t width, uint32_t height, Byte* pData, size_t pitch)
    {
    const HFCInclusiveGrid intersectGrid(HFCInclusiveGrid::FromIntersectionOf(reqGrid, availGrid));

    const uint64_t beginTop = 0;
    const uint64_t endTop = (availGrid.GetYMin() > reqGrid.GetYMin()) ? availGrid.GetYMin() - reqGrid.GetYMin() : 0;
    const uint64_t beginBottom = (reqGrid.GetYMax() > availGrid.GetYMax()) ? reqGrid.GetHeight() - (reqGrid.GetYMax() - availGrid.GetYMax()) : reqGrid.GetHeight();
    const uint64_t endBottom = reqGrid.GetHeight();

    const uint64_t beginLeft = 0;
    const uint64_t endLeft = (availGrid.GetXMin() > reqGrid.GetXMin()) ? availGrid.GetXMin() - reqGrid.GetXMin() : 0;
    const uint64_t beginRight = (reqGrid.GetXMax() > availGrid.GetXMax()) ? reqGrid.GetWidth() - (reqGrid.GetXMax() - availGrid.GetXMax()) : reqGrid.GetWidth();
    const uint64_t endRight = reqGrid.GetWidth();

    uint64_t y = 0;

    // Build first line
    if (endTop > beginTop)
        {
        Byte* pLine = pData;
        Byte* pSrcLine = pData + endTop*pitch;
        uint64_t x = 0;

        // Left
        for (; x < endLeft; ++x)
            memcpy(pLine + x*PixelSize_T, pSrcLine + endLeft*PixelSize_T, PixelSize_T);

        // Center
        for (; x < beginRight; ++x)
            memcpy(pLine + x*PixelSize_T, pSrcLine + x*PixelSize_T, PixelSize_T);

        // Right
        for (; x < endRight; ++x)
            memcpy(pLine + x*PixelSize_T, pSrcLine + (beginRight - 1)*PixelSize_T, PixelSize_T);

        ++y;
        }

    // recopy first line into subsequent lines.
    for (; y < endTop; ++y)
        {
        Byte* pLine = pData + y*pitch;
        Byte* pSrcLine = pData; // first line
        memcpy(pLine, pSrcLine, width*PixelSize_T);
        }

    // Center
    if (endLeft > beginLeft || endRight > beginRight)
        {
        for (; y < beginBottom; ++y)
            {
            Byte* pLine = pData + y*pitch;
            uint64_t x = 0;

            // Left
            for (; x < endLeft; ++x)
                memcpy(pLine + x*PixelSize_T, pLine + endLeft*PixelSize_T, PixelSize_T);

            // Right
            x = beginRight;
            for (; x < endRight; ++x)
                memcpy(pLine + x*PixelSize_T, pLine + (beginRight - 1)*PixelSize_T, PixelSize_T);
            }
        }
    else
        y = beginBottom;

    // Bottom
    // Build last line
    if (endBottom > beginBottom)
        {
        Byte* pLine = pData + y*pitch;
        Byte* pSrcLine = pData + (beginBottom - 1)*pitch; // Last line with valid data
        uint64_t x = 0;

        // Left
        for (; x < endLeft; ++x)
            memcpy(pLine + x*PixelSize_T, pSrcLine + endLeft*PixelSize_T, PixelSize_T);

        // Center
        for (; x < beginRight; ++x)
            memcpy(pLine + x*PixelSize_T, pSrcLine + x*PixelSize_T, PixelSize_T);

        // Right
        for (; x < endRight; ++x)
            memcpy(pLine + x*PixelSize_T, pSrcLine + (beginRight - 1)*PixelSize_T, PixelSize_T);

        ++y;
        }
    else
        y = endBottom;

    // recopy last  line into subsequent lines.
    for (; y < endBottom; ++y)
        {
        Byte* pLine = pData + y*pitch;
        Byte* pSrcLine = pData + beginBottom*pitch;
        memcpy(pLine, pSrcLine, width*PixelSize_T);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<size_t PixelSize_T>
void CopyPixelsN8_T(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch,
                    uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch,
                    PixelOffset64 const& offset)
    {
    BeAssert(inWidth > 0 && inHeight > 0);
    BeAssert(outWidth > 0 && outHeight > 0);
        
    // Source coords
    size_t srcOffsetX = (offset.x < 0) ? 0 : (size_t)offset.x;
    size_t srcOffsetY = (offset.y < 0) ? 0 : (size_t)offset.y;
    
    // Origin\end of the src in dst coords
// > if we were receiving offset in double. Kept as reference.
//     Int64 inBeginX = (offset.x < 0) ? (Int64)(-(floor(offset.x))) : (Int64)(-offset.x);
//     Int64 inBeginY = (offset.y < 0) ? (Int64)(-(floor(offset.y))) : (Int64)(-offset.y);
    
    ptrdiff_t inBeginX = (ptrdiff_t)-offset.x;
    ptrdiff_t inBeginY = (ptrdiff_t)-offset.y;
    ptrdiff_t inEndX = inBeginX + inWidth;
    ptrdiff_t inEndY = inBeginY + inHeight;
   
    size_t srcRight = inWidth - 1;
    size_t srcBottom = inHeight - 1;

    // Intersection part
    size_t beginX = MIN((uint32_t)MAX(inBeginX, 0), outWidth);
    size_t beginY = MIN((uint32_t)MAX(inBeginY, 0), outHeight);
    size_t endX =   MIN((uint32_t)MAX(inEndX, 0), outWidth);
    size_t endY =   MIN((uint32_t)MAX(inEndY, 0), outHeight);

    // Intersection found
    size_t y = 0;

    // Top, (offset.y < 0)
    // Build first line
    if (y < beginY)
        {
        Byte*       pOutLine = pOutData + y*outPitch;
        Byte const* pInLine = pInData; // First line
        size_t x = 0;

        // Left, copy first pixel
        for (; x < beginX; ++x)
            memcpy(pOutLine + x*PixelSize_T, pInLine, PixelSize_T);

        // Center
        memcpy(pOutLine + x*PixelSize_T, pInLine + srcOffsetX*PixelSize_T, (endX - x)*PixelSize_T);
        x = endX;

        // Right, copy last pixel
        for (; x < outWidth; ++x)
            memcpy(pOutLine + x*PixelSize_T, pInLine + srcRight*PixelSize_T, PixelSize_T);

        ++y;
        }

    // recopy first line into subsequent lines.
    for (; y < beginY; ++y)
        {
        Byte*       pOutLine = pOutData + y*outPitch;
        Byte const* pPrevOutLine = pOutData + (y - 1)*outPitch;
        memcpy(pOutLine, pPrevOutLine, outWidth*PixelSize_T);
        }

    // Mid (Intersecting part)
    size_t srcY = srcOffsetY;
    for (; y < endY; ++y, ++srcY)
        {
        Byte* pOutLine = pOutData + y*outPitch;
        Byte const* pInLine = pInData + srcY*inPitch;
        size_t x = 0;

        // Left, copy first pixel
        for (; x < beginX; ++x)
            memcpy(pOutLine + x*PixelSize_T, pInLine, PixelSize_T);
        
        // Center
        memcpy(pOutLine + x*PixelSize_T, pInLine + srcOffsetX*PixelSize_T, (endX - x)*PixelSize_T); 
        x = endX;
        
        // Right, copy last pixel
        for (; x < outWidth; ++x)
            memcpy(pOutLine + x*PixelSize_T, pInLine + srcRight*PixelSize_T, PixelSize_T);
        }

    // Bottom (offset.y > 0)
    // Build first line
    if (y < outHeight)
        {
        Byte*       pOutLine = pOutData + y*outPitch;
        Byte const* pInLine = pInData + srcBottom*inPitch;
        size_t x = 0;

        // Left, copy bottom left pixel
        for (; x < beginX; ++x)
            memcpy(pOutLine + x*PixelSize_T, pInLine, PixelSize_T);
        
        // Center
        memcpy(pOutLine + x*PixelSize_T, pInLine + srcOffsetX*PixelSize_T, (endX - x)*PixelSize_T);
        x = endX;
        
        // Right, copy last pixel
        for (; x < outWidth; ++x)
            memcpy(pOutLine + x*PixelSize_T, pInLine + srcRight*PixelSize_T, PixelSize_T);

        ++y;
        }

    // recopy first line into subsequent lines.
    for (; y < outHeight; ++y)
        {
        Byte*       pOutLine = pOutData + y*outPitch;
        Byte const* pPrevOutLine = pOutData + (y - 1)*outPitch;
        memcpy(pOutLine, pPrevOutLine, outWidth*PixelSize_T);
        }

    VALIDATE_COPY_PIXELS(outWidth, outHeight, pOutData, outPitch, inWidth, inHeight, pInData, inPitch, PixelSize_T, offset);
    }

/*---------------------------------------------------------------------------------**//**
* N1Manip namespace
+---------------+---------------+---------------+---------------+---------------+------*/
namespace N1Manip
{
#define BITSPERPIXEL_N1 (8)
/*---------------------------------------------------------------------------------**//**
* Simple mask template
* RightBitPattern<3>::value == 00000111
* LeftBitPattern<3>::value  == 11100000
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename Type_T, uint32_t bits> struct RightBitPattern { static Type_T const value = 0xFF >> (BITSPERPIXEL_N1 - bits); };
template <typename Type_T> struct RightBitPattern<Type_T, 0> { static Type_T const value = 0; };
template <typename Type_T, uint32_t bits> struct LeftBitPattern { static Type_T const value = 0xFF & (0xFF << (BITSPERPIXEL_N1 - bits)); };
template <typename Type_T> struct LeftBitPattern<Type_T, 0> { static Type_T const value = 0; };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline Byte GetRightBitPattern(uint32_t bits) { return 0xFF >> (BITSPERPIXEL_N1 - bits); }
inline Byte GetLeftBitPattern(uint32_t bits)  { return 0xFF & (0xFF << (BITSPERPIXEL_N1 - bits)); }
inline Byte GetBitPattern(uint32_t firstBit, uint32_t lastBit)  { return ((0xFF >> firstBit) & (0xFF << (7 - lastBit))); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool GetBitValue(Byte const* inData, uint32_t posX)
    {
    return 0 != (inData[posX / BITSPERPIXEL_N1] & (0x80 >> (posX % BITSPERPIXEL_N1)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void SetBitOn(Byte* outData, uint32_t bit)
    {
    BeAssert(bit < BITSPERPIXEL_N1);
    *outData |= (0x80 >> bit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void SetBitOff(Byte* outData, uint32_t bit)
    {
    BeAssert(bit < BITSPERPIXEL_N1);
    *outData &= ~(0x80 >> bit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void SetBit(Byte* outData, uint32_t bit, bool state)
    {
    if (state)
        SetBitOn(outData, bit);
    else
        SetBitOff(outData, bit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void SetBitsRangeInByte(Byte* outData, uint32_t firstBit, uint32_t endBit, bool state)
    {
    BeAssert(firstBit < BITSPERPIXEL_N1);
    BeAssert(endBit <= BITSPERPIXEL_N1);
    if (state)
        *outData |= ((0xFF >> firstBit) & (0xFF << (BITSPERPIXEL_N1 - endBit)));
    else
        *outData &= ((0xFF << (BITSPERPIXEL_N1 - firstBit)) & (0xFF >> endBit));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void SetBitsRange(Byte* outData, uint32_t startPos, uint32_t endPos, bool state)
    {
    BeAssert(startPos <= endPos);
    if (endPos <= startPos)
        return;

    uint32_t firstByte = startPos / BITSPERPIXEL_N1;
    uint32_t lastByte = (endPos - 1) / BITSPERPIXEL_N1;

    if (firstByte == lastByte)
        {
        const uint32_t first = startPos % BITSPERPIXEL_N1;
        const uint32_t end = first + (endPos - startPos);
        SetBitsRangeInByte(outData + firstByte, first, end, state);
        }
    else
        {
        // Set first byte
        SetBitsRangeInByte(outData + firstByte, startPos % BITSPERPIXEL_N1, BITSPERPIXEL_N1/*endPos*/, state);

        // set full bytes
        if (state)
            memset(outData + firstByte + 1, 0xFF, lastByte - firstByte - 1);
        else
            memset(outData + firstByte + 1, 0x00, lastByte - firstByte - 1);

        // Set last byte
        const uint32_t lastPosInLastByte = (endPos - 1) % BITSPERPIXEL_N1;
        SetBitsRangeInByte(outData + lastByte, 0, lastPosInLastByte + 1, state);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void SetBitsOn(Byte* outData, uint32_t pixelCount)
    {
    size_t byteCount = pixelCount / BITSPERPIXEL_N1;
    size_t remainder = pixelCount % BITSPERPIXEL_N1;

    // Set bits on
    memset(outData, 0xFF, byteCount);

    // Set last byte
    if (remainder)
        outData[byteCount] |= (0xFF << (BITSPERPIXEL_N1 - remainder));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void SetBitsOn(Byte* outData, size_t byteCount, size_t remainder)
    {
    memset(outData, 0xFF, byteCount);

    // Set last byte
    if (remainder)
        outData[byteCount] |= (0xFF << (BITSPERPIXEL_N1 - remainder));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void SetBitsOff(Byte* outData, const uint32_t pixelCount)
    {
    size_t byteCount = pixelCount / BITSPERPIXEL_N1;
    size_t remainder = pixelCount % BITSPERPIXEL_N1;

    // Set bits off
    memset(outData, 0x00, byteCount);

    // Set last byte
    if (remainder)
        outData[byteCount] &= (0xFF >> remainder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void SetBitsOff(Byte* outData, const size_t byteCount, const size_t remainder)
    {
    // Set bits off
    memset(outData, 0x00, byteCount);

    // Set last byte
    if (remainder)
        outData[byteCount] &= (0xFF >> remainder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void SetBits(Byte* outData, uint32_t pixelCount, bool state)
    {
    if (state)
        SetBitsOn(outData, pixelCount);
    else
        SetBitsOff(outData, pixelCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t FillFirstByte(Byte* pOutData, uint32_t outPos, uint32_t pixelCount, Byte const* pInData, uint32_t inPos)
    {
    uint32_t inOffset = inPos % BITSPERPIXEL_N1;
    uint32_t outOffset = outPos % BITSPERPIXEL_N1;
    uint32_t outIdx = outPos / BITSPERPIXEL_N1;
    uint32_t inIdx = inPos / BITSPERPIXEL_N1;
    uint32_t bitsToCopy = MIN(pixelCount, BITSPERPIXEL_N1 - outOffset);

    if (0 == bitsToCopy)
        return bitsToCopy;

    if (outOffset <= inOffset)
        {
        uint32_t bitsFromFirstByte = MIN(bitsToCopy, BITSPERPIXEL_N1 - inOffset);
        const uint32_t leftShift = inOffset - outOffset;
        const Byte bitPattern = GetBitPattern(outOffset, outOffset + bitsToCopy - 1);
        if (bitsToCopy <= bitsFromFirstByte)
            {
            // Copy from one byte
            pOutData[outIdx] = (pOutData[outIdx] & ~bitPattern) | ((pInData[inIdx] << leftShift) & bitPattern);
            }
        else
            {
            // Copy from two bytes
            const uint32_t rightShift = outOffset + bitsFromFirstByte;
            pOutData[outIdx] = (pOutData[outIdx] & ~bitPattern) | ((pInData[inIdx] << leftShift) & bitPattern) | ((pInData[inIdx + 1] >> rightShift) & bitPattern);
            }
        }
    else
        {
        const uint32_t rightShift = outOffset - inOffset;
        const Byte bitPattern = GetBitPattern(outOffset, outOffset + bitsToCopy - 1);
        pOutData[outIdx] = (pOutData[outIdx] & ~bitPattern) | ((pInData[inIdx] >> rightShift) & bitPattern);
        }

    return bitsToCopy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline void CopyBits(Byte* pOutData, uint32_t outPos, uint32_t pixelCount, Byte const* pInData, uint32_t inPos)
    {
    if (pixelCount == 0)
        return;

    uint32_t filled = FillFirstByte(pOutData, outPos, pixelCount, pInData, inPos);
    outPos += filled;
    inPos += filled;
    pixelCount -= filled;

    if (pixelCount == 0)
        return;

    uint32_t outIdx = outPos / BITSPERPIXEL_N1;

    // Must be at the beginning of a byte
    BeAssert(outPos % BITSPERPIXEL_N1 == 0);

    // Copy remaining Bytes
    CopyLineN1(pOutData + outIdx, inPos, pixelCount, pInData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
// inline size_t ComputeBytesPerLine(uint64_t width) 
//     {
//     return (width + (BITSPERPIXEL_N1 - 1)) / BITSPERPIXEL_N1; 
//     }


} // N1Manip

/*---------------------------------------------------------------------------------**//**
* Rle1Manip namespace
+---------------+---------------+---------------+---------------+---------------+------*/
namespace Rle1Manip
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool GetState(uint32_t idx) { return (idx & 0x1); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t GetRun(uint32_t posX, const uint16_t* pBuf)
    {
    uint32_t idx = 0;
    for (; posX >= pBuf[idx]; idx++)
        posX -= pBuf[idx];

    return idx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t GetRun(uint32_t& count, uint32_t posX, const uint16_t* pBuf)
    {
    count = posX;
    uint32_t idx = 0;
    for (; posX >= pBuf[idx]; idx++)
        {
        posX -= pBuf[idx];
        }

    count = count + pBuf[idx] - posX;
    return idx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t SetRun(uint32_t idx, uint16_t* pBuf, uint32_t count)
    {
    while (count > RLE_RUN_LIMIT)
        {
        count -= RLE_RUN_LIMIT;
        pBuf[idx] = RLE_RUN_LIMIT;
        pBuf[idx + 1] = 0;
        idx += 2;
        }

    pBuf[idx] = (uint16_t)count;
    return idx;
    }

/*---------------------------------------------------------------------------------**//**
* Return the number of runs copied
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t CopyRuns(uint32_t idxOut, uint16_t* pOutBuf, uint32_t idxFirst, uint32_t idxEnd, uint16_t const* pBuf)
    {
    memcpy((&pOutBuf[idxOut]), (&pBuf[idxFirst]), (idxEnd - idxFirst)*sizeof(uint16_t));
    return (idxEnd - idxFirst);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t WriteFirstRun(uint32_t idx, uint16_t* pBuf, uint32_t count)
    {
    if (GetState(idx))
        idx = SetRun(++idx, pBuf, 0);

    idx = SetRun(idx, pBuf, count);
    return idx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t WriteLastRun(uint32_t idx, uint16_t* pBuf, uint32_t count)
    {
    idx = SetRun(idx, pBuf, count);

    if (GetState(idx))
        return SetRun(++idx, pBuf, 0);

    return idx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  1/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ValidateLineIntegrity(uint16_t const* pLine, uint32_t width, size_t dataSize);
    
} // Rle1Manip

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct RleRunnerWithSizeAndWidth
    {
    public:
        /*---------------------------------------------------------------------------------**//**
        * Constructors
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        RleRunnerWithSizeAndWidth(uint16_t const* pBuf, uint32_t width, size_t dataSize) : m_buf(pBuf), m_count(pBuf[0]), m_idx(0), m_width(width), m_size(dataSize) { }
        RleRunnerWithSizeAndWidth(uint32_t pos, uint16_t const* pBuf, uint32_t width, size_t dataSize) : m_buf(pBuf), m_count(pBuf[0]), m_idx(0), m_width(width), m_size(dataSize) { RunToPos(pos); }
        RleRunnerWithSizeAndWidth(uint32_t pos, RleRunnerWithSizeAndWidth const& hint) : m_buf(hint.m_buf), m_count(hint.m_count), m_idx(hint.m_idx), m_width(hint.m_width), m_size(hint.m_size) { RunToPos(pos); }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t GetCount() const { return m_count; }
        uint32_t GetIndex() const { return m_idx; }
        bool   GetState() const { return (m_idx & 0x1); }

        /*---------------------------------------------------------------------------------**//**
        * Go to next run
        * Return current index
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t GoToNextRun() 
            {
            if (m_count < m_width)
                m_count += m_buf[++m_idx];

            return m_idx;
            }

        /*---------------------------------------------------------------------------------**//**
        * Go to previous run
        * Return current index
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t GoToPreviousRun() 
            {
            if (m_idx > 0)
                m_count -= m_buf[m_idx--];
            return m_idx;
            }

        /*---------------------------------------------------------------------------------**//**
        * Return current index
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t GetRunAtIndex(uint32_t idx)
            {
            return RunToIndex(idx);
            }

    private:
        /*---------------------------------------------------------------------------------**//**
        * Return current index
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t RunToPos(uint32_t posX)
            {
            BeAssert(posX < m_width);
            if (m_count <= posX)
                {// go forward
                if ((posX - m_count) <= (m_width - posX))
                    {
                    return GetRunForward(posX);
                    }
                else
                    {
                    // faster to start from end
                    GoToLast();
                    return GetRunBackward(posX);
                    }
                }
            else
                {// go backward. This is slower than go forward
                if ((m_count - posX) < posX)
                    {
                    return GetRunBackward(posX);
                    }
                else
                    {
                    // faster to start from the beginning
                    GoToFirst();
                    return GetRunForward(posX);
                    }
                }
            }

        /*---------------------------------------------------------------------------------**//**
        * Return current index
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t RunToIndex(uint32_t idx)
            {
            BeAssert(idx <= ((m_size / sizeof(uint16_t)) - 1));
            if (idx > m_idx)
                {// go forward
                while (idx > m_idx)
                    m_count += m_buf[++m_idx];
                }
            else if ((m_idx - idx) < idx)
                {// faster to go backward
                while (idx < m_idx)
                    m_count -= m_buf[m_idx--];
                }
            else
                {// start from the beginning
                GoToFirst();
                while (idx > m_idx)
                    m_count += m_buf[++m_idx];
                }

            return m_idx;
            }

        /*---------------------------------------------------------------------------------**//**
        * Return current index
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t GetRunForward(uint32_t posX)
            {
            while (m_count <= posX)
                m_count += m_buf[++m_idx];

            return m_idx;
            }

        /*---------------------------------------------------------------------------------**//**
        * Return current index
        * This is slower than going forward
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t GetRunBackward(uint32_t posX)
            {
            BeAssert(m_count > posX);
            while (m_count > posX && m_idx > 0)
                m_count -= m_buf[m_idx--];

            if (m_count <= posX)
                m_count += m_buf[++m_idx];

            return m_idx;
            }

        /*---------------------------------------------------------------------------------**//**
        * Go to first run
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        void GoToFirst()
            {
            m_count = m_buf[0];
            m_idx = 0;
            }

        /*---------------------------------------------------------------------------------**//**
        * Go to last run
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        void GoToLast()
            {
            m_count = m_width;
            m_idx = (uint32_t)((m_size / sizeof(uint16_t)) - 1);
            }

        uint16_t const* m_buf;
        uint32_t m_count;
        uint32_t m_idx;
        uint32_t const m_width;
        size_t const m_size; // line size in bytes
    };

/*---------------------------------------------------------------------------------**//**
* Copy pixels RLE to RLE
* pOutData is pre-allocated worst case!
* ! @param[in]    offset       Offset in src buffer
* @bsimethod                                    Stephane.Poulin                 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename Reader_T>
void CopyPixelsRle_T(uint32_t outWidth, uint32_t outHeight, HRAImageBufferRleR outData, uint32_t inWidth, uint32_t inHeight, Reader_T& inData, PixelOffset64 const& offset)
    {
    // Make direct copy if in and out buffers are horizontally aligned and no clamping.
    if ((offset.x == 0) && (outWidth == inWidth) && (offset.y >= 0) && ((offset.y + outHeight) <= inHeight))
        {
        size_t outPitch;
        Byte* pOutLine = outData.GetDataP(outPitch);

        for (uint64_t line = 0; line < outHeight; ++line) // output
            {
            size_t inDataSize;
            Byte const* pInLine = inData.GetLineDataCP((uint32_t)(line + offset.y), inDataSize);

            memcpy(pOutLine + line*outPitch, pInLine, inDataSize);
            outData.SetLineDataSize((uint32_t)line, inDataSize);
            }

        return;
        }

    uint32_t left = (offset.x < 0) ? (uint32_t)-offset.x : 0; // clamp left count

    uint32_t firstInputPixel = (offset.x > 0) ? (uint32_t)offset.x : 0; //src
    if (firstInputPixel > (inWidth - 1))
        firstInputPixel = inWidth - 1;
    uint32_t lastInputPixel = ((offset.x + outWidth - 1) > 0) ? (uint32_t)(offset.x + outWidth - 1) : 0; // src
    if (lastInputPixel > (inWidth - 1))
        lastInputPixel = inWidth - 1;

    size_t outPitch;
    Byte* pOutData = outData.GetDataP(outPitch);

    for (uint32_t y = 0; y < outHeight; ++y) //output
        {
        uint32_t effectiveY = (uint32_t)(MAX(0, MIN(y + offset.y, inHeight - 1))); // src

        uint16_t* pOutLine = (uint16_t*)(pOutData + y*outPitch);

        size_t inDataSize;
        uint16_t const* pInLine = (uint16_t const*)inData.GetLineDataCP(effectiveY, inDataSize);

        RleRunnerWithSizeAndWidth runnerFirst(firstInputPixel, pInLine, inWidth, inDataSize);
        uint32_t idxIn = runnerFirst.GetIndex();

        const RleRunnerWithSizeAndWidth runnerLast(lastInputPixel, runnerFirst);
        uint32_t idxLast = runnerLast.GetIndex();

        uint32_t toWrite = outWidth;
        uint32_t idxOut = 0;

        // Write the first run, must begins with offState
        if (Rle1Manip::GetState(idxIn))
            {
            idxOut = Rle1Manip::SetRun(idxOut, pOutLine, 0); // init first run to 0
            ++idxOut;
            }

        if (idxIn == idxLast)
            {
            idxOut = Rle1Manip::WriteLastRun(idxOut, pOutLine, toWrite); // Sets last run and add offState run if necessary
            outData.SetLineDataSize(y, (idxOut+1)*sizeof(uint16_t));
            continue;
            }

        // set first
        uint32_t outRun = MIN(toWrite, left + runnerFirst.GetCount() - firstInputPixel);
        idxOut = Rle1Manip::SetRun(idxOut, pOutLine, outRun);
        toWrite -= outRun;
        ++idxOut;

        RleRunnerWithSizeAndWidth beforeLast(runnerLast);
        beforeLast.GoToPreviousRun();

        // Copy src runs in the middle of first and last. [first+1, last-1]
        if (beforeLast.GetIndex() > runnerFirst.GetIndex())
            {
            uint32_t runCount = Rle1Manip::CopyRuns(idxOut, pOutLine, runnerFirst.GetIndex() + 1 /*first*/, runnerLast.GetIndex()/*end*/, pInLine);
            toWrite -= (beforeLast.GetCount() - runnerFirst.GetCount());
            idxOut += runCount;
            }

        // Write last
        if (toWrite > 0)
            {
            idxOut = Rle1Manip::SetRun(idxOut, pOutLine, toWrite);
            ++idxOut;
            }

        // idxOut is incremented and points to the next run
        // End with OffState
        if (!Rle1Manip::GetState(idxOut))
            {
            idxOut = Rle1Manip::SetRun(idxOut, pOutLine, 0); // init last run to 0
            ++idxOut;
            }

        outData.SetLineDataSize(y, idxOut*sizeof(uint16_t));
        }
    }


typedef void (*CopyPixelsMethodN8_T)(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch,
                                     uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch,
                                     PixelOffset64 const& offset);

typedef void (*ClampPixelsMethodN8_T)(HFCInclusiveGrid const& availGrid, HFCInclusiveGrid const& reqGrid, uint32_t width, uint32_t height, Byte* pData, size_t pitch);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateCopyClampPixelsMethods(CopyPixelsMethodN8_T& copyMethod, ClampPixelsMethodN8_T& clampMethod, uint32_t bytePerPixel);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<class _GridStripper>
struct GridStripIterator_T : std::iterator<std::forward_iterator_tag, _GridStripper>
{
public:

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  07/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    typedef typename _GridStripper::value_type Stripper_value_type;
    GridStripIterator_T() 
        :m_pGridStripper(NULL)
        {
        ToEnd();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  07/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    GridStripIterator_T (_GridStripper const& stripper)
        :m_pGridStripper(&stripper)
        {
        if (GetGrid().GetWidth() == 0 || GetGrid().GetHeight() == 0)
            {
            ToEnd();
            return;
            }

        int64_t startScanline = MAX(0, GetGrid().GetYMin());

        int64_t endScanline = ((startScanline + GetAligment()) / GetAligment()) * GetAligment();

        // Bound to the grid
        endScanline = MIN(endScanline, GetGrid().GetYMax() + 1);

        m_current.InitFromLenght((double)GetGrid().GetXMin(), (double)startScanline, GetGrid().GetWidth(), endScanline - startScanline);
        }
 
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  07/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool    operator!= (const GridStripIterator_T& other) const
        {
        return m_current != other.m_current || m_pGridStripper != other.m_pGridStripper;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  07/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    Stripper_value_type const& operator* () const {return m_current;}    
 
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  07/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    const GridStripIterator_T& operator++ ()
        {
        if (m_pGridStripper == NULL)
            return *this;

        int64_t startScanline = m_current.GetYMax() + 1;
        if (startScanline > GetGrid().GetYMax())
            {
            ToEnd();
            }
        else
            {
            //Int64 endScanline = startScanline + (startScanline + GetAligment()) % GetAligment();
            int64_t endScanline = ((startScanline + GetAligment()) / GetAligment()) * GetAligment();

            // Bound to the grid
            endScanline = MIN(endScanline, GetGrid().GetYMax() + 1);
            m_current.InitFromLenght((double)GetGrid().GetXMin(), (double)startScanline, GetGrid().GetWidth(), endScanline - startScanline);
            }

        return *this;
        }
 
private:
    HFCInclusiveGrid const& GetGrid() const { return m_pGridStripper->m_grid; }
    uint32_t GetAligment() const { return m_pGridStripper->m_heightAlignment; }  

    void ToEnd()
        {
        m_current.InitEmpty();
        m_pGridStripper = NULL;
        }

    Stripper_value_type     m_current;
    _GridStripper const*    m_pGridStripper;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct HFCGridStripper
{
    typedef HFCInclusiveGrid value_type;
    typedef  GridStripIterator_T<HFCGridStripper> const_iterator;

    HFCGridStripper(value_type const& grid, uint32_t heightAligment)
        :m_grid(grid),
         m_heightAlignment(heightAligment){}

    const_iterator begin() const { return const_iterator(*this); }
    const_iterator end() const { return const_iterator(); }

    value_type const& m_grid;
    uint32_t m_heightAlignment;
};

/*---------------------------------------------------------------------------------**//**
* _SquareTessellation
+---------------+---------------+---------------+---------------+---------------+------*/
template<class _SquareTessellation>
struct SquareTessellationIterator_T : std::iterator<std::forward_iterator_tag, _SquareTessellation>
{
public:
    typedef typename _SquareTessellation::value_type Stripper_value_type;
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    SquareTessellationIterator_T() : m_pTessellation(NULL)
        {
        ToEnd();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    SquareTessellationIterator_T(_SquareTessellation const& stripper) : m_pTessellation(&stripper)
        {
        if (GetGrid().GetWidth() == 0 || GetGrid().GetHeight() == 0)
            {
            ToEnd();
            return;
            }

        int64_t startX = (GetGrid().GetXMin() - GetOriginX());
        int64_t startY = (GetGrid().GetYMin() - GetOriginY());
        startX = startX / GetWidth();
        startY = startY / GetHeight();

        startX = GetOriginX() + startX*GetWidth();
        startY = GetOriginY() + startY*GetHeight();

        m_current.InitFromLenght((double)startX, (double)startY, GetWidth(), GetHeight());
        }
 
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool operator!= (const SquareTessellationIterator_T& other) const
        {
        return m_current != other.m_current || m_pTessellation != other.m_pTessellation;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    Stripper_value_type const& operator* () const { return m_current; }
 
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    const SquareTessellationIterator_T& operator++ ()
        {
        if (m_pTessellation == NULL)
            return *this;

        int64_t nextX = m_current.GetXMax() + 1;
        int64_t startY = m_current.GetYMax() + 1;
        
        if (nextX > GetGrid().GetXMax() && startY > GetGrid().GetYMax())
            {
            ToEnd();
            return *this;
            }

        // Increment X
        if (nextX <= GetGrid().GetXMax())
            {
            m_current.InitFromLenght((double)nextX, (double)m_current.GetYMin(), GetWidth(), GetHeight());
            return *this;
            }

        // Increment Y, reset X
        BeAssert(startY <= GetGrid().GetYMax());
        int64_t startX = (GetGrid().GetXMin() - GetOriginX());
        startX = startX / GetWidth();
        startX = GetOriginX() + startX*GetWidth();
        m_current.InitFromLenght((double)startX, (double)startY, GetWidth(), GetHeight());

        return *this;
        }
 
private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HFCInclusiveGrid const& GetGrid() const { return m_pTessellation->m_grid; }
    uint32_t GetWidth() const { return m_pTessellation->m_blockSizeX; }
    uint32_t GetHeight() const { return m_pTessellation->m_blockSizeY; }
    uint32_t GetOriginX() const { return m_pTessellation->m_originX; }
    uint32_t GetOriginY() const { return m_pTessellation->m_originY; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ToEnd()
        {
        m_current.InitEmpty();
        m_pTessellation = NULL;
        }

    Stripper_value_type        m_current;
    _SquareTessellation const* m_pTessellation;
};

/*---------------------------------------------------------------------------------**//**
* SquareTessellation
+---------------+---------------+---------------+---------------+---------------+------*/
struct SquareTessellation
{
    typedef HFCInclusiveGrid value_type;
    typedef  SquareTessellationIterator_T<SquareTessellation> const_iterator;

    SquareTessellation(value_type const& grid, uint32_t blockSizeX, uint32_t blockSizeY)
        :m_grid(grid), m_blockSizeX(blockSizeX), m_blockSizeY(blockSizeY), m_originX(0), m_originY(0){}
    SquareTessellation(value_type const& grid, uint32_t blockSizeX, uint32_t blockSizeY, uint32_t originX, uint32_t originY)
        :m_grid(grid), m_blockSizeX(blockSizeX), m_blockSizeY(blockSizeY), m_originX(originX), m_originY(originY){}

    const_iterator begin() const { return const_iterator(*this); }
    const_iterator end() const { return const_iterator(); }

    value_type const& GetGrid() const { return m_grid; }

    value_type const& m_grid;
    uint32_t m_blockSizeX;
    uint32_t m_blockSizeY;
    uint32_t m_originX;
    uint32_t m_originY;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Nicolas.Marquis 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename Out_T>
inline Out_T clip_cast(float const& value)
    {
    // *** If you get an error here, you must implement the specialization for type 'Out_T'
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Nicolas.Marquis 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline uint8_t clip_cast(float const& value)
    {
    return (uint8_t)(value < 0 ? 0 : value > UCHAR_MAX ? UCHAR_MAX : value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Nicolas.Marquis 07/2014     
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline int8_t clip_cast(float const& value)
    {
    return (int8_t)(value < SCHAR_MIN ? SCHAR_MIN : value > SCHAR_MAX ? SCHAR_MAX : value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Nicolas.Marquis 07/2014                       
+--------------+---------------+---------------+---------------+---------------+------*/
template<>
inline uint16_t clip_cast(float const& value)
    {
    return (uint16_t)(value < 0 ? 0 : value > USHRT_MAX ? USHRT_MAX : value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Nicolas.Marquis 07/2014                    
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline int16_t clip_cast(float const& value)
    {
    return (int16_t)(value < SHRT_MIN ? SHRT_MIN : value > SHRT_MAX ? SHRT_MAX : value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Nicolas.Marquis 07/2014    
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline int32_t clip_cast(float const& value)
    {
    return (int32_t)value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Nicolas.Marquis 07/2014                      
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline uint32_t clip_cast(float const& value)
    {
    return (uint32_t)(value < 0 ? 0 : value > UINT_MAX ? UINT_MAX : value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Nicolas.Marquis 07/2014                            
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline float clip_cast(float const& value)
    {
    return value;
    }

END_IMAGEPP_NAMESPACE