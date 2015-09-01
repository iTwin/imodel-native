//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAEditorRLE1Line.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------------------
//:> Method for class HRAEditorRLE1Line
//:>---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAEditorRLE1Line.h>

#include <Imagepp/all/h/HGSMemoryRLESurfaceDescriptor.h>
#include <Imagepp/all/h/HCDPacketRLE.h>
#include <Imagepp/all/h/HRATransaction.h>

// We assume blacks are on 0 and whites on 1
#define ON_BLACK_STATE(bufferIndex) (!(bufferIndex & 0x00000001))       // block run is ON even number. 0,2,4,6...
#define RLE_RUN_LIMIT 32767

//:>---------------------------------------------------------------------------------------
//:> public section
//:>---------------------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-----------------------------------------------------------------------------*/
HRAEditorRLE1Line::HRAEditorRLE1Line(HGSMemoryRLESurfaceDescriptor& pi_rDescriptor)
    : HRAGenEditor()
    {
    HPRECONDITION(pi_rDescriptor.GetRLEPacket() != 0);
    //  HPRECONDITION(pi_rDescriptor.GetPacket()->GetCodec() != 0);
    //  HPRECONDITION(pi_rDescriptor.GetPacket()->GetCodec()->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID));
    HPRECONDITION(pi_rDescriptor.GetRLEPacket()->HasBufferOwnership());

    // compute some useful information
    m_Width         = pi_rDescriptor.GetWidth();
    m_Height        = pi_rDescriptor.GetHeight();
    pi_rDescriptor.GetOffsets(&m_XPosInRaster, &m_YPosInRaster);

    m_SLO4          = (pi_rDescriptor.GetSLO() == HGF_UPPER_LEFT_HORIZONTAL);
    m_pPacketRLE    = (HCDPacketRLE*)pi_rDescriptor.GetRLEPacket().GetPtr();
    m_Edited        = false;

    m_pTmpRun = new unsigned short[m_Width * 2 + 2]; // we allocate the worst case
    m_pWorkingRun = new unsigned short[m_Width * 2 + 2]; // we allocate the worst case
    m_pClearRun = new unsigned short[(((m_Width / RLE_RUN_LIMIT) + 1) * 2 + 1)]; // Worst case to generate a clear run
    m_pCurrentCount = 0;
    }

/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
HRAEditorRLE1Line::~HRAEditorRLE1Line()
    {
    }


/**----------------------------------------------------------------------------
 Get a specific run

 @param pi_StartPosX
 @param pi_StartPosY
 @param pi_PixelCount

 @return void*
-----------------------------------------------------------------------------*/
void* HRAEditorRLE1Line::GetRun(HUINTX  pi_StartPosX,
                                HUINTX  pi_StartPosY,
                                size_t  pi_PixelCount,
                                void*   pi_pTransaction) const
    {
    HPRECONDITION(pi_StartPosX + pi_PixelCount <= m_Width);
    HPRECONDITION(pi_StartPosY < m_Height);

    if (pi_PixelCount == 0)
        return 0;

    // get the address
    unsigned short PixelsToSkipInFirstLen;
    unsigned short PixelsToSkipInSecondLen;
    unsigned short* pRawData = ComputeAddress(pi_StartPosX,
                                       pi_StartPosY,
                                       &PixelsToSkipInFirstLen,
                                       &PixelsToSkipInSecondLen);

    if (pi_pTransaction != 0)
        {
        // write the whole line
        ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster,
                                                      m_YPosInRaster + pi_StartPosY,
                                                      m_Width,
                                                      1,
                                                      m_pPacketRLE->GetLineDataSize(pi_StartPosY),
                                                      m_pPacketRLE->GetLineBuffer(pi_StartPosY));
        }

    if (pi_StartPosX = 0 && pi_PixelCount == m_Width)
        return pRawData;

    uint64_t PixelCount = pi_PixelCount + PixelsToSkipInFirstLen + PixelsToSkipInSecondLen;
    uint64_t PixelCopied = 0;
    unsigned short* pEndRun = pRawData;

    while (PixelCopied < PixelCount)
        {
        PixelCopied += *pEndRun;
        pEndRun++;
        }
    size_t RunToCopy = pEndRun - pRawData;
    memcpy(m_pTmpRun, pRawData, RunToCopy * sizeof(unsigned short));

    // remove the pixels to skip
    *(m_pTmpRun)       -= PixelsToSkipInFirstLen;

    if (PixelsToSkipInSecondLen != 0)
        *(m_pTmpRun + 1)   -= PixelsToSkipInSecondLen;

    // adjust the last run
    if (PixelCopied > PixelCount)
        m_pTmpRun[RunToCopy - 1] -= (unsigned short)(PixelCopied - PixelCount);

    if ((RunToCopy & 0x01) == 0)
        m_pTmpRun[RunToCopy] = 0;

    return m_pTmpRun;
    }

