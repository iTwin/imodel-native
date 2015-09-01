//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRANearestSamplerRLE1.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRANearestSamplerRLE1.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGSMemoryRLESurfaceDescriptor.h>
#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HRPPixelType1bitInterface.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>

#define RLE_RUN_LIMIT 32767

/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-----------------------------------------------------------------------------*/
HRANearestSamplerRLE1Base::HRANearestSamplerRLE1Base(HGSMemoryBaseSurfaceDescriptor const& pi_rMemorySurface,
                                                     const HGF2DRectangle&                 pi_rSampleDimension,
                                                     double                                pi_DeltaX,
                                                     double                                pi_DeltaY)
    : HRANearestSampler(pi_rMemorySurface,
                        pi_rSampleDimension,
                        pi_DeltaX,
                        pi_DeltaY)
    {
    HPRECONDITION(pi_rMemorySurface.GetPixelType() != 0);

    // Evaluate foreground mode and state.
    m_ForegroundMode = false;
    m_ForegroundState = true;

    HRPPixelType1BitInterface* pPixelType1BitInterface = pi_rMemorySurface.GetPixelType()->Get1BitInterface();
    HASSERT(pPixelType1BitInterface != 0);  // What kind of pixel type is that? Only 1bit are supposed to end up here.

    if (pPixelType1BitInterface != 0 && pPixelType1BitInterface->IsForegroundStateDefined())
        {
        m_ForegroundMode = true;
        m_ForegroundState = pPixelType1BitInterface->GetForegroundState();
        }

    m_aData[1] = 1;

    m_StretchByLine             = HDOUBLE_EQUAL_EPSILON(pi_DeltaY, 0.0);
    m_StretchByLineWithNoScale  = m_StretchByLine && HDOUBLE_EQUAL_EPSILON(pi_DeltaX, 1.0) ||
                                  m_StretchByLine && HDOUBLE_EQUAL_EPSILON(pi_DeltaX, -1.0);
    m_StretchLineByTwo          = m_StretchByLine &&
                                  (HDOUBLE_EQUAL_EPSILON(pi_DeltaX, 2.0) ||
                                   HDOUBLE_EQUAL_EPSILON(pi_DeltaX, -2.0));

    m_ReverseLine               = m_StretchByLine && pi_DeltaX < 0.0;
    }

/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
HRANearestSamplerRLE1Base::~HRANearestSamplerRLE1Base()
    {
    }


/**----------------------------------------------------------------------------
 Get a specific pixel.

 @param pi_PosX
 @param pi_PosY

 @return const void* The current pixel.
-----------------------------------------------------------------------------*/
void const* HRANearestSamplerRLE1Base::GetPixel(double pi_PosX, double pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0.0);
    HPRECONDITION(pi_PosY >= 0.0);
    HPRECONDITION(pi_PosX <= (double)m_Width + 0.5);
    HPRECONDITION(pi_PosY <= (double)m_Height + 0.5);

    bool OnState = false;

    if (pi_PosX >= m_Width)
        pi_PosX = m_Width - HGLOBAL_EPSILON;
    if (pi_PosY >= m_Height)
        pi_PosY = m_Height - HGLOBAL_EPSILON;

    // get the address of the line
    unsigned short* pCurrentCount = (unsigned short*)GetLineBufferAddress((uint32_t)pi_PosY);

    // parse all the pixels
    while (pi_PosX >= *pCurrentCount)
        {
        pi_PosX -= *pCurrentCount;
        OnState = !OnState;
        pCurrentCount++;
        HASSERT(pCurrentCount <= (unsigned short*)(GetLineBufferAddress((uint32_t)pi_PosY) + GetLineDataSize((uint32_t)pi_PosY)));
        }

    // compute the address
    if(OnState)
        m_aData[0] = 0;
    else
        m_aData[0] = 1;

    return m_aData;
    }

