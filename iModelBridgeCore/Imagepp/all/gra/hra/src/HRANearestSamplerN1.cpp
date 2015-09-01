//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRANearestSamplerN1.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRANearestSamplerN1.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCMath.h>


/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation

 @exception HFCException The pixel type was not supported
-----------------------------------------------------------------------------*/
HRANearestSamplerN1::HRANearestSamplerN1(HGSMemorySurfaceDescriptor const&     pi_rMemorySurface,
                                         const HGF2DRectangle&                 pi_rSampleDimension,
                                         double                                pi_DeltaX,
                                         double                                pi_DeltaY)
    : HRANearestSampler(pi_rMemorySurface,
                        pi_rSampleDimension,
                        pi_DeltaX,
                        pi_DeltaY)
    {
    HPRECONDITION(pi_rMemorySurface.GetPixelType() != 0);

    m_BitsPerPixel = pi_rMemorySurface.GetPixelType()->CountPixelRawDataBits();
    m_PixelsPerByte = 8 / m_BitsPerPixel;
    m_BytesPerLine  = pi_rMemorySurface.GetBytesPerRow();
    m_DataWidth     = pi_rMemorySurface.GetDataWidth();
    m_DataHeight    = pi_rMemorySurface.GetDataHeight();
    m_SLO4          = pi_rMemorySurface.GetSLO() == HGF_UPPER_LEFT_HORIZONTAL;

    // compute a mask to extract pixels
    m_Mask = CONVERT_TO_BYTE(0xff << (8 - m_BitsPerPixel));

    HPRECONDITION(pi_rMemorySurface.GetPacket() != 0);
    m_pPacket = pi_rMemorySurface.GetPacket();

    HPOSTCONDITION((8 % m_BitsPerPixel) == 0);

    // optimization
    m_StretchByLine = HDOUBLE_EQUAL_EPSILON(pi_DeltaY, 0.0);
    m_ReverseLine = m_StretchByLine && pi_DeltaX < 0.0;
    }

/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
HRANearestSamplerN1::~HRANearestSamplerN1()
    {
    }


/**----------------------------------------------------------------------------
 Get a specific pixel.

 @param pi_PosX
 @param pi_PosY

 @return const void* The current pixel.
-----------------------------------------------------------------------------*/
void const* HRANearestSamplerN1::GetPixel(double pi_PosX, double pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0.0);
    HPRECONDITION(pi_PosY >= 0.0);
    HPRECONDITION(pi_PosX < (double)m_Width + 0.5);
    HPRECONDITION(pi_PosY < (double)m_Height + 0.5);

    HPRECONDITION(pi_PosX >= 0L);
    HPRECONDITION(pi_PosY >= 0L);

    Byte* pSrcData;
    Byte  BitIndex;
    // compute the address
    ComputeAddress((uint32_t)pi_PosX,
                   (uint32_t)pi_PosY,
                   &pSrcData,
                   &BitIndex);

    // copy the pixel value in the destination buffer
    m_TmpValue = *pSrcData << (BitIndex * m_BitsPerPixel);

    return &m_TmpValue;
    }

/**----------------------------------------------------------------------------
 Get pixels at specific location.

 @param pi_pPositionsX
 @param pi_pPositionsY
 @param pi_PixelCount
 @param po_pBuffer
-----------------------------------------------------------------------------*/
void HRANearestSamplerN1::GetPixels(const double*  pi_pPositionsX,
                                    const double*  pi_pPositionsY,
                                    size_t          pi_PixelCount,
                                    void*           po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaX, 0.0), L"The X scaling set into the sampler was not used\n");
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaY, 0.0), L"The Y scaling set into the sampler was not used\n");

    Byte* pSrcData;
    Byte  SrcBitIndex;
    Byte* pDstData = (Byte*)po_pBuffer;
    Byte  DstBitIndex = 0;
    Byte  Value;

    // initialize the output buffer to 0
    size_t NbBytes = (pi_PixelCount * m_BitsPerPixel + 7) / 8;
    memset(po_pBuffer, 0, NbBytes);

    for (size_t i = 0; i < pi_PixelCount; i++)
        {
        HPRECONDITION(pi_pPositionsX[i] >= 0.0);
        HPRECONDITION(pi_pPositionsX[i] < (double)m_Width + 0.5);
        HPRECONDITION(pi_pPositionsY[i] >= 0.0);
        HPRECONDITION(pi_pPositionsY[i] < (double)m_Height + 0.5);

        ComputeAddress((uint32_t)pi_pPositionsX[i],
                       (uint32_t)pi_pPositionsY[i],
                       &pSrcData,
                       &SrcBitIndex);

        // get the pixel value
        SrcBitIndex *= (Byte)m_BitsPerPixel;
        Value = (*pSrcData & (m_Mask >> SrcBitIndex)) << SrcBitIndex;

        // copy the pixel value into the destination
        *pDstData |= (Value >> DstBitIndex);
        DstBitIndex += (Byte)m_BitsPerPixel;

        // this condition was OK because we support only the pixeltype that fit onto a byte
        // see the constructor
        if (DstBitIndex == 8)
            {
            ++pDstData;
            DstBitIndex = 0;
            }
        }
    }

