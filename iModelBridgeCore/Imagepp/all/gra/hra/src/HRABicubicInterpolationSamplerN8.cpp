//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRABicubicInterpolationSamplerN8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRABiCubicInterpolationSamplerN8.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRAAveragingGrid.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>


/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-------------------------------------------------------------------------------*/
HRABiCubicInterpolationSamplerN8::HRABiCubicInterpolationSamplerN8(HGSMemorySurfaceDescriptor const&   pi_rMemorySurface,
                                                                   const HGF2DRectangle&               pi_rSampleDimension)
    : HRAGenericSampler(pi_rMemorySurface,
                        pi_rSampleDimension),
    m_Smoothing(-0.5)
    {
    HFCPtr<HRPPixelType> pSourcePixelType(pi_rMemorySurface->GetPixelType());

    HPRECONDITION(pSourcePixelType != 0);
    m_SourceBytesPerPixel   = pSourcePixelType->CountPixelRawDataBits() / 8;
    m_BytesPerPixel         = m_SourceBytesPerPixel;
    m_SourceBytesPerLine    = pi_rMemorySurface->GetBytesPerRow();
    m_Width                 = pi_rMemorySurface->GetWidth();
    m_Height                = pi_rMemorySurface->GetHeight();
    m_SLO4                  = pi_rMemorySurface->GetSLO() == HGF_UPPER_LEFT_HORIZONTAL;


    HPRECONDITION(pi_rMemorySurface->GetPacket() != 0);
    m_pPacket = pi_rMemorySurface->GetPacket();

    if (pSourcePixelType->CountIndexBits() > 0 || (pSourcePixelType->CountPixelRawDataBits() % 8) != 0)
        {
        // We need to change the pixeltype. Work with the palette channels as values
        if (pSourcePixelType->CountIndexBits() > 0)
            m_pWorkingPixelType = HRPPixelTypeFactory::GetInstance()->Create(pSourcePixelType->GetChannelOrg(), 0);
        else
            m_pWorkingPixelType = new HRPPixelTypeV8Gray8;  // V1Gray source data

        // Factory couldn't fullfill request. Work with generic pixeltype
        if (m_pWorkingPixelType == 0)
            {
            if (pSourcePixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelOrg::FREE)
                m_pWorkingPixelType = new HRPPixelTypeV32R8G8B8A8();
            else
                m_pWorkingPixelType = new HRPPixelTypeV24R8G8B8();
            }

        m_BytesPerPixel = m_pWorkingPixelType->CountPixelRawDataBits() / 8;

        m_pConverter = pSourcePixelType->GetConverterTo(m_pWorkingPixelType);

        // Allocate one more line, used when returning partial
        // lines in ComputeAddress.
        m_ppConvertedLines = new Byte*[m_Height+1];
        memset(m_ppConvertedLines, 0, (m_Height+1) * sizeof(Byte*));
        }

    m_a    = m_Smoothing;
    m_2a   = 2.0 * m_a;
    m_2aP3 = m_2a + 3.0;
    m_aP2  = m_a + 2.0;
    m_aP3  = m_a + 3.0;

    m_pMatrix = new Byte[16 * m_BytesPerPixel];
    }


/**----------------------------------------------------------------------------
 Destructor for this class
-------------------------------------------------------------------------------*/
HRABiCubicInterpolationSamplerN8::~HRABiCubicInterpolationSamplerN8()
    {
    }


/**----------------------------------------------------------------------------
 Get a specific pixel.

 @param pi_PosX
 @param pi_PosY

 @return const void* The current pixel.
-------------------------------------------------------------------------------*/
void* HRABiCubicInterpolationSamplerN8::GetPixel(double     pi_PosX,
                                                 double     pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0.0);
    HPRECONDITION(pi_PosY >= 0.0);
    HPRECONDITION(pi_PosX < (double)m_Width);
    HPRECONDITION(pi_PosY < (double)m_Height);


    // Clear everything just in case.
    memset(m_aTempData, 0, 4);

    double Dummy;
    double Dx = modf(pi_PosX, &Dummy);
    double Dy = modf(pi_PosY, &Dummy);


    GetMatrix(pi_PosX, pi_PosY, m_pMatrix);

    return Convoluate(Dx, Dy, m_pMatrix);
    }


