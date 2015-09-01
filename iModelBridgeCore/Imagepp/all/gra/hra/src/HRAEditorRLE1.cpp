//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAEditorRLE1.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------------------
//:> Method for class HRAEditorRLE1
//:>---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAEditorRLE1.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDPacket.h>



//:>---------------------------------------------------------------------------------------
//:> public section
//:>---------------------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-----------------------------------------------------------------------------*/
HRAEditorRLE1::HRAEditorRLE1(HGSMemorySurfaceDescriptor& pi_rDescriptor)
    : HRAGenEditor()
    {
    HPRECONDITION(pi_rDescriptor.GetPacket() != 0);
    HPRECONDITION(pi_rDescriptor.GetPacket()->GetCodec() != 0);
    HPRECONDITION(pi_rDescriptor.GetPacket()->GetCodec()->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID));

    // compute some useful information
    m_Width         = pi_rDescriptor.GetWidth();
    m_Height        = pi_rDescriptor.GetHeight();
    m_SLO4          = (pi_rDescriptor.GetSLO() == HGF_UPPER_LEFT_HORIZONTAL);
    m_pPacket       = pi_rDescriptor.GetPacket();
    m_Edited        = false;

    // take a copy of the line indexes table
    HFCPtr<HCDCodecHMRRLE1> pCodec = (const HFCPtr<HCDCodecHMRRLE1>&)pi_rDescriptor.GetPacket()->GetCodec();
    HPRECONDITION(pCodec->HasLineIndexesTable());
    m_pLineIndexes  = pCodec->GetLineIndexesTable();

    m_pTmpRun = new unsigned short[m_Width * 2 + 2]; // we allocate the worst case
    m_pWorkingRun = new unsigned short[m_Width * 2 + 2]; // we allocate the worst case
    m_pCurrentCount = 0;
    }

/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
HRAEditorRLE1::~HRAEditorRLE1()
    {
    if (m_Edited)
        {
        // alloc a new buffer
        Byte* pNewBuffer = new Byte[m_pPacket->GetDataSize()];

        unsigned short* pNewLine = (unsigned short*)pNewBuffer;
        unsigned short* pLinePtr;
        unsigned short* pRun;
        uint32_t PixelCount;
        bool OnState;
        size_t LineSize;
        size_t DataSize = 0;
        unsigned short* pBuffer = (unsigned short*)m_pPacket->GetBufferAddress();

        for (uint32_t i = 0; i < m_Height; i++)
            {
            OnState = false;

            // get the address of the line
            pLinePtr = &pBuffer[m_pLineIndexes[i]];
            pRun = pLinePtr;
            PixelCount = 0;
            LineSize = 0;
            while (PixelCount < m_Width)
                {
                PixelCount += *pRun;
                pRun++;
                OnState = !OnState;
                }

            // if the OnState == false, the last run written was OnState (1),
            // we must end with 0
            if (!OnState)
                pRun++;

            LineSize = pRun - pLinePtr;

            // write into the new buffer
            memcpy(pNewLine, pLinePtr, LineSize * sizeof(unsigned short));

            DataSize += LineSize;

            // set the new index for this line
            HASSERT_X64(pNewLine - (unsigned short*)pNewBuffer < ULONG_MAX);
            m_pLineIndexes[i] = (uint32_t)(pNewLine - (unsigned short*)pNewBuffer);
            pNewLine += LineSize;
            }

        // set packet
        m_pPacket->SetBuffer(pNewBuffer, m_pPacket->GetDataSize());
        m_pPacket->SetBufferOwnership(true);
        m_pPacket->SetDataSize(DataSize * sizeof(unsigned short));
        m_pPacket->ShrinkBufferToDataSize();
        }
    }