//-----------------------------------------------------------------------------
// public
// SetRun
//-----------------------------------------------------------------------------
void HRAEditorRLE1Line::SetRun(HUINTX       pi_StartPosX,
                               HUINTX       pi_StartPosY,
                               size_t       pi_PixelCount,
                               const void*  pi_pRun,
                               void*        pi_pTransaction)
    {
    HPRECONDITION(pi_StartPosX >= 0L);
    HPRECONDITION(pi_StartPosY >= 0L);
    HPRECONDITION(pi_StartPosX < m_Width);
    HPRECONDITION(pi_StartPosY < m_Height);
    HPRECONDITION(pi_PixelCount > 0);
    HPRECONDITION(pi_StartPosX + pi_PixelCount <= m_Width);
    HPRECONDITION(m_pPacketRLE->GetLineBuffer(pi_StartPosY) != 0);

    unsigned short* pCurrentRun = (unsigned short*)m_pPacketRLE->GetLineBuffer(pi_StartPosY);
    uint32_t CurrentRunIndex = 0;
    size_t   PixelsFromCurrentRun = 0;
    uint32_t WorkRunIndex = 0;

    if (pi_pTransaction)
        {
        // record the line before editing it
        // write the whole line
        ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster,
                                                      m_YPosInRaster + pi_StartPosY,
                                                      m_Width,
                                                      1,
                                                      m_pPacketRLE->GetLineDataSize(pi_StartPosY),
                                                      m_pPacketRLE->GetLineBuffer(pi_StartPosY));

        }

    // copy the first segment from the run into the new run
    // N.B. its faster, by about 20%, to compute index and do a single memcpy.
    while (PixelsFromCurrentRun < pi_StartPosX)
        {
        PixelsFromCurrentRun += pCurrentRun[CurrentRunIndex];
        ++CurrentRunIndex;
        }
    memcpy(m_pWorkingRun, pCurrentRun, CurrentRunIndex*sizeof(unsigned short));
    WorkRunIndex=CurrentRunIndex;

    size_t   PixelsFromNewRun = 0;
    uint32_t NewRunIndex = 0;
    const unsigned short* pNewRun = (const unsigned short*)pi_pRun;

    // Skip empty runs.
    while(pNewRun[NewRunIndex] == 0)
        ++NewRunIndex;

    // cut previous run. So m_pWorkingRun pos == pi_StartPosX
    if (PixelsFromCurrentRun > pi_StartPosX)
        m_pWorkingRun[WorkRunIndex-1] -= (unsigned short)(PixelsFromCurrentRun - pi_StartPosX);

    // If we are not on the same state we must adjust with previous.
    if(ON_BLACK_STATE(NewRunIndex) != ON_BLACK_STATE(WorkRunIndex))
        {
        if(0 != WorkRunIndex)
            {
            size_t ToWrite = m_pWorkingRun[WorkRunIndex-1] + pNewRun[NewRunIndex];

            while(ToWrite > RLE_RUN_LIMIT)
                {
                m_pWorkingRun[WorkRunIndex-1] = RLE_RUN_LIMIT;
                m_pWorkingRun[WorkRunIndex] = 0;
                ToWrite -= RLE_RUN_LIMIT;
                WorkRunIndex+=2;
                }
            m_pWorkingRun[WorkRunIndex-1] = (unsigned short)ToWrite;

            // Append to previous run
            PixelsFromNewRun = pNewRun[NewRunIndex];
            ++NewRunIndex;
            }
        else
            {
            m_pWorkingRun[0] = 0;
            WorkRunIndex = 1;
            }
        }

    HASSERT(ON_BLACK_STATE(NewRunIndex) == ON_BLACK_STATE(WorkRunIndex));

    // Copy the new run.
    int32_t RunIndexStart = NewRunIndex;
    while (PixelsFromNewRun < pi_PixelCount)
        {
        PixelsFromNewRun += pNewRun[NewRunIndex];
        ++NewRunIndex;
        }
    memcpy(&m_pWorkingRun[WorkRunIndex], &pNewRun[RunIndexStart], (NewRunIndex - RunIndexStart)*sizeof(unsigned short));
    WorkRunIndex += (NewRunIndex - RunIndexStart);

    // I don't know if that can happen but adjust last run in case where pi_pRun encoded more that pi_PixelCount
    if(PixelsFromNewRun > pi_PixelCount)
        {
        HPOSTCONDITION(!"Number of pixels in pi_pRun is NOT equal to pi_PixelCount");

        // cut previous run.
        m_pWorkingRun[WorkRunIndex-1] -= (unsigned short)(PixelsFromNewRun - pi_PixelCount);
        PixelsFromNewRun = pi_PixelCount;
        }

    // If the new run covered till the end of the line there is nothing else to copy.
    if(pi_StartPosX + PixelsFromNewRun < m_Width)
        {
        // Goto end merge position in current run.
        size_t skipTo = pi_StartPosX + pi_PixelCount;
        while (PixelsFromCurrentRun < skipTo)
            {
            PixelsFromCurrentRun += pCurrentRun[CurrentRunIndex];
            ++CurrentRunIndex;
            }

        // cut previous run. So pCurrentRun pos == pi_StartPosX + pi_PixelCount
        if (PixelsFromCurrentRun > skipTo)
            pCurrentRun[--CurrentRunIndex] = (unsigned short)(PixelsFromCurrentRun - skipTo);

        size_t currentPos = skipTo;

        // If we are not on the same state we must adjust with previous.
        if(ON_BLACK_STATE(CurrentRunIndex) != ON_BLACK_STATE(WorkRunIndex))
            {
            size_t ToWrite = m_pWorkingRun[WorkRunIndex-1] + pCurrentRun[CurrentRunIndex];

            while(ToWrite > RLE_RUN_LIMIT)
                {
                m_pWorkingRun[WorkRunIndex-1] = RLE_RUN_LIMIT;
                m_pWorkingRun[WorkRunIndex] = 0;
                ToWrite -= RLE_RUN_LIMIT;
                WorkRunIndex+=2;
                }
            m_pWorkingRun[WorkRunIndex-1] = (unsigned short)ToWrite;

            // Append to previous run
            currentPos += pCurrentRun[CurrentRunIndex];
            ++CurrentRunIndex;
            }

        // We should be able to copy all the remaining runs without counting but for extra precaution we count them.
        int32_t CurrentRunIndexStart = CurrentRunIndex;
        while(currentPos < m_Width)
            {
            currentPos += pCurrentRun[CurrentRunIndex];
            ++CurrentRunIndex;
            }
        memcpy(&m_pWorkingRun[WorkRunIndex], &pCurrentRun[CurrentRunIndexStart], (CurrentRunIndex - CurrentRunIndexStart)*sizeof(unsigned short));
        WorkRunIndex += (CurrentRunIndex - CurrentRunIndexStart);

        HPOSTCONDITION(currentPos == m_Width);

        // Make sure we have the right number of pixels.
        if (currentPos > m_Width)
            m_pWorkingRun[WorkRunIndex-1] -= (unsigned short)(currentPos - m_Width);
        }

    // Must finish on black
    if(ON_BLACK_STATE(WorkRunIndex))
        {
        m_pWorkingRun[WorkRunIndex] = 0;
        ++WorkRunIndex;
        }

    // Copy runs to real buffer.
    size_t RunLenInBytes = WorkRunIndex * sizeof(unsigned short);

    // Reuse current buffer if we can. Reallocating buffers is costly and fragment memory.
    // What we would need is a method that shrink packetRLE to data size or keep only a "reasonable"
    // amount of extra space or realloc to a certain threshold (ex. when we use less then 50% of the buffer)
    if(RunLenInBytes <= m_pPacketRLE->GetLineBufferSize(pi_StartPosY))
        {
        memcpy(pCurrentRun, m_pWorkingRun, RunLenInBytes);
        m_pPacketRLE->SetLineDataSize(pi_StartPosY, RunLenInBytes);
        }
    else
        {
        Byte* pNewBuffer = new Byte[RunLenInBytes];
        memcpy(pNewBuffer, m_pWorkingRun, RunLenInBytes);
        m_pPacketRLE->SetLineBuffer(pi_StartPosY, pNewBuffer, RunLenInBytes, RunLenInBytes);
        }

    m_Edited = true;
    }