/**----------------------------------------------------------------------------
 Get pixels at specific locations.

 @param pi_pPositionsX
 @param pi_pPositionsY
 @param pi_PixelCount
 @param po_pBuffer
-------------------------------------------------------------------------------*/
void HRABiCubicInterpolationSamplerN8::GetPixels(const double*  pi_pPositionsX,
                                                 const double*  pi_pPositionsY,
                                                 size_t          pi_PixelCount,
                                                 void*           po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);

    Byte* pOut= (Byte*)po_pBuffer;
    double* pPosX = pi_pPositionsX;
    double* pPosY = pi_pPositionsY;

    while (pi_PixelCount)
        {
        memcpy(pOut, GetPixel(*pPosX, *pPosY), m_BytesPerPixel);
        pOut += m_BytesPerPixel;
        pi_PixelCount--;
        }
    }


/**----------------------------------------------------------------------------
 Get pixels

 @param pi_PositionX
 @param pi_PositionY
 @param pi_DeltaX
 @param pi_DeltaY
 @param pi_PixelCount
 @param pi_pBuffer
-------------------------------------------------------------------------------*/
void HRABiCubicInterpolationSamplerN8::GetPixels(double         pi_PositionX,
                                                 double         pi_PositionY,
                                                 double         pi_DeltaX,
                                                 double         pi_DeltaY,
                                                 size_t          pi_PixelCount,
                                                 void*           po_pBuffer) const
    {
    HPRECONDITION(pi_PositionX >= 0.0);
    HPRECONDITION(pi_PositionY >= 0.0);
    HPRECONDITION(pi_PositionX + ((double)(pi_PixelCount - 1) * pi_DeltaX) >= 0.0);
    HPRECONDITION(pi_PositionX + (double)(pi_PixelCount - 1) * pi_DeltaX <= (double)m_Width);
    HPRECONDITION(pi_PositionY + ((double)(pi_PixelCount - 1) * pi_DeltaY) >= 0.0);
    HPRECONDITION(pi_PositionY + (double)(pi_PixelCount -  1) * pi_DeltaY <= (double)m_Height);

    // HChk MR: We could do better by choosing the preceding OR following line depending
    // on the received Y coordinate?


    Byte* pOut = (Byte*) po_pBuffer;

    if (!HDOUBLE_EQUAL_EPSILON(pi_DeltaY, 0.0) && !HDOUBLE_EQUAL_EPSILON(pi_DeltaY, 0.0))
        {
        while (pi_PixelCount)
            {
            memcpy(pOut, GetPixel(pi_PositionX, pi_PositionY), m_BytesPerPixel);
            pi_PositionX += pi_DeltaX;
            pi_PositionY += pi_DeltaY;
            pi_PixelCount--;
            }
        }
    else if (HDOUBLE_EQUAL_EPSILON(pi_DeltaY, 0.0))
        {

        }
    else
        {
        }

        {
        if (HDOUBLE_EQUAL_EPSILON(pi_DeltaX, 2.0) && HDOUBLE_EQUAL_EPSILON(GetSampleDimension().GetXMax() - GetSampleDimension().GetXMin(), 2.0))
            {
            // Adjust X,Y positions by 1 pixel, because stretch by two will read 2x2 pixels,
            // and the received coordinate represents the sample center.

            StretchByTwo(MAX(pi_PositionX - 1.0, 0.0), MAX(pi_PositionY - 1.0, 0.0), pi_PixelCount, (Byte*) po_pBuffer);
            }
        else
            {
            double HalfScaleIncrementX = (GetSampleDimension().GetXMax() - GetSampleDimension().GetXMin()) * 0.5;
            double HalfScaleIncrementY = (GetSampleDimension().GetYMax() - GetSampleDimension().GetYMin()) * 0.5;

            HRAAveragingGrid Grid(pi_PositionX - HalfScaleIncrementX,
                                  pi_PositionY - HalfScaleIncrementY,
                                  pi_PositionX + HalfScaleIncrementX,
                                  pi_PositionY + HalfScaleIncrementY,
                                  m_Width - 1,
                                  m_Height - 1);

            uint32_t NumberOfLines = Grid.GetHeight();
            HASSERT(NumberOfLines > 0);

            uint32_t CurrentSourcePositionX = Grid.GetXMin();

            // Compute source data pointers for each line
            Byte** ppSrcData = new Byte*[NumberOfLines];
            for (int Line = 0 ; Line < NumberOfLines ; ++Line)
                {
                ppSrcData[Line] = ComputeAddress(CurrentSourcePositionX, Grid.GetYMin() + Line);
                }

            uint32_t TotalChannel[4];
            uint32_t NumberOfColumns;
            uint32_t NumberOfPixels;

            while(pi_PixelCount > 0)
                {
                memset(&TotalChannel, 0, m_BytesPerPixel * sizeof(uint32_t));

                NumberOfColumns = Grid.GetWidth();
                NumberOfPixels = NumberOfColumns * NumberOfLines;

                Grid.TranslateX(pi_DeltaX);
                uint32_t NewSourcePositionX = Grid.GetXMin();

                // Add each useful pixel
                for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
                    {
                    for (uint32_t Column = 0 ; Column < NumberOfColumns ; ++Column)
                        {
                        for (size_t Byte = 0 ; Byte < m_BytesPerPixel ; ++Byte)
                            TotalChannel[Byte] += ppSrcData[Line][Column*m_BytesPerPixel+Byte];
                        }

                    // Adjust source data pointer for next turn
                    ppSrcData[Line] += m_BytesPerPixel * (NewSourcePositionX - CurrentSourcePositionX);
                    }

                // Set result
                for (size_t Byte = 0 ; Byte < m_BytesPerPixel ; ++Byte)
                    *pOut++ = (Byte) (TotalChannel[Byte] / NumberOfPixels);

                // Advance to next destination

                CurrentSourcePositionX = NewSourcePositionX;

                --pi_PixelCount;
                }

            delete ppSrcData;
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

            pi_PositionX += pi_DeltaX;
            pi_PositionY += pi_DeltaY;
            }

        GetPixels(pXPositions, pYPositions, pi_PixelCount, po_pBuffer);
        }
    }


