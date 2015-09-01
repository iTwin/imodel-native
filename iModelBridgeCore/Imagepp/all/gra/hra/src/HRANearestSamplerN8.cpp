//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRANearestSamplerN8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//:>---------------------------------------------------------------------------------------
//:> Class HRANearestSamplerN8
//:>---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRANearestSamplerN8.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRASurface.h>

//#define MMX_OPTIMIZE_ON

#ifdef MMX_OPTIMIZE_ON
#include <Imagepp/all/h/emmintrin.h>
#endif

/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-----------------------------------------------------------------------------*/
HRANearestSamplerN8::HRANearestSamplerN8(HGSMemorySurfaceDescriptor const&     pi_rMemorySurface,
                                         const HGF2DRectangle&                 pi_rSampleDimension,
                                         double                                pi_DeltaX,
                                         double                                pi_DeltaY)
    : HRANearestSampler(pi_rMemorySurface,
                        pi_rSampleDimension,
                        pi_DeltaX,
                        pi_DeltaY)
    {
    HPRECONDITION(pi_rMemorySurface.GetPixelType() != 0);
    HPRECONDITION((pi_rMemorySurface.GetPixelType()->CountPixelRawDataBits() % 8) == 0);
    m_BytesPerPixel = pi_rMemorySurface.GetPixelType()->CountPixelRawDataBits() / 8;
    m_BytesPerLine  = pi_rMemorySurface.GetBytesPerRow();
    m_DataWidth     = pi_rMemorySurface.GetDataWidth();
    m_DataHeight    = pi_rMemorySurface.GetDataHeight();
    m_SLO4          = pi_rMemorySurface.GetSLO() == HGF_UPPER_LEFT_HORIZONTAL;

    HPRECONDITION(pi_rMemorySurface.GetPacket() != 0);
    m_pPacket = pi_rMemorySurface.GetPacket();

    // optimization
    m_StretchByLine             = HDOUBLE_EQUAL_EPSILON(m_DeltaY, 0.0);
    m_StretchByLineWithNoScale  = m_StretchByLine && HDOUBLE_EQUAL_EPSILON(m_DeltaX, 1.0);
    m_StretchLineByTwo          = m_StretchByLine &&
                                  HDOUBLE_EQUAL_EPSILON(m_DeltaX, 2.0) &&
                                  m_BytesPerPixel <= 4;

    }

/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
HRANearestSamplerN8::~HRANearestSamplerN8()
    {
    }


/**----------------------------------------------------------------------------
 Get a specific pixel.

 @param pi_PosX
 @param pi_PosY

 @return const void* The current pixel.
-----------------------------------------------------------------------------*/
void const* HRANearestSamplerN8::GetPixel(double pi_PosX, double pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0.0);
    HPRECONDITION(pi_PosY >= 0.0);
    HPRECONDITION(pi_PosX <= (double)m_DataWidth + 0.5);
    HPRECONDITION(pi_PosY <= (double)m_DataHeight + 0.5);

    return ComputeAddress((uint32_t)pi_PosX, (uint32_t)pi_PosY);
    }