/**----------------------------------------------------------------------------
 Get a specific pixel

 @param pi_PosX
 @param pi_PosY

 @return void*
-----------------------------------------------------------------------------*/
void* HRAEditorRLE1Line::GetPixel(HUINTX    pi_PosX,
                                  HUINTX    pi_PosY) const
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);

    bool OnState = false;
    m_CurrentLine = pi_PosY;

    // get the address of the line
    HPRECONDITION(m_pPacketRLE->GetLineBuffer(pi_PosY) != 0);
    m_pCurrentCount = (unsigned short*)m_pPacketRLE->GetLineBuffer(pi_PosY);

    // parse all the pixels
    while (pi_PosX > *m_pCurrentCount || *m_pCurrentCount == 0)
        {
        pi_PosX -= *m_pCurrentCount;
        OnState = !OnState;
        m_pCurrentCount++;
        HPOSTCONDITION(m_pCurrentCount <= (unsigned short*)(m_pPacketRLE->GetLineBuffer(pi_PosY) + m_pPacketRLE->GetLineDataSize(pi_PosY)));
        }

    HPRECONDITION(*m_pCurrentCount >= pi_PosX);
    m_RemainingCount = (unsigned short)(*m_pCurrentCount - pi_PosX);

    // compute the address
    if(OnState)
        {
        m_aData[0] = 0;
        m_aData[1] = 1;
        }
    else
        {
        m_aData[0] = 1;
        m_aData[1] = 0;
        }

    return m_aData;
    }

