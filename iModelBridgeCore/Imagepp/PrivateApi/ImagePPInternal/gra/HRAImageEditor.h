//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PrivateApi/ImagePPInternal/gra/HRAImageEditor.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <ImagePP/h/HmrMacro.h>

#include <ImagePP/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRAImageOp.h>
#include "ImageCommon.h"        //<ImagePPInternal/gra/ImageCommon.h>
#include "HRAImageSurface.h"    //<ImagePPInternal/gra/HRAImageSurface.h>

#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDPacketRLE.h>

BEGIN_IMAGEPP_NAMESPACE 

#define ON_BLACK_STATE(bufferIndex) (!(bufferIndex & 0x00000001))       // block run is ON even number. 0,2,4,6...

/*=================================================================================**//**
* ImageEditor

Each surface will provide a typdef to an ImageEditorXXX. Caller will use the editor as a 
template parameter to its processing algorithm to benefit from code inlining or at a minimum avoid virtual calls.

The editor must provide the following non-virtual methods:

All methods come in pair with an optional datasize param.Ask for the datasize only when required to avoid unnecessary overhead.

-A read - Only access to the pixels at a specific position.The original pixels are returned if possible otherwise a copy is returned.
 Byte const* GetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount) const;
 Byte const* GetPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount) const;

-Modify pixels at a specific position.
 void SetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount) const;
 void SetPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount) const;

-Get a write access to pixels at a specific position.The returned pixels might be the original pixels or a working copy big enough to hold
 pixels modification(worst - case scenario for RLE).You must call UnLockPixels() after your done.
 void LockPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount) const;
 void LockPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount) const;
 void UnLockPixels(size_t newDataSize)
 void UnLockPixels()
+===============+===============+===============+===============+===============+======*/