/**----------------------------------------------------------------------------
 Get pixels

 @param pi_PositionX
 @param pi_PositionY
 @param pi_PixelCount
 @param pi_pBuffer
-----------------------------------------------------------------------------*/
void HRANearestSamplerN1::GetPixels(double         pi_PositionX,
                                    double         pi_PositionY,
                                    size_t          pi_PixelCount,
                                    void*           po_pBuffer) const
    {
    HPRECONDITION(pi_PositionX >= 0.0);
    HPRECONDITION(pi_PositionY >= 0.0);

    pi_PositionY = MIN(pi_PositionY, (double)(m_DataHeight - 1));

    // intialize the destination buffer
    size_t DstSizeInBytes = (pi_PixelCount * m_BitsPerPixel) / 8;
    Byte BitIndex = (Byte)((pi_PixelCount * m_BitsPerPixel) % 8);

    if (BitIndex == 0)
        memset(po_pBuffer, 0, DstSizeInBytes);
    else
        {
        memset(po_pBuffer, 0, DstSizeInBytes);
        *((Byte*)po_pBuffer + DstSizeInBytes) &= (0xFF >> BitIndex);
        }

    if (m_StretchByLine)
        {
        double DeltaX;
        Byte* pDstData;
        Byte  DstBitIndex;
        if (m_ReverseLine)
            {
            pi_PositionX += m_DeltaX * (pi_PixelCount - 1);
            DeltaX = -m_DeltaX;
            pDstData = (Byte*)po_pBuffer + ((m_BitsPerPixel * (pi_PixelCount - 1)) / 8);
            DstBitIndex = (Byte)((m_BitsPerPixel * (pi_PixelCount - 1)) % 8);
            }
        else
            {
            DeltaX = m_DeltaX;
            pDstData = (Byte*)po_pBuffer;
            DstBitIndex = 0;
            }

        Byte* pSrcData;
        Byte SrcBitIndexStart = 0;
        ComputeAddress((uint32_t)pi_PositionX,
                       (uint32_t)pi_PositionY,
                       &pSrcData,
                       &SrcBitIndexStart);
        uint32_t SrcBitIndex = SrcBitIndexStart;

        SrcBitIndex *= m_BitsPerPixel;

        size_t DstPixelCount = pi_PixelCount;

        double SrcPosition = pi_PositionX;
        uint32_t SrcPositionInteger = (uint32_t)pi_PositionX;
        uint32_t IntegerSourceAdvance;
        Byte Value;

        while(DstPixelCount != 0)
            {
            Value = (*pSrcData & (m_Mask >> SrcBitIndex)) << SrcBitIndex;

            *pDstData |= (Value >> DstBitIndex);

            SrcPosition += DeltaX;
            if (SrcPosition >= m_DataWidth)
                SrcPosition = m_DataWidth - HGLOBAL_EPSILON;

            IntegerSourceAdvance = ((uint32_t) SrcPosition) - SrcPositionInteger;
            if (IntegerSourceAdvance > 0)
                {
                SrcBitIndex += (IntegerSourceAdvance * m_BitsPerPixel);

                SrcPositionInteger = (uint32_t) SrcPosition;

                if(SrcBitIndex >= 8)
                    {
                    pSrcData += (SrcBitIndex / 8);
                    SrcBitIndex = SrcBitIndex % 8;
                    }
                }

            --DstPixelCount;

            if (m_ReverseLine)
                {
                // this condition was OK because we support only the pixeltype that fit onto a byte
                // see the constructor
                if (DstBitIndex == 0)
                    {
                    --pDstData;
                    DstBitIndex = (Byte)(8 - m_BitsPerPixel);
                    }
                else
                    DstBitIndex -= (Byte)m_BitsPerPixel;
                }
            else
                {
                DstBitIndex += (Byte)m_BitsPerPixel;
                // this condition was OK because we support only the pixeltype that fit onto a byte
                // see the constructor
                if (DstBitIndex == 8)
                    {
                    ++pDstData;
                    DstBitIndex = 0;
                    }
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
// protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// ComputeAddress
//-----------------------------------------------------------------------------
void HRANearestSamplerN1::ComputeAddress(HUINTX     pi_PosX,
                                         HUINTX     pi_PosY,
                                         Byte**   po_ppRawData,
                                         Byte*    po_pBitIndex) const
    {
    HPRECONDITION(po_ppRawData != 0);
    HPRECONDITION(po_pBitIndex != 0);
    HPRECONDITION(m_pPacket->GetBufferAddress() != 0);

    Byte* pRawData = m_pPacket->GetBufferAddress();

    // compute the address

    // test if we are in SLO4 or SLO6
    if(m_SLO4)
        pRawData += MIN(pi_PosY, m_Height-1) * m_BytesPerLine;
    else
        pRawData += (m_Height - MIN(pi_PosY, m_Height-1) - 1) * m_BytesPerLine;

    *po_ppRawData = pRawData + (MIN(pi_PosX, m_Width-1) / m_PixelsPerByte);

    *po_pBitIndex = (Byte)(MIN(pi_PosX, m_Width-1) % m_PixelsPerByte);

    //:> This post condition is use to verify if we read a initialized data
    //:> call Clear() to initialize the buffer
    HPOSTCONDITION(*po_ppRawData <= m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize());
    }