/**----------------------------------------------------------------------------
 Get the next pixel of the current position

 @note GetPixel() must have been called before this method.

 @return void*
-----------------------------------------------------------------------------*/
void* HRAEditorRLE1Line::GetNextPixel() const
    {
    HPRECONDITION(m_pCurrentCount != 0);

    // Attention: GetPixel() must have been called before this method.

    void* pData;
    if (m_pCurrentCount == 0)
        {
        if (m_CurrentLine == m_Height - 1)
            pData = 0;
        else
            {
            // GetPixel() will record the whole line.
            pData = GetPixel(0, m_CurrentLine + 1);
            }
        }
    else
        {
        if (m_RemainingCount == 0)
            {
            // Fetch the next non-zero run count
            ++m_pCurrentCount;
            bool SameState = false;
            HPOSTCONDITION(m_pCurrentCount <= (unsigned short*)(m_pPacketRLE->GetLineBuffer(m_CurrentLine) + m_pPacketRLE->GetLineDataSize(m_CurrentLine)));

            while (*m_pCurrentCount == 0)
                {
                SameState = !SameState;
                ++m_pCurrentCount;
                HPOSTCONDITION(m_pCurrentCount <= (unsigned short*)(m_pPacketRLE->GetLineBuffer(m_CurrentLine) + m_pPacketRLE->GetLineDataSize(m_CurrentLine)));
                }

            m_RemainingCount = *m_pCurrentCount - 1;

            // Alternate the color (we changed run count)
            if (!SameState)
                {
                if (m_aData[0] == 0)
                    {
                    m_aData[0] = 1;
                    m_aData[1] = 0;
                    }
                else
                    {
                    m_aData[0] = 0;
                    m_aData[1] = 1;
                    }
                }
            }
        else
            {
            --m_RemainingCount;
            }
        pData = m_aData;
        }
    return pData;
    }