/**----------------------------------------------------------------------------
 Get a specific run

 @param pi_StartPosX
 @param pi_StartPosY
 @param pi_PixelCount

 @return void*
-----------------------------------------------------------------------------*/
void* HRAEditorRLE1::GetRun(HUINTX  pi_StartPosX,
                            HUINTX  pi_StartPosY,
                            size_t  pi_PixelCount,
                            void*   pi_pTransaction) const
    {
    HPRECONDITION(pi_StartPosX + pi_PixelCount <= m_Width);
    HPRECONDITION(pi_StartPosY < m_Height);
    HPRECONDITION(pi_pTransaction == 0);

    if (pi_PixelCount == 0)
        return 0;

    // get the address
    unsigned short PixelsToSkipInFirstLen;
    unsigned short PixelsToSkipInSecondLen;
    unsigned short* pRawData = ComputeAddress(pi_StartPosX,
                                       pi_StartPosY,
                                       &PixelsToSkipInFirstLen,
                                       &PixelsToSkipInSecondLen);

    if (PixelsToSkipInFirstLen == 0 && PixelsToSkipInSecondLen == 0 && pi_PixelCount == m_Width)
        return pRawData;

    int32_t PixelCount = 0 - (PixelsToSkipInFirstLen + PixelsToSkipInSecondLen);
    unsigned short* pEndRun = pRawData;

    while (PixelCount < (int32_t)pi_PixelCount)
        {
        PixelCount += *pEndRun;
        pEndRun++;
        }
    memcpy(m_pTmpRun, pRawData, (pEndRun - pRawData) * sizeof(unsigned short));

    // remove the pixels to skip
    *(m_pTmpRun)       -= PixelsToSkipInFirstLen;
    *(m_pTmpRun + 1)   -= PixelsToSkipInSecondLen;

    return m_pTmpRun;
    }


//-----------------------------------------------------------------------------
// public
// SetRun
//-----------------------------------------------------------------------------
void HRAEditorRLE1::SetRun(HUINTX       pi_StartPosX,
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
    HPRECONDITION(pi_pTransaction == 0);

    unsigned short* pRun = (unsigned short*)pi_pRun;

    HPRECONDITION(m_pPacket->GetBufferAddress() != 0);
    unsigned short* pCurrentRun = &((unsigned short*)m_pPacket->GetBufferAddress())[m_pLineIndexes[pi_StartPosY]];
    HPOSTCONDITION(pCurrentRun <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize()));

    bool CurrentRunState = false;

    uint32_t PixelsFromRun = 0;

    unsigned short* pNewRun = (unsigned short*)m_pWorkingRun.get();
    bool NewRunState = false;

    // copy the first segment from the run into the new run
    while (PixelsFromRun < pi_StartPosX)
        {
        PixelsFromRun += *pCurrentRun;
        *pNewRun = *pCurrentRun;
        NewRunState = !NewRunState;
        pCurrentRun++;
        pNewRun++;
        HPOSTCONDITION(pCurrentRun <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize()));
        }

    CurrentRunState = NewRunState;

    // adjust the last entry
    if (PixelsFromRun > pi_StartPosX)
        *(pNewRun - 1) -= (unsigned short)(PixelsFromRun - pi_StartPosX);

    // the pi_pRun start with the OFF run
    if (NewRunState)
        {
        *pNewRun = 0;
        pNewRun++;
        NewRunState = !NewRunState;
        }

    // insert the run
    uint32_t PixelCount = 0;
    while (PixelCount < pi_PixelCount)
        {
        *pNewRun = *pRun;
        PixelCount += *pNewRun;
        pNewRun++;
        pRun++;
        NewRunState = !NewRunState;
        }

    // adjust the last entry
    if (PixelCount > pi_PixelCount)
        *(pNewRun - 1) -= (unsigned short)(PixelCount - pi_PixelCount);

    // complete the run
    size_t RunLen = pi_StartPosX + pi_PixelCount;
    while (PixelsFromRun < RunLen)
        {
        PixelsFromRun += *pCurrentRun;
        pCurrentRun++;
        CurrentRunState = !CurrentRunState;
        HPOSTCONDITION(pCurrentRun <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize()));
        }

    // if we are not in the same state, change the state
    if (PixelsFromRun > RunLen)
        {
        if (CurrentRunState == NewRunState)
            {
            *pNewRun = 0;
            pNewRun++;
            *pNewRun = (unsigned short)(PixelsFromRun - RunLen);
            RunLen += *pNewRun;
            pNewRun++;
            NewRunState = !NewRunState;
            }
        else
            {
            *pNewRun = (unsigned short)(PixelsFromRun - RunLen);
            RunLen += *pNewRun;
            pNewRun++;
            }
        }
    else
        {
        if (CurrentRunState != NewRunState)
            {
            *pNewRun = 0;
            pNewRun++;
            NewRunState = !NewRunState;
            }
        }

    // now, complete with the run
    while (RunLen < m_Width)
        {
        *pNewRun = *pCurrentRun;
        RunLen += *pNewRun;
        pNewRun++;
        pCurrentRun++;
        NewRunState = !NewRunState;
        HPOSTCONDITION(pCurrentRun <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize()));
        }

    // we must end with the OFF run
    if (!NewRunState)
        {
        *pNewRun = 0;
        pNewRun++;
        }

    *(pNewRun-1) -= (unsigned short)(RunLen - m_Width);
    HPOSTCONDITION(RunLen == m_Width);

    // now, add the run at the end of the buffer
    size_t RunLenInByte = (pNewRun - m_pWorkingRun.get()) * sizeof(unsigned short);
    unsigned short* pNewOffset = PrepareToAppendDataInBuffer(RunLenInByte);
    memcpy(pNewOffset, m_pWorkingRun.get(), RunLenInByte);

    // now set the new buffer size
    m_pPacket->SetDataSize(m_pPacket->GetDataSize() + RunLenInByte);

    // update the new offset
    HASSERT_X64(pNewOffset - (unsigned short*)(m_pPacket->GetBufferAddress()) < ULONG_MAX);
    m_pLineIndexes[pi_StartPosY] = (uint32_t)(pNewOffset - (unsigned short*)(m_pPacket->GetBufferAddress()));

    m_Edited = true;
    }