/**----------------------------------------------------------------------------
 Get pixels at specific location.

 @param pi_pPositionsX
 @param pi_pPositionsY
 @param pi_PixelCount
 @param po_pBuffer
-----------------------------------------------------------------------------*/
void HRANearestSamplerN8::GetPixels(const double*  pi_pPositionsX,
                                    const double*  pi_pPositionsY,
                                    size_t          pi_PixelCount,
                                    void*           po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaX, 0.0), L"The X scaling set into the sampler was not used\n");
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaY, 0.0), L"The Y scaling set into the sampler was not used\n");

    const double*  pPosX = pi_pPositionsX;
    const double*  pPosY = pi_pPositionsY;
    Byte*          pOut = (Byte*)po_pBuffer;

    while (pi_PixelCount > 0)
        {
// The position we get here is a result of a transfo which means the initial epsilon may have grow and thus causing an assertion failed.
// So use a smaller one to minimize the case where this HASSERT pop up. Any double position near 0.0 will be cast to 0 before the call to ComputeAddress().
#define EPS_SMALLER_THEN_HGLOBAL_EPSILON (HGLOBAL_EPSILON*10)

        HPRECONDITION(HDOUBLE_GREATER_OR_EQUAL(*pPosX, 0.0, EPS_SMALLER_THEN_HGLOBAL_EPSILON) && *pPosX <= (double)m_DataWidth + 0.5);
        HPRECONDITION(HDOUBLE_GREATER_OR_EQUAL(*pPosY, 0.0, EPS_SMALLER_THEN_HGLOBAL_EPSILON) && *pPosY <= (double)m_DataHeight + 0.5);

        memcpy(pOut,
               ComputeAddress((uint32_t)*pPosX,
                              (uint32_t)*pPosY),
               m_BytesPerPixel);
        pPosX++;
        pPosY++;
        pOut += m_BytesPerPixel;
        pi_PixelCount--;
        }
    }