/**----------------------------------------------------------------------------
 Set a specific pixel value

 @param pi_PosX
 @param pi_PosY
 @param pi_pValue
-----------------------------------------------------------------------------*/
void HRAEditorRLE1Line::SetPixel(HUINTX       pi_PosX,
                                 HUINTX       pi_PosY,
                                 const void*  pi_pValue)
    {
    // not yet supported
    HASSERT(0);
    }

/**----------------------------------------------------------------------------
 Get pixels at specific positions

 @param pi_pPositionsX
 @param pi_pPositionsY
 @param pi_PixelCount
 @param po_pBuffer
-----------------------------------------------------------------------------*/
void HRAEditorRLE1Line::GetPixels(const HUINTX* pi_pPositionsX,
                                  const HUINTX* pi_pPositionsY,
                                  size_t        pi_PixelCount,
                                  void*         po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(pi_PixelCount > 0);
    HPRECONDITION(po_pBuffer != 0);


    // get the individual pixels
    unsigned short* pBuffer = (unsigned short*)po_pBuffer;
    bool OnMode = false;
    *pBuffer = 0;
    for(uint32_t PixelIndex = 0; PixelIndex < pi_PixelCount; PixelIndex++)
        {
        HPOSTCONDITION(pi_pPositionsX[PixelIndex] < m_Width);
        HPOSTCONDITION(pi_pPositionsY[PixelIndex] < m_Height);

        // test if the pixel has the same state as the current state
        if (IsPixelOn(pi_pPositionsX[PixelIndex], pi_pPositionsY[PixelIndex]) == OnMode)
            {
            // if yes, test if we reach the maximum
            if (*pBuffer == RLE_RUN_LIMIT)
                {
                // if yes, go two runs later
                pBuffer++;
                *pBuffer = 0;
                pBuffer++;
                *pBuffer = 1;
                }
            else
                {
                // increment the current run
                (*pBuffer)++;
                }
            }
        else
            {
            // if not, go to the next run
            OnMode = !OnMode;
            pBuffer++;
            *pBuffer = 1;
            }
        }
    }

/**----------------------------------------------------------------------------
 Get pixels at specific positions

 @param pi_PosX
 @param pi_PosY
 @param pi_DeltaX
 @param pi_DeltaY
 @param pi_PixelCount
 @param po_pBuffer
-----------------------------------------------------------------------------*/
void HRAEditorRLE1Line::GetPixels(HUINTX    pi_PosX,
                                  HUINTX    pi_PosY,
                                  HSINTX    pi_DeltaX,
                                  HSINTX    pi_DeltaY,
                                  size_t    pi_PixelCount,
                                  void*     po_pBuffer) const
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);
    HPRECONDITION(pi_PixelCount > 0);
    HPRECONDITION(po_pBuffer != 0);

    unsigned short* pBuffer = (unsigned short*)po_pBuffer;
    bool OnMode = false;
    *pBuffer = 0;
    if (pi_DeltaX == 1 && pi_DeltaY == 0)
        {
        HPRECONDITION(pi_PosX + pi_PixelCount <= m_Width);

        GetRun(pi_PosX, pi_PosY, pi_PixelCount, (unsigned short*)po_pBuffer);
        }
    else
        {
        while (pi_PixelCount > 0)
            {
            HPRECONDITION(pi_PosX < m_Width);
            HPRECONDITION(pi_PosY < m_Height);

            // test if the pixel has the same state as the current state
            if (IsPixelOn(pi_PosX, pi_PosY) == OnMode)
                {
                // yes, test if we reach the maximum
                if (*pBuffer == RLE_RUN_LIMIT)
                    {
                    // yes, go two run later
                    pBuffer++;
                    *pBuffer = 0;
                    pBuffer++;
                    *pBuffer = 1;
                    }
                else
                    {
                    // increment the current run
                    (*pBuffer)++;
                    }
                }
            else
                {
                // if not, go to the next run
                OnMode = !OnMode;
                pBuffer++;
                *pBuffer = 1;
                }

            --pi_PixelCount;
            pi_PosX += pi_DeltaX;
            pi_PosY += pi_DeltaY;
            }
        }
    }