/**----------------------------------------------------------------------------
 Retrieve the pixeltype in which the results of the averaging process will
 be created, if different from the source pixeltype.

 @return The output pixeltype, or NULL if no conversion is applied.
-------------------------------------------------------------------------------*/
HFCPtr<HRPPixelType> HRABiCubicInterpolationSamplerN8::GetOutputPixelType() const
    {
    return m_pWorkingPixelType;
    }


/**----------------------------------------------------------------------------
 Specify in which pixeltype we'd like to see the results of the averaging.
 This method can only be used if the sampler was already forced to convert
 the source data apply the averaging.

 @return true if the specified pixeltype will be used as output format.

 @see GetOutputPixelType()
-------------------------------------------------------------------------------*/
bool HRABiCubicInterpolationSamplerN8::TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType)
    {
    // Accept only if we were already forced to convert data, and
    // the new pixeltype has no palette.
    if (m_pWorkingPixelType != 0 &&
        pi_rpOutputPixelType->CountIndexBits() == 0)
        {
        m_pWorkingPixelType = pi_rpOutputPixelType;

        m_BytesPerPixel = m_pWorkingPixelType->CountPixelRawDataBits() / 8;

        m_pConverter = m_PixelType->GetConverterTo(m_pWorkingPixelType);

        return true;
        }
    else
        {
        return false;
        }
    }


//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------

void* HRABiCubicInterpolationSamplerN8::Convoluate(double        pi_CoefX0,
                                                   double        pi_CoefX1,
                                                   double        pi_CoefX2,
                                                   double        pi_CoefX3,
                                                   double        pi_CoefY0,
                                                   double        pi_CoefY1,
                                                   double        pi_CoefY2,
                                                   double        pi_CoefY3,
                                                   double        pi_Dx,
                                                   double        pi_Dy,
                                                   const Byte*   pi_pMatrix)
    {

    Byte* pMatrix;
    Byte* pData = m_aTempData;
    for (Byte i = 0; i < m_BytesPerPixel; i++)
        {
        pMatrix = m_pMatrix + i;
        double Hm1  = (double)*pMatrix * pi_CoefX2 +
                       (double)*(pMatrix + 3) * m_CoefX2 +
                       (double)*(pMatrix + 6) * m_CoefX1 +
                       (double)*(pMatrix + 9) * m_CoefX0;

        pMatrix++;
        double H0   = (double)*pMatrix * m_CoefX2 +
                       (double)*(pMatrix + 3) * m_CoefX2 +
                       (double)*(pMatrix + 6) * m_CoefX1 +
                       (double)*(pMatrix + 9) * m_CoefX0;

        pMatrix++;
        double Hp1  = (double)*pMatrix * m_CoefX2 +
                       (double)*(pMatrix + 3) * m_CoefX2 +
                       (double)*(pMatrix + 6) * m_CoefX1 +
                       (double)*(pMatrix + 9) * m_CoefX0;

        pMatrix++;
        double Hp2  = (double)*pMatrix * m_CoefX2 +
                       (double)*(pMatrix + 3) * m_CoefX2 +
                       (double)*(pMatrix + 6) * m_CoefX1 +
                       (double)*(pMatrix + 9) * m_CoefX0;

        Value = m_CoefY3 * Hm1 + CoefY2 * H0 + CoefY1 * Hp1 + CoefY0 * Hp2;
        *pData = (Byte)MAX(MIN(Value, 0.0), 255.0);
        pData++;
        }

    return m_aTempData;
    }