/**----------------------------------------------------------------------------
 Get pixels

 @note For performance, the code was duplicated


 @param pi_PositionX
 @param pi_PositionY
 @param pi_PixelCount
 @param pi_pBuffer
-----------------------------------------------------------------------------*/
void HRANearestSamplerN8::GetPixels(double         pi_PositionX,
                                    double         pi_PositionY,
                                    size_t          pi_PixelCount,
                                    void*           po_pBuffer) const
    {
    BeAssertOnce(pi_PositionX >= 0.0);
    BeAssertOnce(pi_PositionY >= 0.0);


    pi_PositionY = MIN(pi_PositionY, (double)(m_DataHeight - 1));

    // Try to optimize the pixel sampling by testing if we can use a unit scaling
    bool ForceStretchWithNoScale(false);

    if (!m_StretchByLineWithNoScale && m_StretchByLine)
        {
        if ((fabs(((double)(pi_PixelCount) * m_DeltaX) - ((double)(pi_PixelCount))) < 1.0) && (uint32_t)pi_PositionX + pi_PixelCount <= m_DataWidth)
            ForceStretchWithNoScale = true;
        }

    if (m_StretchByLineWithNoScale || ForceStretchWithNoScale)
        {
        uint32_t PosX = (uint32_t)pi_PositionX;
        Byte* pRawData = ComputeAddress(PosX,
                                         (uint32_t)pi_PositionY);
        if (PosX + pi_PixelCount > m_DataWidth)
            {
            size_t PixelOver = pi_PixelCount + PosX - m_DataWidth;
            pi_PixelCount -= PixelOver;
            memcpy(po_pBuffer, pRawData, pi_PixelCount * m_BytesPerPixel);
            Byte* pLastValue((Byte*)po_pBuffer + (pi_PixelCount - 1) * m_BytesPerPixel);
            Byte* pBuffer(pLastValue + m_BytesPerPixel);
            while (PixelOver > 0)
                {
                memcpy(pBuffer, pLastValue, m_BytesPerPixel);
                pBuffer += m_BytesPerPixel;
                --PixelOver;
                }
            }
        else
            {
            memcpy(po_pBuffer, pRawData, pi_PixelCount * m_BytesPerPixel);
            }
        }
    else if (m_StretchLineByTwo)
        {
        if (m_BytesPerPixel == 1)
            {
#ifndef MMX_OPTIMIZE_ON

            Byte* pFirstPixel = (Byte*)ComputeAddress(0,
                                                          (uint32_t)pi_PositionY);
            Byte* pLastPixel = pFirstPixel + m_DataWidth - 1;
            Byte* pSrcData = pFirstPixel + (uint32_t)MAX(MIN(pi_PositionX, (double)m_DataWidth - HGLOBAL_EPSILON), 0.0);
            Byte* pDstData = (Byte*)po_pBuffer;
            short SrcStep = m_DeltaX < 0.0 ? -2 : 2;
            while (pi_PixelCount != 0)
                {
                *pDstData++ = *pSrcData;
                pSrcData = MAX(MIN(pSrcData + SrcStep, pLastPixel), pFirstPixel);
                pi_PixelCount--;
                }
#else
            Byte* pFirstPixel = (Byte*)ComputeAddress(0,
                                                          (uint32_t)pi_PositionY);
            Byte* pLastPixel = pFirstPixel + m_DataWidth - 1;
            register Byte* pSrcData = pFirstPixel + (uint32_t)MAX(MIN(pi_PositionX, (double)m_DataWidth - HGLOBAL_EPSILON), 0.0);
            register Byte* pDstData = (Byte*)po_pBuffer;
            short SrcStep = m_DeltaX < 0.0 ? -2 : 2;

            uint32_t PixelCount = pi_PixelCount;
            if ((m_DataWidth < (pi_PixelCount<<1)))
                {
                PixelCount = pi_PixelCount - (((pi_PixelCount<<1) - m_DataWidth) >> 1);
                }

            uint32_t PixelCountReste = PixelCount % 8;
            int32_t PixelCount1 = PixelCount>>3;
            uint32_t SrcStepBig = SrcStep>>3;

            int64_t* pDstData8 = (int64_t*)pDstData;
            int64_t* pSrcData8 = (int64_t*)pSrcData;
            __m64   m1;
            __m64   m2;
            while (--PixelCount1 >= 0)
                {
                m1.m64_u64 = *pSrcData8++;
                m2.m64_u64 = *pSrcData8++;
                *pDstData8++ = (_mm_packs_pu16(_mm_srli_pi16(m1, 8),
                                               _mm_srli_pi16(m2, 8))).m64_u64;
                }
            _mm_empty();

            pSrcData += PixelCount;
            while(PixelCountReste > 0)
                {
                *pDstData++ = *pSrcData;
                pSrcData += SrcStep;
                --PixelCountReste;
                }
            if (pi_PixelCount-PixelCount > 0)
                memset(pDstData, *pLastPixel, pi_PixelCount-PixelCount);
#endif
            }
        else if (m_BytesPerPixel == 2)
            {
            HPRECONDITION(sizeof(unsigned short) == 2);

            unsigned short* pFirstPixel = (unsigned short*)ComputeAddress(0,
                                                            (uint32_t)pi_PositionY);
            unsigned short* pLastPixel = pFirstPixel + m_DataWidth - 1;
            unsigned short* pSrcData = pFirstPixel + (uint32_t)MAX(MIN(pi_PositionX, (double)m_DataWidth - HGLOBAL_EPSILON), 0.0);

            unsigned short* pDstData = (unsigned short*)po_pBuffer;
            short SrcStep = m_DeltaX < 0.0 ? -2 : 2;
            while (pi_PixelCount != 0)
                {
                *pDstData++ = *pSrcData;
                pSrcData = MAX(MIN(pSrcData + SrcStep, pLastPixel), pFirstPixel);
                pi_PixelCount--;
                }
            }
        else if (m_BytesPerPixel == 3)
            {
            Byte* pFirstPixel = (Byte*)ComputeAddress(0,
                                                          (uint32_t)pi_PositionY);
            Byte* pLastPixel = pFirstPixel + ((m_DataWidth - 1) * 3);
            Byte* pSrcData = pFirstPixel + (uint32_t)MAX(MIN(pi_PositionX, (double)m_DataWidth - HGLOBAL_EPSILON), 0.0) * 3;
            Byte* pDstData = (Byte*)po_pBuffer;
            short SrcStep = m_DeltaX < 0.0 ? -6 : 3;
            while (pi_PixelCount != 0)
                {
                *pDstData++ = *pSrcData++;
                *pDstData++ = *pSrcData++;
                *pDstData++ = *pSrcData++;
                pSrcData = MAX(MIN(pSrcData + SrcStep, pLastPixel), pFirstPixel);
                pi_PixelCount--;
                }
            }
        else if (m_BytesPerPixel == 4)
            {
            HPRECONDITION(sizeof(uint32_t) == 4);

            uint32_t* pFirstPixel = (uint32_t*)ComputeAddress(0,
                                                          (uint32_t)pi_PositionY);
            uint32_t* pLastPixel = pFirstPixel + m_DataWidth - 1;
            uint32_t* pSrcData = pFirstPixel + (uint32_t)MAX(MIN(pi_PositionX, (double)m_DataWidth - HGLOBAL_EPSILON), 0.0);

            uint32_t* pDstData = (uint32_t*)po_pBuffer;
            short SrcStep = m_DeltaX < 0.0 ? -2 : 2;
            while (pi_PixelCount != 0)
                {
                *pDstData++ = *pSrcData;
                pSrcData = MAX(MIN(pSrcData + SrcStep, pLastPixel), pFirstPixel);
                pi_PixelCount--;
                }
            }
        else
            {
            Byte* pSrcData = (Byte*)ComputeAddress((uint32_t)pi_PositionX,
                                                     (uint32_t)pi_PositionY);
            Byte* pDstData = (Byte*)po_pBuffer;
            short SrcStep = (short)(m_DeltaX * m_BytesPerPixel);
            while (pi_PixelCount != 0)
                {
                memcpy(pDstData, pSrcData, m_BytesPerPixel);
                pDstData += m_BytesPerPixel;
                pSrcData += SrcStep;
                pi_PixelCount--;
                }
            }
        }
    else
        {
        double MaxPosX = (double)m_DataWidth - HGLOBAL_EPSILON;
        pi_PositionX = MAX(MIN(pi_PositionX, MaxPosX), 0.0);

        if (m_StretchByLine)
            {
            if (m_BytesPerPixel == 1)
                {
                Byte* pSrcData = (Byte*)ComputeAddress(0,
                                                           (uint32_t)pi_PositionY);
                Byte* pDstData = (Byte*)po_pBuffer;

                while (pi_PixelCount > 0)
                    {
                    *pDstData++ = pSrcData[(uint32_t)pi_PositionX];
                    pi_PositionX = MAX(MIN(pi_PositionX + m_DeltaX, MaxPosX), 0.0);
                    pi_PixelCount--;
                    }
                }
            else if (m_BytesPerPixel == 2)
                {
                HPRECONDITION(sizeof(unsigned short) == 2);

                unsigned short* pDstData = (unsigned short*)po_pBuffer;
                unsigned short* pSrcData = (unsigned short*)ComputeAddress(0,
                                                             (uint32_t)pi_PositionY);

                while (pi_PixelCount > 0)
                    {
                    *pDstData++ = pSrcData[(uint32_t)pi_PositionX];
                    pi_PositionX = MAX(MIN(pi_PositionX + m_DeltaX, MaxPosX), 0.0);
                    pi_PixelCount--;
                    }
                }
            else if (m_BytesPerPixel == 3)
                {
                Byte* pSrcData = (Byte*)ComputeAddress(0,
                                                           (uint32_t)pi_PositionY);
                Byte* pDstData = (Byte*)po_pBuffer;
                int32_t PixelIndex;
                while (pi_PixelCount > 0)
                    {
                    PixelIndex = (uint32_t)pi_PositionX * m_BytesPerPixel;
                    *pDstData++ = pSrcData[PixelIndex];
                    *pDstData++ = pSrcData[PixelIndex + 1];
                    *pDstData++ = pSrcData[PixelIndex + 2];
                    pi_PositionX = MAX(MIN(pi_PositionX + m_DeltaX, MaxPosX), 0.0);
                    pi_PixelCount--;
                    }
                }
            else if (m_BytesPerPixel == 4)
                {
                HPRECONDITION(sizeof(uint32_t) == 4);

                uint32_t* pSrcData = (uint32_t*)ComputeAddress(0,
                                                           (uint32_t)pi_PositionY);
                uint32_t* pDstData = (uint32_t*)po_pBuffer;

                while (pi_PixelCount > 0)
                    {
                    *pDstData++ = pSrcData[(uint32_t)pi_PositionX];
                    pi_PositionX = MAX(MIN(pi_PositionX + m_DeltaX, MaxPosX), 0.0);
                    pi_PixelCount--;
                    }
                }
            else
                {
                Byte* pSrcData = (Byte*)ComputeAddress(0,
                                                         (uint32_t)pi_PositionY);
                Byte* pDstData = (Byte*)po_pBuffer;

                while (pi_PixelCount > 0)
                    {
                    memcpy(pDstData, &pSrcData[(uint32_t)pi_PositionX * m_BytesPerPixel], m_BytesPerPixel);
                    pDstData += m_BytesPerPixel;
                    pi_PositionX = MAX(MIN(pi_PositionX + m_DeltaX, MaxPosX), 0.0);
                    pi_PixelCount--;
                    }
                }
            }
        else
            {
            if (m_BytesPerPixel == 1)
                {
                Byte* pDstData = (Byte*)po_pBuffer;
                while (pi_PixelCount > 0)
                    {
                    *pDstData++ = *(Byte*)ComputeAddress((uint32_t)pi_PositionX,
                                                          (uint32_t)pi_PositionY);
                    pi_PositionX += m_DeltaX;
                    pi_PositionY += m_DeltaY;
                    pi_PixelCount--;
                    }
                }
            else if (m_BytesPerPixel == 2)
                {
                short* pDstData = (short*)po_pBuffer;
                while (pi_PixelCount > 0)
                    {
                    *pDstData++ = *(short*)ComputeAddress((uint32_t)pi_PositionX,
                                                           (uint32_t)pi_PositionY);
                    pi_PositionX += m_DeltaX;
                    pi_PositionY += m_DeltaY;
                    pi_PixelCount--;
                    }
                }
            else if (m_BytesPerPixel == 3)
                {
                }
            else if (m_BytesPerPixel == 4)
                {
                int32_t* pDstData = (int32_t*)po_pBuffer;
                while (pi_PixelCount > 0)
                    {
                    *pDstData++ = *(int32_t*)ComputeAddress((uint32_t)pi_PositionX,
                                                          (uint32_t)pi_PositionY);
                    pi_PositionX += m_DeltaX;
                    pi_PositionY += m_DeltaY;
                    pi_PixelCount--;
                    }
                }
            else
                {
                Byte* pDstData = (Byte*)po_pBuffer;
                while (pi_PixelCount > 0)
                    {
                    memcpy(pDstData,
                           (Byte*)ComputeAddress((uint32_t)pi_PositionX,
                                                  (uint32_t)pi_PositionY),
                           m_BytesPerPixel);

                    pDstData += m_BytesPerPixel;
                    pi_PositionX += m_DeltaX;
                    pi_PositionY += m_DeltaY;
                    pi_PixelCount--;
                    }
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// protected
// ComputeAddress
//-----------------------------------------------------------------------------
Byte* HRANearestSamplerN8::ComputeAddress(HUINTX  pi_PosX, HUINTX  pi_PosY) const
    {
    // Accept one pixel outside, remapped to last pixel.
    HPRECONDITION(pi_PosX <= m_Width);
    HPRECONDITION(pi_PosY <= m_Height);

    HPRECONDITION(m_pPacket->GetBufferAddress() != 0);

    Byte* pRawData = m_pPacket->GetBufferAddress();

    // compute the address

    // test if we are in SLO4 or SLO6
    if(m_SLO4)
        pRawData += MIN(pi_PosY, m_Height-1) * m_BytesPerLine;
    else
        pRawData += (m_Height - MIN(pi_PosY, m_Height-1) - 1) * m_BytesPerLine;

    pRawData += MIN(pi_PosX, m_Width-1) * m_BytesPerPixel;

    //:> This post condition is use to verify if we read a initialized data
    HPOSTCONDITION(pRawData <= m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize());

    return pRawData;
    }