/**----------------------------------------------------------------------------
 Clear the surface

 @param pi_pValue
-----------------------------------------------------------------------------*/
void HRAEditorRLE1Line::Clear(const void*  pi_pValue,
                              void*        pi_pTransaction)
    {
    HPRECONDITION(pi_pValue != 0);
    HPRECONDITION(*((Byte*)pi_pValue) == 1 || (*((Byte*)pi_pValue) == 0));

    m_Edited = true;

    bool ClearBlack = *((Byte*)pi_pValue) == 0 ? true : false;

    size_t RunsPerLine = (m_Width / RLE_RUN_LIMIT) * 2 + 1;

    if (!ClearBlack)
        RunsPerLine += 2; // we clear in white, add 2 bytes because we need to end with black

    HArrayAutoPtr<unsigned short> pTemplateLine(new unsigned short[RunsPerLine]);
    uint32_t PixelCount = m_Width;

    if(ClearBlack)
        {
        unsigned short* pTLineItr = pTemplateLine;
        for(; PixelCount > RLE_RUN_LIMIT; PixelCount-=RLE_RUN_LIMIT)
            {
            *pTLineItr++ = RLE_RUN_LIMIT;
            *pTLineItr++ = 0;
            }
        *pTLineItr = (unsigned short)PixelCount;    // remaining blacks.
        }
    else
        {
        unsigned short* pTLineItr = pTemplateLine;
        for(; PixelCount > RLE_RUN_LIMIT; PixelCount-=RLE_RUN_LIMIT)
            {
            *pTLineItr++ = 0;
            *pTLineItr++ = RLE_RUN_LIMIT;
            }
        *pTLineItr++ = 0;
        *pTLineItr++ = (unsigned short)PixelCount;    // remaining whites.
        *pTLineItr = 0;
        }

    for(uint32_t i=0; i < m_Height; ++i)
        {
        if (pi_pTransaction)
            {
            // write the whole line
            ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster,
                                                          m_YPosInRaster + i,
                                                          m_Width,
                                                          1,
                                                          m_pPacketRLE->GetLineDataSize(i),
                                                          m_pPacketRLE->GetLineBuffer(i));
            }

        if(m_pPacketRLE->GetLineDataSize(i) > RunsPerLine*sizeof(unsigned short))
            {
            memcpy(m_pPacketRLE->GetLineBuffer(i), pTemplateLine, RunsPerLine*sizeof(unsigned short));
            m_pPacketRLE->SetLineDataSize(i, RunsPerLine*sizeof(unsigned short));
            }
        else
            {
            unsigned short* pLine = new unsigned short[RunsPerLine];
            memcpy(pLine, pTemplateLine, RunsPerLine*sizeof(unsigned short));
            m_pPacketRLE->SetLineBuffer(i, (Byte*)pLine, RunsPerLine*sizeof(unsigned short), RunsPerLine*sizeof(unsigned short));
            }
        }
    }