/**----------------------------------------------------------------------------
 Get pixels at specific location.

 @param pi_pPositionsX
 @param pi_pPositionsY
 @param pi_PixelCount
 @param po_pBuffer
-----------------------------------------------------------------------------*/
void HRANearestSamplerRLE1Base::GetPixels(const double*  pi_pPositionsX,
                                          const double*  pi_pPositionsY,
                                          size_t          pi_PixelCount,
                                          void*           po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaX, 0.0), L"The X scaling set into the sampler was not used\n");
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaY, 0.0), L"The Y scaling set into the sampler was not used\n");

    //TR#259220: Alloc array that will remember the last position in RLE buffer. It gives a huge performance gains when resampling: 45 MIN -> 2 MIN
    if(m_pLastRLEBufferPosition == 0)
        {
        m_pLastRLEBufferPosition = new RLEBufferPosition[m_Height];
        memset(m_pLastRLEBufferPosition.get(), 0, sizeof(RLEBufferPosition)*m_Height);
        }

    unsigned short* pBuffer = (unsigned short*)po_pBuffer;
    uint32_t BufferIndex = 0;
    uint32_t LastWrittenPixelIndex = 0;
    bool    OnMode = false;

    // get the individual pixels
    for(uint32_t PixelIndex = 0; PixelIndex < pi_PixelCount; ++PixelIndex)
        {
        HASSERT(pi_pPositionsX[PixelIndex] <= m_Width + 0.5);
        HASSERT(pi_pPositionsY[PixelIndex] <= m_Height + 0.5);

        double PosX = MIN(pi_pPositionsX[PixelIndex], m_Width - HGLOBAL_EPSILON);
        double PosY = MIN(pi_pPositionsY[PixelIndex], m_Height - HGLOBAL_EPSILON);

        if(OnMode != IsPixelOn((uint32_t)PosX, (uint32_t)PosY))
            {
            uint32_t ToWrite = (PixelIndex - LastWrittenPixelIndex);

            while(ToWrite > RLE_RUN_LIMIT)
                {
                pBuffer[BufferIndex] = RLE_RUN_LIMIT;
                pBuffer[BufferIndex + 1] = 0;
                ToWrite -= RLE_RUN_LIMIT;
                BufferIndex+=2;
                }
            pBuffer[BufferIndex] = (unsigned short)ToWrite;
            ++BufferIndex;

            LastWrittenPixelIndex = PixelIndex;
            OnMode = !OnMode;
            }
        }

    // Fill last run.
    if(LastWrittenPixelIndex < pi_PixelCount)
        {
        size_t ToWrite = (pi_PixelCount - LastWrittenPixelIndex);

        while(ToWrite > RLE_RUN_LIMIT)
            {
            pBuffer[BufferIndex] = RLE_RUN_LIMIT;
            pBuffer[BufferIndex + 1] = 0;
            ToWrite -= RLE_RUN_LIMIT;
            BufferIndex+=2;
            }
        pBuffer[BufferIndex] = (unsigned short)ToWrite;
        ++BufferIndex;

        OnMode = !OnMode;
        }

    // Make sure we end with black
    if(!OnMode)
        pBuffer[BufferIndex] = 0;
    }