/**----------------------------------------------------------------------------
 Get a specific pixel

 @param pi_PosX
 @param pi_PosY

 @return void*
-----------------------------------------------------------------------------*/
void* HRAEditorRLE1::GetPixel(HUINTX    pi_PosX,
                              HUINTX    pi_PosY) const
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);

    bool OnState = false;
    m_CurrentLine = pi_PosY;

    // get the address of the line
    HPRECONDITION(m_pPacket->GetBufferAddress() != 0);
    m_pCurrentCount = &((unsigned short*)m_pPacket->GetBufferAddress())[m_pLineIndexes[pi_PosY]];
    HPOSTCONDITION(m_pCurrentCount <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize()));

    // parse all the pixels
    while (pi_PosX > *m_pCurrentCount || *m_pCurrentCount == 0)
        {
        pi_PosX -= *m_pCurrentCount;
        OnState = !OnState;
        m_pCurrentCount++;
        HPOSTCONDITION(m_pCurrentCount <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize()));
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
void* HRAEditorRLE1::GetNextPixel() const
    {
    HPRECONDITION(m_pCurrentCount != 0);

    // Attention: GetPixel() must have been called before this method.
    // Also, GetNextPixel() is not capable of falling to the next Y line.

    void* pData;
    if (m_pCurrentCount == 0)
        {
        if (m_CurrentLine == m_Height - 1)
            pData = 0;
        else
            pData = GetPixel(0, m_CurrentLine + 1);
        }
    else
        {
        if (m_RemainingCount == 0)
            {
            // Fetch the next non-zero run count
            ++m_pCurrentCount;
            bool SameState = false;
            HPOSTCONDITION(m_pCurrentCount <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize()));

            while (*m_pCurrentCount == 0)
                {
                SameState = !SameState;
                ++m_pCurrentCount;
                HPOSTCONDITION(m_pCurrentCount <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize()));
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
void HRAEditorRLE1::SetPixel(HUINTX         pi_PosX,
                             HUINTX         pi_PosY,
                             const void*    pi_pValue)
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
void HRAEditorRLE1::GetPixels(const HUINTX* pi_pPositionsX,
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
            if (*pBuffer == 32767)
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
void HRAEditorRLE1::GetPixels(HUINTX    pi_PosX,
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
                if (*pBuffer == 32767)
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
void HRAEditorRLE1::Clear(const void*   pi_pValue,
                          void*         pi_pTransaction)
    {
    HPRECONDITION(pi_pValue != 0);
    HPRECONDITION(*((Byte*)pi_pValue) == 1 || (*((Byte*)pi_pValue) == 0));
    HPRECONDITION(pi_pTransaction == 0);

    m_Edited = true;

    size_t RunsPerLine = (m_Width / 32767) * 2 + 1;

    if (*((Byte*)pi_pValue) == 1)
        RunsPerLine += 2; // we clear in white, add 2 bytes because we need to end with black

    size_t DataSize = RunsPerLine * m_Height * sizeof(unsigned short);
    m_pPacket->SetDataSize(0);
    if (m_pPacket->GetBufferSize() < DataSize)
        m_pPacket->ChangeBufferSize(DataSize);

    unsigned short* pRawData = (unsigned short*)m_pPacket->GetBufferAddress();

    // clear the first line
    unsigned short* pRun = pRawData;
    bool OnState;
    if (*((Byte*)pi_pValue) == 0)
        OnState = false;
    else
        {
        OnState = true;

        HPRECONDITION(pRun <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetBufferSize()));
        *pRun = 0;
        pRun++;
        }


    uint32_t PixelCount = m_Width;
    while (PixelCount != 0)
        {
        if (PixelCount > 32767)
            {
            HPOSTCONDITION(pRun <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetBufferSize() + 1));
            *pRun = 32767;
            PixelCount -= 32767;
            pRun++;

            *pRun = 0;
            pRun++;
            }
        else
            {
            HPOSTCONDITION(pRun <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetBufferSize()));
            *pRun = (unsigned short)PixelCount;
            pRun++;

            PixelCount = 0;
            }
        }

    if (OnState)
        {
        HPOSTCONDITION(pRun <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetBufferSize()));
        *pRun = 0;
        pRun++;
        }

    // set index of the first line
    m_pLineIndexes[0] = 0;

    // use the first line to clear all bitmap
    for (uint32_t LineIndex = 1; LineIndex < m_Height; LineIndex++, pRun += RunsPerLine)
        {
        memcpy(pRun, pRawData, RunsPerLine * sizeof(unsigned short));
        HASSERT_X64(m_pLineIndexes[LineIndex - 1] + RunsPerLine < ULONG_MAX);
        m_pLineIndexes[LineIndex] = (uint32_t)(m_pLineIndexes[LineIndex - 1] + RunsPerLine);
        }

    m_pPacket->SetDataSize(DataSize);
    }