/**----------------------------------------------------------------------------
 Clear a specific run

 @param pi_PosX
 @param pi_PosY
 @param pi_PixelCount
 @param pi_pValue
-----------------------------------------------------------------------------*/
void HRAEditorRLE1Line::ClearRun(HUINTX       pi_PosX,
                                 HUINTX       pi_PosY,
                                 size_t       pi_PixelCount,
                                 const void*  pi_pValue,
                                 void*        pi_pTransaction)
    {
    HPRECONDITION(pi_pValue != 0);
    HPRECONDITION(pi_PosX + pi_PixelCount <= m_Width);

    unsigned short* pRun = m_pClearRun;
    uint32_t WorkRunIndex=0;
    size_t   WorkPixelCount = pi_PixelCount;

    if (*((Byte*)pi_pValue) == 1) // clear with the On
        {
        pRun[WorkRunIndex] = 0;
        ++WorkRunIndex; // skip the off value

        while(WorkPixelCount > RLE_RUN_LIMIT)
            {
            pRun[WorkRunIndex] = RLE_RUN_LIMIT;
            pRun[WorkRunIndex+1] = 0;
            WorkPixelCount -= RLE_RUN_LIMIT;
            WorkRunIndex+=2;
            }
        pRun[WorkRunIndex] = (unsigned short)WorkPixelCount;
        pRun[WorkRunIndex+1] = 0;   // Must end with black
        }
    else
        {
        while(WorkPixelCount > RLE_RUN_LIMIT)
            {
            pRun[WorkRunIndex] = RLE_RUN_LIMIT;
            pRun[WorkRunIndex+1] = 0;
            WorkPixelCount -= RLE_RUN_LIMIT;
            WorkRunIndex+=2;
            }
        pRun[WorkRunIndex] = (unsigned short)WorkPixelCount;
        }


    SetRun(pi_PosX,
           pi_PosY,
           pi_PixelCount,
           pRun,
           pi_pTransaction);
    }


/**----------------------------------------------------------------------------
Merge run

@param pi_PosX
@param pi_PosY
@param pi_PixelCount
@param pi_pValue
@param pi_pTransaction

Note : The position must be relative to the image
-----------------------------------------------------------------------------*/
void HRAEditorRLE1Line::MergeRuns(HUINTX      pi_StartPosX,
                                  HUINTX      pi_StartPosY,
                                  size_t      pi_Width,
                                  size_t      pi_Height,
                                  const void* pi_pRun,
                                  void*       pi_pTransaction)
    {
    HPRECONDITION(pi_StartPosX == 0);
    HPRECONDITION(pi_Width == m_Width);
    HPRECONDITION(pi_Height == 1);

    // check if the pi_pRun replace the whole line
    if (pi_StartPosX == m_XPosInRaster && pi_Width == m_Width)
        {
        // compute the size, in byte, of the run
        const unsigned short* pRun((unsigned short*)pi_pRun);
        uint32_t PixelCount = 0;
        while (PixelCount < pi_Width)
            PixelCount += *pRun++;
        HPOSTCONDITION(PixelCount == pi_Width);

        if (ON_BLACK_STATE(pRun - (unsigned short*)pi_pRun))
            {
            HPRECONDITION(*pRun == 0);
            ++pRun;
            }


        if (pi_pTransaction)
            {
            // record the line before editing it
            // write the whole line
            ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster,
                                                          pi_StartPosY,
                                                          m_Width,
                                                          1,
                                                          m_pPacketRLE->GetLineDataSize(pi_StartPosY - m_YPosInRaster),
                                                          m_pPacketRLE->GetLineBuffer(pi_StartPosY - m_YPosInRaster));
            }

        size_t RunLen((pRun - (unsigned short*)pi_pRun) * sizeof(unsigned short));
        m_pPacketRLE->SetLineData(pi_StartPosY - m_YPosInRaster,
                                  (Byte*)pi_pRun,
                                  RunLen);

        m_Edited = true;
        }
    else
        SetRun(0,
               pi_StartPosY - m_YPosInRaster,
               m_Width,
               pi_pRun,
               pi_pTransaction);
    }


//:>---------------------------------------------------------------------------------------
//:> protected section
//:>---------------------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Compute address of a specific pixel

 @return UShort*