/*---------------------------------------------------------------------------------**//**
* Byte-Aligned (N8) DataSource_T must have the following members:
*   GetPixelType()  > The source pixelType
*   GetWidth()      > The source width
*   GetHeight()     > The source height
*   m_pData         > the start of the buffer.
*   m_pitch         > size in bytes (including padding) of a line.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageEditorN8
{
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageEditorN8(HRAImageSampleR sample)
        {
        BeAssert(sample.GetPixelType().CountPixelRawDataBits() % 8 == 0);
        
        m_width = sample.GetWidth();
        m_height = sample.GetHeight();
        m_pData = sample.GetBufferP()->GetDataP(m_pitch);
        m_pixelSize = sample.GetPixelType().CountPixelRawDataBits() / 8;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageEditorN8(HRAPacketN8Surface& surface)
        {
        BeAssert(surface.GetPixelType().CountPixelRawDataBits() % 8 == 0);

        m_width = surface.GetWidth();
        m_height = surface.GetHeight();
        m_pData = surface.GetDataP(m_pitch);
        m_pixelSize = surface.GetPixelType().CountPixelRawDataBits() / 8;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageEditorN8(HRASampleN8Surface& surface)
        :ImageEditorN8(surface.GetSampleR()) //delegating constructors
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte const* GetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount) const
        {
        BeAssert(posX + pixelCount <= GetWidth());
        BeAssert(posY < GetHeight());
        
        return m_pData + posY*m_pitch + m_pixelSize*posX;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte const* GetPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount) const
        {
        dataSize = m_pixelSize*pixelCount;

        return GetPixels(posX, posX, pixelCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void SetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount, Byte const* pData)
        {
        BeAssert(posX + pixelCount <= GetWidth());
        BeAssert(posY < GetHeight());

        memcpy(m_pData + posY*m_pitch + m_pixelSize*posX, pData, pixelCount*m_pixelSize);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void SetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount, Byte const* pData, size_t dataSize)
        {
        BeAssert(m_pixelSize*pixelCount == dataSize);
        SetPixels(posX, posY, pixelCount, pData);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte* LockPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount)
        {
        BeAssert(posX + pixelCount <= GetWidth());
        BeAssert(posY < GetHeight());
        
        return m_pData + posY*m_pitch + m_pixelSize*posX;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte* LockPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount)
        {
        dataSize = m_pixelSize*pixelCount;
        
        return LockPixels(posX, posY, posY);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void UnLockPixels(size_t newDataSize)
        {
        UnLockPixels();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void UnLockPixels()
        {
        // nothing to do we returned the original pixels and they were edited directly.
        }

    inline uint32_t GetWidth() const { return m_width; }
    inline uint32_t GetHeight() const { return m_height; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void Clear(uint32_t width, uint32_t height, Byte* pBuf, size_t pitch, uint32_t bytesPerPixel, Byte const* rawData)
        {
        bool pixelEqual = true;
        for (uint64_t i = 1; i < bytesPerPixel && pixelEqual; ++i)
            {
            if (rawData[0] != rawData[i])
                pixelEqual = false;
            }

        const uint32_t dataSize = width * bytesPerPixel;

        if (pixelEqual)
            {
            // Should be faster to use memset than memcpy
            for (uint64_t line = 0; line < height; ++line)
                memset(&pBuf[line*pitch], rawData[0], dataSize);
            }
        else
            {
            // build first line
            for (uint32_t x = 0; x < width; x++)
                memcpy(&pBuf[x*bytesPerPixel], rawData, bytesPerPixel);

            // Recopy first line into subsequent lines.
            for (uint64_t line = 1; line < height; ++line)
                memcpy(&pBuf[line*pitch], &pBuf[0], dataSize);
            }
        }


    uint32_t m_width;
    uint32_t m_height;
    Byte* m_pData;
    size_t m_pitch;    
    size_t m_pixelSize;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Source_T>
struct ImageEditorBaseRle
{
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  12/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void UpdateLastPosition(uint32_t line, uint32_t position, uint32_t index)
        {
        RLEBufferPosition& bufPos = m_lastBufferPositions[line];
        bufPos.m_index = index;
        bufPos.m_position = position;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  12/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void GetIndexAtPosition(uint32_t& index, uint32_t& effectivePosition, uint32_t posX, uint32_t posY, Byte const* pInData)
        {
        if(0 == posX)
            {
            index = effectivePosition = 0;    // Do not update last position when 0.
            return;
            }

        uint16_t const* pInLine = (uint16_t const*)pInData;

        RLEBufferPosition& lastPosition = m_lastBufferPositions[posY];

        // Go forward?
        if(lastPosition.m_position <= posX) // if equal we will do nothing.
            {
            while(lastPosition.m_position < posX)   
                {
                lastPosition.m_position += pInLine[lastPosition.m_index];
                ++lastPosition.m_index;
                }
            }
        else  // Go backward
            {
            while (lastPosition.m_position > posX)
                {
                lastPosition.m_position -= pInLine[lastPosition.m_index-1];
                --lastPosition.m_index;
                }
            
            lastPosition.m_position += pInLine[lastPosition.m_index];
            ++lastPosition.m_index;
            }

        index = lastPosition.m_index;
        effectivePosition = lastPosition.m_position;        
        }

    /*---------------------------------------------------------------------------------**//**
    * Hold information about the last call to Lock. 
    * @bsiclass
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct LastLockRequest
        {
        bool IsLock() const { return 0 != m_pixelCount; }

        void Reset() {m_pixelCount = 0;}

        void Init(uint32_t posX, uint32_t posY, uint32_t pixelCount)
            {
            m_posX = posX;
            m_posY = posY;
            m_pixelCount = pixelCount;
            }

        uint32_t m_posX;
        uint32_t m_posY;
        uint32_t m_pixelCount;
        };

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageEditorBaseRle(Source_T& source)
        :m_source(source)
        {
        m_lastLockRequest.Reset();
        m_workingRunsQuery.resize(GetWorkingBufferSize());
        m_workingRunsEdit.resize(GetWorkingBufferSize());
        m_lastBufferPositions.resize(GetHeight());
        memset(&m_lastBufferPositions[0], 0, m_lastBufferPositions.size()*sizeof(RLEBufferPosition));
        }

    ~ImageEditorBaseRle(){};

    HRPPixelType const& GetPixelType() const { return m_source.GetPixelType(); }
    uint32_t GetWidth() const { return m_source.GetWidth(); }
    uint32_t GetHeight() const { return m_source.GetHeight(); }
    size_t GetWorkingBufferSize() const { return RLE_WORST_CASE_BYTES(GetWidth()); }
                  
protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t ComputeWorkingCopy(Byte* pOutData, uint32_t posX, uint32_t pixelCount, Byte const* pInData, size_t inDataSize) const
        {
        BeAssert(posX + pixelCount <= GetWidth());

        uint16_t* pOutLine = (uint16_t*)pOutData;
        uint16_t const* pInLine = (uint16_t const*)pInData;

        // Perfect match. simple copy
        if (0 == posX && GetWidth() == pixelCount)
            {
            memcpy(pOutLine, pInLine, inDataSize);
            return inDataSize;            
            }         

        // Happen in some cases, code below cannot handle that, it will read beyond the source.
        if (0 == pixelCount)
            return 0;
       
        uint32_t lineIndex = 0;
        uint32_t position = pInLine[0];

        // Find starting position
        while (position <= posX)
            {
            ++lineIndex;
            position += pInLine[lineIndex];
            }
        uint32_t startIndex = lineIndex;
        uint32_t startPosition = (position - pInLine[lineIndex]);

        // Find ending position
        while (position < (posX + pixelCount))
            {
            ++lineIndex;
            position += pInLine[lineIndex];
            }

        uint32_t outIndex = 0;

        uint32_t runToCopy = ((lineIndex - startIndex) + 1);
        if (startIndex & 0x1)
            {
            pOutLine[0] = 0;    // must start on black

            // Copy RLE runs as they are, will adjust afterward.
            memcpy(pOutLine + 1, pInLine + startIndex, runToCopy*sizeof(uint16_t));

            BeAssert((posX - startPosition) < RLE_RUN_LIMIT);

            // remove extras from the first run
            pOutLine[1] -= (uint16_t)(posX - startPosition);

            outIndex = runToCopy + 1;            
            }
        else
            {
            // Copy RLE runs as they are, will adjust afterward.
            memcpy(pOutLine, pInLine + startIndex, runToCopy*sizeof(uint16_t));

            BeAssert((posX - startPosition) < RLE_RUN_LIMIT);

            // remove extras from the first run
            pOutLine[0] -= (uint16_t)(posX - startPosition);

            outIndex = runToCopy;
            }

        BeAssert((position - (posX + pixelCount)) < RLE_RUN_LIMIT);

        // remove extras from the last run
        pOutLine[outIndex-1] -= (uint16_t)(position - (posX + pixelCount));

        // make sure we end on black
        if (!(outIndex & 0x1))
            {
            pOutLine[outIndex] = 0;
            ++outIndex;
            }

        return outIndex*sizeof(uint16_t);   
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t MergeRuns(Byte* pOutLine, Byte* pCurrentLine, size_t DataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount, Byte const* pNewLine)
        {
        BeAssert(pixelCount > 0);
        BeAssert(posX + pixelCount <= GetWidth());

        uint16_t* pWorkingRuns = (uint16_t*)pOutLine;
        uint16_t* pCurrentRun = (uint16_t*)pCurrentLine;
        uint16_t const* pNewData = (uint16_t const*)pNewLine;
        uint32_t CurrentRunIndex = 0;
        uint32_t PixelsFromCurrentRun = 0;
        uint32_t WorkRunIndex = 0;
        uint32_t width = GetWidth();

        // copy the first segment from the run into the new run
        // N.B. its faster, by about 20%, to compute index and do a single memcpy.
        GetIndexAtPosition(CurrentRunIndex, PixelsFromCurrentRun, posX, posY, pCurrentLine);

#if 0   // DEBUG_PURPOSE_ONLY
        static bool s_validatePos = false;

        // Kept to validate errors with GetIndexAtPosition
        if(s_validatePos)
            {
            uint32_t PixelsFromCurrentRun_Reel = 0;
            uint32_t CurrentRunIndex_Reel = 0;
            // copy the first segment from the run into the new run
            // N.B. its faster, by about 20%, to compute index and do a single memcpy.
            while (PixelsFromCurrentRun_Reel < posX)
                {
                PixelsFromCurrentRun_Reel += pCurrentRun[CurrentRunIndex_Reel];
                ++CurrentRunIndex_Reel;
                }
            BeAssert(CurrentRunIndex_Reel == CurrentRunIndex);
            BeAssert(PixelsFromCurrentRun_Reel == PixelsFromCurrentRun);
            }
#endif

        memcpy(pWorkingRuns, pCurrentRun, CurrentRunIndex*sizeof(uint16_t));
        WorkRunIndex = CurrentRunIndex;

        size_t   PixelsFromNewRun = 0;
        uint32_t NewRunIndex = 0;

        // Skip empty runs.
        while (pNewData[NewRunIndex] == 0)
            ++NewRunIndex;

        // cut previous run. So pWorkingRuns pos == posX
        if (PixelsFromCurrentRun > posX)
            pWorkingRuns[WorkRunIndex - 1] -= (uint16_t)(PixelsFromCurrentRun - posX);

        // If we are not on the same state we must adjust with previous.
        if (ON_BLACK_STATE(NewRunIndex) != ON_BLACK_STATE(WorkRunIndex))
            {
            if (0 != WorkRunIndex)
                {
                size_t ToWrite = pWorkingRuns[WorkRunIndex - 1] + pNewData[NewRunIndex];

                while (ToWrite > RLE_RUN_LIMIT)
                    {
                    pWorkingRuns[WorkRunIndex - 1] = RLE_RUN_LIMIT;
                    pWorkingRuns[WorkRunIndex] = 0;
                    ToWrite -= RLE_RUN_LIMIT;
                    WorkRunIndex += 2;
                    }
                pWorkingRuns[WorkRunIndex - 1] = (uint16_t)ToWrite;

                // Append to previous run
                PixelsFromNewRun = pNewData[NewRunIndex];
                ++NewRunIndex;
                }
            else
                {
                pWorkingRuns[0] = 0;
                WorkRunIndex = 1;
                }
            }

        HASSERT(ON_BLACK_STATE(NewRunIndex) == ON_BLACK_STATE(WorkRunIndex));

        // Copy the new run.
        int32_t RunIndexStart = NewRunIndex;
        while (PixelsFromNewRun < pixelCount)
            {
            PixelsFromNewRun += pNewData[NewRunIndex];
            ++NewRunIndex;
            }
        memcpy(&pWorkingRuns[WorkRunIndex], &pNewData[RunIndexStart], (NewRunIndex - RunIndexStart)*sizeof(uint16_t));
        WorkRunIndex += (NewRunIndex - RunIndexStart);

        // I don't know if that can happen but adjust last run in case where pi_pRun encoded more that pixelCount
        if (PixelsFromNewRun > pixelCount)
            {
            HPOSTCONDITION(!"Number of pixels in pi_pRun is NOT equal to pixelCount");

            // cut previous run.
            pWorkingRuns[WorkRunIndex - 1] -= (uint16_t)(PixelsFromNewRun - pixelCount);
            PixelsFromNewRun = pixelCount;
            }

        // If the new run covered till the end of the line there is nothing else to copy.
        if (posX + PixelsFromNewRun < width)
            {
            // Goto end merge position in current run.
            size_t skipTo = posX + pixelCount;
            while (PixelsFromCurrentRun < skipTo)
                {
                PixelsFromCurrentRun += pCurrentRun[CurrentRunIndex];
                ++CurrentRunIndex;
                }

            // cut previous run. So pCurrentRun pos == posX + pixelCount
            if (PixelsFromCurrentRun > skipTo)
                pCurrentRun[--CurrentRunIndex] = (uint16_t)(PixelsFromCurrentRun - skipTo);

            size_t currentPos = skipTo;

            // If we are not on the same state we must adjust with previous.
            if (ON_BLACK_STATE(CurrentRunIndex) != ON_BLACK_STATE(WorkRunIndex))
                {
                size_t ToWrite = pWorkingRuns[WorkRunIndex - 1] + pCurrentRun[CurrentRunIndex];

                while (ToWrite > RLE_RUN_LIMIT)
                    {
                    pWorkingRuns[WorkRunIndex - 1] = RLE_RUN_LIMIT;
                    pWorkingRuns[WorkRunIndex] = 0;
                    ToWrite -= RLE_RUN_LIMIT;
                    WorkRunIndex += 2;
                    }
                pWorkingRuns[WorkRunIndex - 1] = (uint16_t)ToWrite;

                // Append to previous run
                currentPos += pCurrentRun[CurrentRunIndex];
                ++CurrentRunIndex;
                }

            UpdateLastPosition(posY, (uint32_t)currentPos, WorkRunIndex);

            // We should be able to copy all the remaining runs without counting but for extra precaution we count them.
            int32_t CurrentRunIndexStart = CurrentRunIndex;
            while (currentPos < width)
                {
                currentPos += pCurrentRun[CurrentRunIndex];
                ++CurrentRunIndex;
                }
            memcpy(&pWorkingRuns[WorkRunIndex], &pCurrentRun[CurrentRunIndexStart], (CurrentRunIndex - CurrentRunIndexStart)*sizeof(uint16_t));
            WorkRunIndex += (CurrentRunIndex - CurrentRunIndexStart);

            HPOSTCONDITION(currentPos == width);

            // Make sure we have the right number of pixels.
            if (currentPos > width)
                pWorkingRuns[WorkRunIndex - 1] -= (uint16_t)(currentPos - width);
            }
        else
            {
            UpdateLastPosition(posY, width, WorkRunIndex);  // We encoded till the end.
            }

        // Must finish on black
        if (ON_BLACK_STATE(WorkRunIndex))
            {
            pWorkingRuns[WorkRunIndex] = 0;
            ++WorkRunIndex;
            }

        // Copy runs to real buffer.
        size_t RunLenInBytes = WorkRunIndex * sizeof(uint16_t);

        BeDataAssertOnce(Rle1Manip::ValidateLineIntegrity(pWorkingRuns, width, RunLenInBytes));
        return RunLenInBytes;
        }

    struct RLEBufferPosition
        {
        uint32_t m_index;
        uint32_t m_position;
        };

    Source_T& m_source;
    LastLockRequest m_lastLockRequest;
    mutable vector<Byte> m_workingRunsQuery;
    vector<Byte> m_workingRunsEdit;
    vector<RLEBufferPosition> m_lastBufferPositions;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageEditorSampleRle : public ImageEditorBaseRle<HRAImageSample>
{
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageEditorSampleRle(HRASampleRleSurface& surface)
        :ImageEditorSampleRle(surface.GetSampleR()) //delegating constructors
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageEditorSampleRle(HRAImageSampleR sample)
        :ImageEditorBaseRle<HRAImageSample>(sample),
        m_pBufferRle(sample.GetBufferRleP())
        {
        BeAssert(GetPixelType().CountPixelRawDataBits() == 1);
        BeAssert(sample.GetBufferRleP() != NULL);
        
        m_pData = sample.GetBufferP()->GetDataP(m_pitch);
        }

    ~ImageEditorSampleRle() {/*Unfortunately we have badly encoded source files. Validation is now per line after the merge operation. 
                             BeAssert(m_source.ValidateIntegrity());*/}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte const* GetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount) const
        {
        size_t dataSize;
        return GetPixels(dataSize, posX, posY, pixelCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void SetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount, Byte const* pNewData, size_t newDataSize)
        {
        SetPixels(posX, posY, pixelCount, pNewData);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte* LockPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount)
        {
        size_t dataSize;
        return LockPixels(dataSize, posX, posY, pixelCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void UnLockPixels(size_t newDataSize)
        {
        UnLockPixels();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte const* GetPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount) const
        {
        BeAssert(posX + pixelCount <= GetWidth());
        BeAssert(posY < GetHeight());

        // Perfect match, no need for a copy.
        if (0 == posX && GetWidth() == pixelCount)
            {
            dataSize = m_pBufferRle->GetLineDataSize(posY);
            return m_pData + (posY*m_pitch);
            }
            
        dataSize = ComputeWorkingCopy(&m_workingRunsQuery[0], posX, pixelCount, m_pData + (posY*m_pitch), m_pBufferRle->GetLineDataSize(posY));
        return &m_workingRunsQuery[0];
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void SetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount, Byte const* pNewData)
        {
        BeAssert(posX + pixelCount <= GetWidth());
        BeAssert(posY < GetHeight());
        
        size_t newSize = MergeRuns(&m_workingRunsEdit[0], m_pData + posY*m_pitch, m_pBufferRle->GetLineDataSize(posY), posX, posY, pixelCount, pNewData);

        memcpy(m_pData + posY*m_pitch, &m_workingRunsEdit[0], newSize);
        m_pBufferRle->SetLineDataSize(posY, newSize);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte* LockPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount)
        {
        BeAssert(!m_lastLockRequest.IsLock());  // Unlock call is missing
        BeAssert(posX + pixelCount <= GetWidth());
        BeAssert(posY < GetHeight());
        
        m_lastLockRequest.Init(posX, posY, pixelCount);

        dataSize = ComputeWorkingCopy(&m_workingRunsQuery[0], posX, pixelCount, m_pData + (posY*m_pitch), m_pBufferRle->GetLineDataSize(posY));
        return &m_workingRunsQuery[0];
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void UnLockPixels()
        {
        BeAssert(m_lastLockRequest.IsLock());  // Lock call is missing or double unlock.

        size_t newSize = MergeRuns(&m_workingRunsEdit[0], m_pData + m_lastLockRequest.m_posY*m_pitch, m_pBufferRle->GetLineDataSize(m_lastLockRequest.m_posY), m_lastLockRequest.m_posX, m_lastLockRequest.m_posY, m_lastLockRequest.m_pixelCount, &m_workingRunsQuery[0]);
       
        memcpy(m_pData + m_lastLockRequest.m_posY*m_pitch, &m_workingRunsEdit[0], newSize);
        m_pBufferRle->SetLineDataSize(m_lastLockRequest.m_posY, newSize);

        HDEBUGCODE(m_lastLockRequest.Reset());
        }  
               
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void Clear(uint32_t width, uint32_t height, HRAImageBufferRleR bufferRle, bool state)
        {
        size_t pitch;
        uint16_t* pBuf = reinterpret_cast<uint16_t*>(bufferRle.GetDataP(pitch));

        BeAssert(height > 0);
        BeAssert(NULL != pBuf);

        uint32_t idx;

        if (state)
            {
            // build first line
            idx = Rle1Manip::SetRun(0, &pBuf[0], 0);
            ++idx;
            idx = Rle1Manip::SetRun(idx, &pBuf[idx], width);
            ++idx;
            idx = Rle1Manip::SetRun(idx, &pBuf[idx], 0);
            ++idx;
            }
        else
            {
            idx = Rle1Manip::SetRun(0, &pBuf[0], width);
            ++idx;
            }

        const size_t dataSize = idx*sizeof(uint16_t);
        bufferRle.SetLineDataSize(0, dataSize);

        // Recopy first line into subsequent lines.
        for (uint64_t line = 1; line < height; ++line)
            {
            memcpy(((Byte*)pBuf) + line*pitch, pBuf, dataSize);
            bufferRle.SetLineDataSize((uint32_t)line, dataSize);
            }
        }

private:
    HRAImageBufferRleP m_pBufferRle;
    Byte* m_pData;
    size_t m_pitch;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageEditorPacketRle : public ImageEditorBaseRle<HRAPacketRleSurface>
{
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageEditorPacketRle(HRAPacketRleSurfaceR surface)
        :ImageEditorBaseRle<HRAPacketRleSurface>(surface)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline HCDPacketRLE& GetPacketAt(uint32_t& lineInPacket, uint32_t posY) const
        {
        uint32_t packetOffsetY;
        HCDPacketRLE& packet = m_source.GetPacketRLE(packetOffsetY, posY);
       
        lineInPacket = posY - packetOffsetY;
        return packet;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte const* GetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount) const
        {
        size_t dataSize;
        return GetPixels(dataSize, posX, posY, pixelCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void SetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount, Byte const* pNewData, size_t dataSize)
        {
        SetPixels(posX, posY, pixelCount, pNewData);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte* LockPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount)
        {
        size_t dataSize;
        return LockPixels(dataSize, posX, posY, pixelCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void UnLockPixels(size_t newDataSize)
        {
        UnLockPixels();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte const* GetPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount) const
        {
        BeAssert(posX + pixelCount <= GetWidth());
        BeAssert(posY < GetHeight());
        
        uint32_t lineInPacket = 0;
        HCDPacketRLE& packet = GetPacketAt(lineInPacket, posY);
        
        Byte const* pLine = packet.GetLineBuffer(lineInPacket);

        // Perfect match, no need for a copy.
        if (0 == posX && GetWidth() == pixelCount)
            return pLine;
            
        dataSize = ComputeWorkingCopy(&m_workingRunsQuery[0], posX, pixelCount, pLine, packet.GetLineDataSize(lineInPacket));
        return &m_workingRunsQuery[0];
        }

     /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void SetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount, Byte const* pNewData)
        {
        BeAssert(posX + pixelCount <= GetWidth());
        BeAssert(posY < GetHeight());

        uint32_t lineInPacket = 0;
        HCDPacketRLE& packet = GetPacketAt(lineInPacket, posY);

        size_t dataSize = MergeRuns(&m_workingRunsEdit[0], packet.GetLineBuffer(lineInPacket), packet.GetLineDataSize(lineInPacket), posX, posY, pixelCount, pNewData);

        // Copy runs to the packet.
        SetLine(packet, lineInPacket, &m_workingRunsEdit[0], dataSize);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte* LockPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount)
        {
        BeAssert(!m_lastLockRequest.IsLock());  // Unlock call is missing
        BeAssert(posX + pixelCount <= GetWidth());
        BeAssert(posY < GetHeight());

        uint32_t lineInPacket = 0;
        HCDPacketRLE& packet = GetPacketAt(lineInPacket, posY);
        
        m_lastLockRequest.Init(posX, posY, pixelCount);

        dataSize = ComputeWorkingCopy(&m_workingRunsQuery[0], posX, pixelCount, packet.GetLineBuffer(lineInPacket), packet.GetLineDataSize(lineInPacket));
        return &m_workingRunsQuery[0];
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void UnLockPixels()
        {
        BeAssert(m_lastLockRequest.IsLock());  // Lock call is missing or double unlock.

        uint32_t lineInPacket = 0;
        HCDPacketRLE& packet = GetPacketAt(lineInPacket, m_lastLockRequest.m_posY);

        size_t RunLenInBytes = MergeRuns(&m_workingRunsEdit[0], packet.GetLineBuffer(lineInPacket), packet.GetLineDataSize(lineInPacket), m_lastLockRequest.m_posX, m_lastLockRequest.m_posY, m_lastLockRequest.m_pixelCount, &m_workingRunsQuery[0]);
       
        // Copy runs to the packet.
        SetLine(packet, lineInPacket, &m_workingRunsEdit[0], RunLenInBytes);
 
        HDEBUGCODE(m_lastLockRequest.Reset());
        }  

private:

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void SetLine(HCDPacketRLE& packet, uint32_t line, Byte const* pLine, size_t dataSize)
        {
        BeAssert(packet.HasBufferOwnership());

        // Reuse current buffer if we can. Reallocating buffers is costly and fragment memory.
        // What we would need is a method that shrink packetRLE to data size or keep only a "reasonable"
        // amount of extra space or realloc to a certain threshold (ex. when we use less then 50% of the buffer)
        if (dataSize <= packet.GetLineBufferSize(line))
            {
            memcpy(packet.GetLineBuffer(line), pLine, dataSize);
            packet.SetLineDataSize(line, dataSize);
            }
        else
            {
            // no gain! ??
            //dataSize *= 1.2; // an extra 20% for future edition.
//             dataSize = MAX(dataSize*1.2, GetWorkingBufferSize()/3);
            Byte* pNewBuffer = new Byte[dataSize];
            memcpy(pNewBuffer, pLine, dataSize);
            packet.SetLineBuffer(line, pNewBuffer, dataSize, dataSize);
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* ImageEditorN1
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageEditorN1
{
public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageEditorN1(HRAPacketN1Surface& surface) : m_workingLinePosX(0), m_workingLinePosY(0), m_workingLinePixelCount(0), m_workingLineLocked(false)
        {
        BeAssert(surface.GetPixelType().CountPixelRawDataBits() % BITSPERPIXEL_N1 != 0);

        m_width = surface.GetWidth();
        m_height = surface.GetHeight();
        m_pData = surface.GetDataP(m_pitch);

        m_workingLine = new Byte[m_pitch];
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageEditorN1(HRAImageSampleR sample) : m_workingLinePosX(0), m_workingLinePosY(0), m_workingLinePixelCount(0), m_workingLineLocked(false)
        {
        BeAssert(sample.GetPixelType().CountPixelRawDataBits() % BITSPERPIXEL_N1 != 0);
        
        m_width = sample.GetWidth();
        m_height = sample.GetHeight();
        m_pData = sample.GetBufferP()->GetDataP(m_pitch);
        m_workingLine = new Byte[m_pitch];
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageEditorN1(HRASampleN1Surface& surface)
        :ImageEditorN1(surface.GetSampleR()) //delegating constructors
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ~ImageEditorN1()
        {
        delete[] m_workingLine;
        }

    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte const* GetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount)
        {
        BeAssert(posX + pixelCount <= m_width);
        BeAssert(posY < m_height);

        // return original data if possible
        if (0 == (posX % BITSPERPIXEL_N1))
            return m_pData + (posY*m_pitch) + (posX / BITSPERPIXEL_N1);

        CopyLineN1(m_workingLine, posX, pixelCount, (m_pData + posY*m_pitch));
        return m_workingLine;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte const* GetPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount)
        {
        dataSize = (pixelCount + BITSPERPIXEL_N1 - 1) / BITSPERPIXEL_N1;

        return GetPixels(posX, posY, pixelCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void SetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount, Byte const* pData)
        {
        BeAssert(posX + pixelCount <= m_width);
        BeAssert(posY < m_height);

        MergeLine(m_pData + posY*m_pitch, posX, pixelCount, pData);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void SetPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount, Byte const* pData, size_t dataSize)
        {
        BeAssert((pixelCount + BITSPERPIXEL_N1 - 1) / BITSPERPIXEL_N1 == dataSize);

        SetPixels(posX, posY, pixelCount, pData);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte* LockPixels(uint32_t posX, uint32_t posY, uint32_t pixelCount)
        {
        BeAssert(posX + pixelCount <= m_width);
        BeAssert(posY < m_height);

        // return original data if possible
        if (0 == posX && pixelCount == m_width)
            {
            m_workingLineLocked = false;
            return m_pData + (posY*m_pitch);
            }

        // ***** Begin Not-thread safe *****
        CopyLineN1(m_workingLine, posX, pixelCount, (m_pData + posY*m_pitch));
        m_workingLinePosX = posX;
        m_workingLinePosY = posY;
        m_workingLinePixelCount = pixelCount;
        m_workingLineLocked = true;
        return m_workingLine;
        // ***** Begin Not-thread safe ***** 
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline Byte* LockPixels(size_t& dataSize, uint32_t posX, uint32_t posY, uint32_t pixelCount)
        {
        dataSize = (pixelCount + BITSPERPIXEL_N1 - 1) / BITSPERPIXEL_N1;

        return LockPixels(posX, posY, pixelCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void UnLockPixels()
        {
        if (m_workingLineLocked)
            {
            MergeLine((m_pData + m_workingLinePosY*m_pitch), m_workingLinePosX, m_workingLinePixelCount, m_workingLine);
            m_workingLineLocked = false;
            }

        // nothing to do we returned the original pixels and they were edited directly.
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void UnLockPixels(size_t newDataSize)
        {
        BeAssert((m_workingLinePixelCount + BITSPERPIXEL_N1 - 1) / BITSPERPIXEL_N1 == newDataSize);
        UnLockPixels();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void Clear(uint32_t width, uint32_t height, Byte* pBuf, size_t pitch, bool state)
        {
        const size_t byteCount = width / BITSPERPIXEL_N1;
        const size_t remainder = width % BITSPERPIXEL_N1;

        if (state)
            {
            for (uint64_t line = 0; line < height; ++line)
                N1Manip::SetBitsOn((Byte*)pBuf + line*pitch, byteCount, remainder);
            }
        else
            {
            for (uint64_t line = 0; line < height; ++line)
                N1Manip::SetBitsOff((Byte*)pBuf + line*pitch, byteCount, remainder);
            }
        }

    size_t GetWorkingBufferSize() const { return m_pitch; }

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void MergeLine(Byte* pOutData, uint32_t outPosX, uint32_t pixelCount, Byte const* pInData) const
        {
        N1Manip::CopyBits(pOutData, outPosX, pixelCount, pInData, 0);
        }

private:

    Byte*  m_pData;
    Byte*  m_workingLine;
    size_t m_pitch;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_workingLinePosX;
    uint32_t m_workingLinePosY;
    uint32_t m_workingLinePixelCount;
    bool   m_workingLineLocked;
};

END_IMAGEPP_NAMESPACE