/**----------------------------------------------------------------------------
 Get pixels

 @param pi_PositionX
 @param pi_PositionY
 @param pi_PixelCount
 @param pi_pBuffer
-----------------------------------------------------------------------------*/
void HRANearestSamplerRLE1Base::GetPixels(double         pi_PositionX,
                                          double         pi_PositionY,
                                          size_t          pi_PixelCount,
                                          void*           po_pBuffer) const
    {
    BeAssertOnce(pi_PositionX >= 0.0);
    BeAssertOnce(pi_PositionY >= 0.0);

    unsigned short* pDstRun = (unsigned short*)po_pBuffer;
    pi_PositionY = MIN(pi_PositionY, (double)(m_Height - 1));  // TR 228473

    if (m_StretchByLine)
        {
        double DeltaX = m_DeltaX;
        if (m_ReverseLine)
            {
            pi_PositionX += m_DeltaX * (pi_PixelCount - 1);
            DeltaX = -m_DeltaX;
            }

        // compute the address of the RLE1 run
        uint32_t PixelsToSkipInFirstLen;
        uint32_t PixelsToSkipInSecondLen;
        uint32_t RunPos;
        unsigned short* pSrcRun =  ComputeAddress((uint32_t)pi_PositionX,
                                           (uint32_t)pi_PositionY,
                                           &PixelsToSkipInFirstLen,
                                           &PixelsToSkipInSecondLen,
                                           &RunPos);

        bool   SrcState = false;   // start with the black pixel
        bool   DstState = false;   // start with the black pixel
        int32_t SrcLen = *pSrcRun;

        uint32_t EndRunPos = RunPos + *pSrcRun;

        if (PixelsToSkipInFirstLen || PixelsToSkipInSecondLen)
            {
            SrcLen -= PixelsToSkipInFirstLen;
            if (SrcLen == 0)
                {
                SrcState = !SrcState;
                SrcLen = *(++pSrcRun);
                EndRunPos += SrcLen;
                SrcLen -= PixelsToSkipInSecondLen;
                }
            HDEBUGCODE(else)
            HDEBUGCODE( {)
                        HDEBUGCODE(    HASSERT(PixelsToSkipInSecondLen == 0);)
                        HDEBUGCODE(
                        })
            }

        if (m_StretchByLineWithNoScale)
            {
            if (SrcState != DstState)   // the source and destination aren't in the same state,
                {   // write a 0 run length and change the destination state
                *pDstRun++ = 0;
                DstState = !DstState;
                }

            // Note
            // our code is written like that for PAGALLOC
            // don't increment pSrcRun outside the run

            // Copy runs directly to destination.
            uint32_t OutputPixels;

            if (pi_PixelCount > 0)
                {
                // need more pixel than we have
                unsigned short PixelOver = 0;
                if (pi_PixelCount > (uint32_t)pi_PositionX + m_Width)
                    {
                    PixelOver = (unsigned short)((uint32_t)pi_PositionX + pi_PixelCount - m_Width);
                    pi_PixelCount -= PixelOver;
                    }

                OutputPixels = MIN(SrcLen, (int32_t)pi_PixelCount);
                *pDstRun++ = (unsigned short)OutputPixels;     // write the run

                pi_PixelCount -= OutputPixels;

                DstState = !DstState;   // change the destination state

                while (pi_PixelCount != 0)
                    {
                    SrcLen = *(++pSrcRun);    // can increment the ptr, we are inside the buffer run (PAGALLOC)
                    OutputPixels = MIN(SrcLen, (int32_t)pi_PixelCount);
                    *pDstRun++ = (unsigned short)OutputPixels; // write the run

                    pi_PixelCount -= OutputPixels;

                    DstState = !DstState;   // change the destination state
                    }

                if (PixelOver != 0)
                    {
                    // fill the end of the line with the last pixel available
                    if (SHRT_MAX - PixelOver > *(pDstRun - 1))
                        {
                        *pDstRun++ = 0;
                        *pDstRun++ = (unsigned short)PixelOver;
                        }
                    else
                        *(pDstRun - 1) += PixelOver;
                    }
                }
            }
        else
            {
            double RunLen;
            size_t  DstRunLen;
            *pDstRun = 0;   // initialize the run to 0

            // Foreground mode activated?
            if(m_ForegroundMode)
                {
                while (pi_PixelCount != 0)
                    {
                    if (EndRunPos == m_Width)
                        DstRunLen = pi_PixelCount;
                    else
                        {
                        HPRECONDITION(pi_PositionX < (double)m_Width);
                        while (EndRunPos < m_Width && ((double)EndRunPos < pi_PositionX || *pSrcRun == 0))
                            {
                            EndRunPos += *(++pSrcRun);
                            SrcState = !SrcState;
                            }

                        if (EndRunPos == m_Width)
                            DstRunLen = pi_PixelCount;
                        else
                            {
                            RunLen = EndRunPos - pi_PositionX;
                            if(m_ForegroundState == SrcState)
                                {
                                //OPTIMIZ: Use a premult instead of a division
                                DstRunLen = MIN((size_t)((RunLen / DeltaX)+1), pi_PixelCount);
                                }
                            else
                                {
                                DstRunLen = MIN(((size_t)(RunLen / DeltaX)), pi_PixelCount);
                                }
                            pi_PositionX += (double)DstRunLen * DeltaX;

                            // we have reach the end of the image
                            if ((uint32_t)pi_PositionX == m_Width)
                                DstRunLen = pi_PixelCount;
                            }
                        }

                    if (SrcState != DstState)   // the source and destination aren't in the same state,
                        {
                        if (pDstRun > po_pBuffer)
                            pDstRun--;          // set to the run before, this run is in the same state that the src run
                        else
                            *(++pDstRun) = 0;   // set pointer to the next run, initialize the run to 0
                        DstState = !DstState;
                        }

                    HPRECONDITION((uint32_t)pi_PositionX <= m_Width || DstRunLen == pi_PixelCount);
                    HPRECONDITION(pi_PixelCount >= DstRunLen);

                    pi_PixelCount -= DstRunLen;

                    DstRunLen += *pDstRun;

                    // Write the current run to the destination
                    while (DstRunLen > SHRT_MAX)
                        {
                        *pDstRun++ = SHRT_MAX;
                        *pDstRun++ = 0;
                        DstRunLen -= SHRT_MAX;
                        }
                    *pDstRun = (unsigned short)DstRunLen;  // write the run
                    //HASSERT(*pDstRun != 0);         // Make sure we have keep at least one pixel
                    *(++pDstRun) = 0;               // set to the next run, initialize to 0

                    if (pi_PixelCount)
                        EndRunPos += *(++pSrcRun);  // go to the next source run

                    // change state
                    DstState = !DstState;
                    SrcState = !SrcState;
                    }
                }
            else
                {
                while (pi_PixelCount != 0)
                    {
                    if (EndRunPos == m_Width)
                        DstRunLen = pi_PixelCount;
                    else
                        {
                        HPRECONDITION(pi_PositionX < (double)m_Width);
                        while (EndRunPos < m_Width && ((double)EndRunPos < pi_PositionX || *pSrcRun == 0))
                            {
                            EndRunPos += *(++pSrcRun);
                            SrcState = !SrcState;
                            }

                        if (EndRunPos == m_Width)
                            DstRunLen = pi_PixelCount;
                        else
                            {
                            RunLen = EndRunPos - pi_PositionX;
                            // at least, keep one pixel
                            DstRunLen = MAX(MIN((uint32_t)(RunLen / DeltaX), pi_PixelCount), 1);
                            pi_PositionX += (double)DstRunLen * DeltaX;

                            // we have reach the end of the image
                            if ((uint32_t)pi_PositionX == m_Width)
                                DstRunLen = pi_PixelCount;
                            }
                        }

                    if (SrcState != DstState)   // the source and destination aren't in the same state,
                        {
                        if (pDstRun > po_pBuffer)
                            pDstRun--;          // set to the run before, this run is in the same state that the src run
                        else
                            *(++pDstRun) = 0;   // set pointer to the next run, initialize the run to 0
                        DstState = !DstState;
                        }

                    HPRECONDITION((uint32_t)pi_PositionX <= m_Width || DstRunLen == pi_PixelCount);
                    HPRECONDITION(pi_PixelCount >= DstRunLen);

                    pi_PixelCount -= DstRunLen;

                    DstRunLen += *pDstRun;

                    // Write the current run to the destination
                    while (DstRunLen > SHRT_MAX)
                        {
                        *pDstRun++ = SHRT_MAX;
                        *pDstRun++ = 0;
                        DstRunLen -= SHRT_MAX;
                        }
                    *pDstRun = (unsigned short)DstRunLen;  // write the run
                    HASSERT(*pDstRun != 0);         // Make sure we have keep at least one pixel
                    *(++pDstRun) = 0;               // set to the next run, initialize to 0

                    if (pi_PixelCount)
                        EndRunPos += *(++pSrcRun);  // go to the next source run

                    // change state
                    DstState = !DstState;
                    SrcState = !SrcState;
                    }
                }
            HPOSTCONDITION((uint32_t)EndRunPos <= m_Width);


            }
        if (!DstState)
            *pDstRun++ = 0;

        if (m_ReverseLine)
            {
            unsigned short* pRun = (unsigned short*)po_pBuffer;
            unsigned short Tmp;
            pDstRun--;
            while (pDstRun > pRun)
                {
                Tmp = *pDstRun;
                *pDstRun-- = *pRun;
                *pRun++ = Tmp;
                }
            }
        }
    else
        {
        HAutoPtr<double> pXPositions(new double[pi_PixelCount]);
        HAutoPtr<double> pYPositions(new double[pi_PixelCount]);

        for (size_t Position = 0 ; Position < pi_PixelCount ; ++Position)
            {
            pXPositions[Position] = pi_PositionX;
            pYPositions[Position] = pi_PositionY;

            pi_PositionX += m_DeltaX;
            pi_PositionY += m_DeltaY;
            }

        GetPixels(pXPositions, pYPositions, pi_PixelCount, po_pBuffer);
        }
    }