-----------------------------------------------------------------------------*/
unsigned short* HRAEditorRLE1Line::ComputeAddress(HUINTX   pi_PosX,
                                           HUINTX   pi_PosY,
                                           unsigned short* po_pPixelsToSkipInFirstLen,
                                           unsigned short* po_pPixelsToSkipInSecondLen) const
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);
    HPRECONDITION(po_pPixelsToSkipInFirstLen != 0);
    HPRECONDITION(po_pPixelsToSkipInSecondLen != 0);

    // pointer at the beginning of a line
    unsigned short* pOffRun = (unsigned short*)m_pPacketRLE->GetLineBuffer(pi_PosY);

    HUINTX PixelsFromRun = pi_PosX;

    bool PixelFound = false;

    while (!PixelFound)
        {
        if (PixelsFromRun < *pOffRun)
            {
            *po_pPixelsToSkipInFirstLen   = (unsigned short)PixelsFromRun;
            *po_pPixelsToSkipInSecondLen  = 0;

            PixelFound = true;
            }
        else if (PixelsFromRun < (uint32_t)(*pOffRun + *(pOffRun + 1)))
            {
            *po_pPixelsToSkipInFirstLen   = *pOffRun;
            *po_pPixelsToSkipInSecondLen  = (unsigned short)(PixelsFromRun - *pOffRun);

            PixelFound = true;
            }
        else
            {
            PixelsFromRun -= (*pOffRun + *(pOffRun + 1));
            pOffRun += 2;
            }
        }

    //:> This post condition is use to verify if we read a initialized data
    //:> call Clear() to initialize the buffer
    HPOSTCONDITION(pOffRun <= (unsigned short*)(m_pPacketRLE->GetLineBuffer(pi_PosY) + m_pPacketRLE->GetLineDataSize(pi_PosY)));
    return pOffRun;
    }



//:>---------------------------------------------------------------------------------------
//:> private section
//:>---------------------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Get the next pixel of the current position

 @return void*
-----------------------------------------------------------------------------*/
void HRAEditorRLE1Line::GetRun(HUINTX   pi_StartPosX,
                               HUINTX   pi_StartPosY,
                               size_t   pi_PixelCount,
                               unsigned short* po_pBuffer) const
    {
    HPRECONDITION(pi_StartPosX < m_Width);
    HPRECONDITION(pi_StartPosY < m_Height);
    HPRECONDITION(pi_PixelCount > 0);
    HPRECONDITION(pi_StartPosX + pi_PixelCount <= m_Width);
    HPRECONDITION(po_pBuffer != 0);

    // get the address
    unsigned short PixelsToSkipInFirstLen;
    unsigned short PixelsToSkipInSecondLen;
    unsigned short* pRawData = ComputeAddress((uint32_t)pi_StartPosX,
                                       (uint32_t)pi_StartPosY,
                                       &PixelsToSkipInFirstLen,
                                       &PixelsToSkipInSecondLen);

    int32_t PixelCount = 0 - (PixelsToSkipInFirstLen + PixelsToSkipInSecondLen);
    unsigned short* pEndRun = pRawData;

    while (PixelCount < (int32_t)pi_PixelCount)
        {
        PixelCount += *pEndRun;
        pEndRun++;
        }
    memcpy(po_pBuffer, pRawData, (pEndRun - pRawData) * sizeof(unsigned short));

    // remove the pixels to skip
    *(po_pBuffer)       -= PixelsToSkipInFirstLen;
    *(po_pBuffer + 1)   -= PixelsToSkipInSecondLen;
    }

/**----------------------------------------------------------------------------
 Check if a specific pixel is on or off

 @param pi_PosX
 @param pi_PosY

 @return bool true if the pixel is on, false if the pixel is off
-----------------------------------------------------------------------------*/
bool HRAEditorRLE1Line::IsPixelOn(HUINTX pi_PosX, HUINTX pi_PosY) const
    {
    bool OnState = false;

    // get the address of the line
    unsigned short* pRun = (unsigned short*)m_pPacketRLE->GetLineBuffer(pi_PosY);
    HPOSTCONDITION(pRun < (unsigned short*)(m_pPacketRLE->GetLineBuffer(pi_PosY) + m_pPacketRLE->GetLineDataSize(pi_PosY)));

    // parse all the pixels
    while(pi_PosX >= *pRun)
        {
        pi_PosX -= *pRun;

        OnState = !OnState;

        pRun++;
        }

    return OnState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool    HRAEditorRLE1Line::HasData() const
    {
    return m_pPacketRLE->GetDataSize() != 0;
    }