//-----------------------------------------------------------------------------
// protected
// ComputeAddress
//-----------------------------------------------------------------------------
Byte* HRABiCubicInterpolationSamplerN8::ComputeAddress(uint32_t pi_PosX,
                                                        uint32_t pi_PosY,
                                                        uint32_t pi_NeededPixels) const
    {
    HPRECONDITION(pi_PosX <= m_Width);
    HPRECONDITION(pi_PosY <= m_Height);

    pi_PosX = MIN(pi_PosX, m_Width-1);
    pi_PosY = MIN(pi_PosY, m_Height-1);

    HPRECONDITION(m_pPacket->GetBufferAddress() != 0);

    // For N1 sources, convert full lines only
    if (m_SourceBytesPerPixel == 0)
        pi_NeededPixels = ULONG_MAX;

    // Compute start of source line
    Byte* pSrcData = m_pPacket->GetBufferAddress();
    if(m_SLO4)
        pSrcData += pi_PosY * m_SourceBytesPerLine;
    else
        pSrcData += (m_Height - pi_PosY - 1) * m_SourceBytesPerLine;

    Byte* pResult;

    if (m_pWorkingPixelType != 0)
        {
        if (pi_NeededPixels == ULONG_MAX)
            {
            // Convert and store full lines.

            if (m_ppConvertedLines[pi_PosY] == 0)
                {
                m_ppConvertedLines[pi_PosY] = new Byte[m_Width * m_BytesPerPixel];
                m_pConverter->Convert(pSrcData, m_ppConvertedLines[pi_PosY], m_Width);
                }

            pResult = m_ppConvertedLines[pi_PosY];

            pResult += pi_PosX * m_BytesPerPixel;
            }
        else
            {
            if (m_ppConvertedLines[m_Height] == 0)
                {
                m_ppConvertedLines[m_Height] = new Byte[m_Width * m_BytesPerPixel];
                }

            // We only convert the necessary pixels, and we place them at
            // the start of the temp. line in the buffer.
            pSrcData += pi_PosX * m_SourceBytesPerPixel;
            m_pConverter->Convert(pSrcData, m_ppConvertedLines[m_Height], pi_NeededPixels);
            pResult = m_ppConvertedLines[m_Height];
            }
        }
    else
        {
        pResult = pSrcData;

        pResult += pi_PosX * m_SourceBytesPerPixel;

        // This post condition is use to verify if we read initialized data
        HPOSTCONDITION(pResult <= m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize());
        }

    return pResult;
    }


//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Calculate all coefficients

 @param pi_Offset
 @param po_pCoef0
 @param po_pCoef1
 @param po_pCoef2
 @param po_pCoef3

 @see GetOutputPixelType()
-------------------------------------------------------------------------------*/
void HRABiCubicInterpolationSamplerN8::GetCoefficients(double    pi_Offset,
                                                       double*   po_pCoef0,
                                                       double*   po_pCoef1,
                                                       double*   po_pCoef2,
                                                       double*   po_pCoef3) const
    {

    HPRECONDITION(po_pCoef0 != 0);
    HPRECONDITION(po_pCoef1 != 0);
    HPRECONDITION(po_pCoef2 != 0);
    HPRECONDITION(po_pCoef3 != 0);

    double T2 = pi_Offset * pi_Offset;
    double T3 = T2 * pi_Offset;

    // coefficient for x
    *po_pCoef0 = m_a * (T2 - T3);
    *po_pCoef1 = T2 * m_2aP3 - m_T3 * m_aP2 - m_a * pi_pOffset;
    *po_pCoef2 = T3 * m_aP2 - T2 * m_aP3 + 1;
    *po_pCoef3 = T3 * m_a - T2 * m_2a + m_a * pi_pOffset;

    }