//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// private
// ComputeAddress
//-----------------------------------------------------------------------------
unsigned short* HRANearestSamplerRLE1Base::ComputeAddress(HUINTX   pi_PosX,
                                                   HUINTX   pi_PosY,
                                                   uint32_t*  po_pPixelsToSkipInFirstLen,
                                                   uint32_t*  po_pPixelsToSkipInSecondLen,
                                                   uint32_t*  po_pRunPos) const
    {
    HPRECONDITION(po_pPixelsToSkipInFirstLen != 0);
    HPRECONDITION(po_pPixelsToSkipInSecondLen != 0);
    HPRECONDITION(po_pRunPos != 0);

    // pointer at the beginning of a line
    unsigned short* pOffRun = (unsigned short*)GetLineBufferAddress(MIN(pi_PosY, m_Height-1));

    HASSERT_X64(pi_PosX < ULONG_MAX);
    uint32_t PixelsFromRun = (uint32_t)MIN(pi_PosX, m_Width-1);

    bool PixelFound = false;
    *po_pRunPos = 0;

    while (!PixelFound)
        {
        if (PixelsFromRun < *pOffRun)
            {
            *po_pPixelsToSkipInFirstLen   = PixelsFromRun;
            *po_pPixelsToSkipInSecondLen  = 0;

            PixelFound = true;
            }
        else if (PixelsFromRun < (uint32_t)(*pOffRun + *(pOffRun + 1)))
            {
            *po_pPixelsToSkipInFirstLen   = *pOffRun;
            *po_pPixelsToSkipInSecondLen  = PixelsFromRun - *pOffRun;

            PixelFound = true;
            }
        else
            {
            *po_pRunPos += (*pOffRun + *(pOffRun + 1));
            PixelsFromRun -= (*pOffRun + *(pOffRun + 1));
            pOffRun += 2;
            }
        }

    //:> This post condition is use to verify if we read a initialized data
    //:> call Clear() to initialize the buffer
    HPOSTCONDITION(pOffRun <= (unsigned short*)GetLineBufferAddress(MIN(pi_PosY, m_Height-1)) + GetLineDataSize(MIN(pi_PosY, m_Height-1)));

    return pOffRun;
    }