/**----------------------------------------------------------------------------
 Clear a specific run

 @param pi_PosX
 @param pi_PosY
 @param pi_PixelCount
 @param pi_pValue
-----------------------------------------------------------------------------*/
void HRAEditorRLE1::ClearRun(HUINTX         pi_PosX,
                             HUINTX         pi_PosY,
                             size_t         pi_PixelCount,
                             const void*    pi_pValue,
                             void*          pi_pTransaction)
    {
    HPRECONDITION(pi_pValue != 0);
    HPRECONDITION(pi_pTransaction == 0);

    // calculate the worst-case
    // Add 1 for reminder
    // multiply by 2 for Off and On Value,
    // Add 1 to end with Off value
    size_t BufferSize = ((pi_PixelCount / 32767) + 1) * 2 + 1;

    HArrayAutoPtr<unsigned short> pRunBuffer(new unsigned short[BufferSize]);
    unsigned short* pRun = pRunBuffer;

    // initialize the buffer with 0
    memset(pRun, 0, BufferSize * sizeof(unsigned short));

    if (*((Byte*)pi_pValue) == 1) // clear with the On
        pRun++; // skip the off value

    size_t PixelCount = pi_PixelCount;
    while (PixelCount > 0)
        {
        if (PixelCount > 32767)
            {
            *pRun = 32767;
            PixelCount -= 32767;
            pRun += 2;  // with skip the On or the Off value
            }
        else
            {
            *pRun = (unsigned short)PixelCount;
            PixelCount = 0;
            }
        }

    SetRun(pi_PosX,
           pi_PosY,
           pi_PixelCount,
           pRunBuffer);
    }