//-----------------------------------------------------------------------------
// private
// IsPixelOn
//-----------------------------------------------------------------------------
bool HRANearestSamplerRLE1Base::IsPixelOn(uint32_t pi_PosX, uint32_t pi_PosY) const
    {
    HPRECONDITION(m_pLastRLEBufferPosition != NULL);

    unsigned short const*     pRun = (unsigned short const*)GetLineBufferAddress(pi_PosY);

    RLEBufferPosition& LastBufferPosition = m_pLastRLEBufferPosition[pi_PosY];

    if(LastBufferPosition.m_StartPosition <= pi_PosX)
        {
        // Go foreword in RLE buffer
        while (LastBufferPosition.m_StartPosition <= pi_PosX)
            {
            HASSERT(LastBufferPosition.m_IndexInBuffer < GetLineDataSize(pi_PosY) >> 1);
            LastBufferPosition.m_StartPosition += pRun[LastBufferPosition.m_IndexInBuffer];
            ++LastBufferPosition.m_IndexInBuffer;
            }

        HASSERT(LastBufferPosition.m_IndexInBuffer-1 < GetLineDataSize(pi_PosY) >> 1);

        // Black runs are ON even number. 0,2,4,6...
        return (LastBufferPosition.m_IndexInBuffer-1) & 0x00000001;
        }

    // Go backward in RLE buffer
    while (LastBufferPosition.m_StartPosition > pi_PosX)
        {
        HASSERT(LastBufferPosition.m_IndexInBuffer-1 < GetLineDataSize(pi_PosY) >> 1);
        LastBufferPosition.m_StartPosition -= pRun[LastBufferPosition.m_IndexInBuffer-1];
        --LastBufferPosition.m_IndexInBuffer;
        }

    HASSERT(LastBufferPosition.m_IndexInBuffer < GetLineDataSize(pi_PosY) >> 1);

    // Black runs are ON even number. 0,2,4,6...
    return LastBufferPosition.m_IndexInBuffer & 0x00000001;
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
HRANearestSamplerRLE1::HRANearestSamplerRLE1(HGSMemorySurfaceDescriptor const&    pi_rMemorySurface,
                                             const HGF2DRectangle&                pi_rSampleDimension,
                                             double                               pi_DeltaX,
                                             double                               pi_DeltaY)
    :HRANearestSamplerRLE1Base(pi_rMemorySurface, 
                               pi_rSampleDimension, 
                               pi_DeltaX, pi_DeltaY)
    {
    HPRECONDITION(pi_rMemorySurface.GetPacket() != 0);

    m_pPacket = pi_rMemorySurface.GetPacket();
    HFCPtr<HCDCodecHMRRLE1> pCodec( (const HFCPtr<HCDCodecHMRRLE1>&) m_pPacket->GetCodec() );
    HPRECONDITION(pCodec->HasLineIndexesTable());
    m_pLineIndexes  = pCodec->GetLineIndexesTable();
    m_LineIndexesCount = static_cast<uint32_t>(pCodec->GetHeight());
    }

//-----------------------------------------------------------------------------
// protected
//-----------------------------------------------------------------------------
Byte* HRANearestSamplerRLE1::GetLineBufferAddress(HUINTX pi_PosY) const
    {
    HPRECONDITION(pi_PosY < m_LineIndexesCount);

    return m_pPacket->GetBufferAddress() + (m_pLineIndexes[pi_PosY] << 1);
    }

//-----------------------------------------------------------------------------
// protected
//-----------------------------------------------------------------------------
size_t  HRANearestSamplerRLE1::GetLineDataSize(HUINTX pi_PosY) const
    {
    HPRECONDITION(pi_PosY < m_LineIndexesCount);

    if(m_LineIndexesCount == pi_PosY + 1)
        return m_pPacket->GetDataSize() - (m_pLineIndexes[pi_PosY] << 1);

    return (m_pLineIndexes[pi_PosY + 1] - m_pLineIndexes[pi_PosY]) << 1;
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
HRANearestSamplerRLE1Line::HRANearestSamplerRLE1Line(HGSMemoryRLESurfaceDescriptor const& pi_rMemorySurface,
                                                     const HGF2DRectangle&                pi_rSampleDimension,
                                                     double                               pi_DeltaX,
                                                     double                               pi_DeltaY)
    :HRANearestSamplerRLE1Base(pi_rMemorySurface, 
                               pi_rSampleDimension, 
                               pi_DeltaX, pi_DeltaY)
    {
    HPRECONDITION(pi_rMemorySurface.GetRLEPacket() != 0);
    HPRECONDITION(pi_rMemorySurface.GetRLEPacket()->IsCompatibleWith(HCDPacketRLE::CLASS_ID));
    m_pPacketRLE = (HCDPacketRLE*)pi_rMemorySurface.GetRLEPacket().GetPtr();
    }

//-----------------------------------------------------------------------------
// protected
//-----------------------------------------------------------------------------
Byte* HRANearestSamplerRLE1Line::GetLineBufferAddress(HUINTX pi_PosY) const
    {
    return m_pPacketRLE->GetLineBuffer(pi_PosY);
    }

//-----------------------------------------------------------------------------
// protected
//-----------------------------------------------------------------------------
size_t  HRANearestSamplerRLE1Line::GetLineDataSize(HUINTX pi_PosY) const
    {
    return m_pPacketRLE->GetLineDataSize(pi_PosY);
    }