/**----------------------------------------------------------------------------
Merge run

@param pi_PosX
@param pi_PosY
@param pi_PixelCount
@param pi_pValue
@param pi_Record
-----------------------------------------------------------------------------*/
void HRAEditorRLE1::MergeRuns(HUINTX        pi_StartPosX,
                              HUINTX        pi_StartPosY,
                              size_t        pi_Width,
                              size_t        pi_Height,
                              const void*   pi_pRun,
                              void*         pi_pTransaction)
    {
    HASSERT(0);
    }


//:>---------------------------------------------------------------------------------------
//:> protected section
//:>---------------------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Compute address of a specific pixel

 @return UShort*
-----------------------------------------------------------------------------*/
unsigned short* HRAEditorRLE1::ComputeAddress(HUINTX   pi_PosX,
                                       HUINTX   pi_PosY,
                                       unsigned short* po_pPixelsToSkipInFirstLen,
                                       unsigned short* po_pPixelsToSkipInSecondLen) const
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);
    HPRECONDITION(po_pPixelsToSkipInFirstLen != 0);
    HPRECONDITION(po_pPixelsToSkipInSecondLen != 0);
    HPRECONDITION(m_pLineIndexes != 0);


    // pointer at the beginning of a line
    unsigned short* pOffRun = &((unsigned short*)m_pPacket->GetBufferAddress())[m_pLineIndexes[pi_PosY]];

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
    HPOSTCONDITION(pOffRun <= (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize()));
    return pOffRun;
    }



//:>---------------------------------------------------------------------------------------
//:> private section
//:>---------------------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Get the next pixel of the current position

 @return void*
-----------------------------------------------------------------------------*/
void HRAEditorRLE1::GetRun(HUINTX   pi_StartPosX,
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
 PrepareToAppendDataInBuffer

 @param pi_DataSize

 @return UShort*
-----------------------------------------------------------------------------*/
unsigned short* HRAEditorRLE1::PrepareToAppendDataInBuffer(size_t pi_DataSize)
    {
    // test if the buffer is large enough
    if ((m_pPacket->GetBufferSize() - m_pPacket->GetDataSize()) <= pi_DataSize)
        {
        // add 10% of the actual size
        size_t BufferSize = pi_DataSize + (uint32_t)(m_pPacket->GetBufferSize() * 1.10);

        m_pPacket->ChangeBufferSize(BufferSize);
        }

    return (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize());
    }

/**----------------------------------------------------------------------------
 Check if a specific pixel is on or off

 @param pi_PosX
 @param pi_PosY

 @return bool true if the pixel is on, false if the pixel is off
-----------------------------------------------------------------------------*/
bool HRAEditorRLE1::IsPixelOn(HUINTX pi_PosX, HUINTX pi_PosY) const
    {
    bool OnState = false;

    // get the address of the line
    unsigned short* pRun = &((unsigned short*)m_pPacket->GetBufferAddress())[m_pLineIndexes[pi_PosY]];
    HPOSTCONDITION(pRun < (unsigned short*)(m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize()));

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
bool    HRAEditorRLE1::HasData() const
    {
    return m_pPacket->GetDataSize() != 0;
    }