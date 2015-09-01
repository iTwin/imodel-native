//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAAverageSamplerN8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAAverageSamplerN8.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRAAveragingGrid.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>

/** ----------------------------------------------------------------------------
    Constructor for this class

    @param pi_pAttributes
    @param pi_pSurfaceImplementation
    ----------------------------------------------------------------------------
*/
HRAAverageSamplerN8::HRAAverageSamplerN8(HGSMemorySurfaceDescriptor const&     pi_rMemorySurface,
                                         const HGF2DRectangle&                 pi_rSampleDimension,
                                         double                                pi_DeltaX,
                                         double                                pi_DeltaY)
    : HRAGenericSampler(pi_rMemorySurface,
                        pi_rSampleDimension,
                        pi_DeltaX,
                        pi_DeltaY)
    {
    // compute some useful information
    HFCPtr<HRPPixelType> pSourcePixelType(pi_rMemorySurface.GetPixelType());

    HPRECONDITION(pSourcePixelType != 0);
    m_SourceBytesPerPixel = pSourcePixelType->CountPixelRawDataBits() / 8;
    m_BytesPerPixel = m_SourceBytesPerPixel;
    m_SourceBytesPerLine = pi_rMemorySurface.GetBytesPerRow();
    m_DataWidth     = pi_rMemorySurface.GetDataWidth();
    m_DataHeight    = pi_rMemorySurface.GetDataHeight();
    m_SLO4          = pi_rMemorySurface.GetSLO() == HGF_UPPER_LEFT_HORIZONTAL;


    HPRECONDITION(pi_rMemorySurface.GetPacket() != 0);
    m_pPacket = pi_rMemorySurface.GetPacket();

    if (pSourcePixelType->CountIndexBits() > 0 || pSourcePixelType->CountPixelRawDataBits() < 8)
        {
        // We need to change the pixeltype. Work with the palette channels as values
        if (pSourcePixelType->CountIndexBits() > 0)
            m_pWorkingPixelType = HRPPixelTypeFactory::GetInstance()->Create(pSourcePixelType->GetChannelOrg(), 0);
        else
            m_pWorkingPixelType = new HRPPixelTypeV8Gray8;  // V1Gray source data

        // Factory couldn't fullfill request. Work with generic pixeltype
        if (m_pWorkingPixelType == 0)
            {
            if (pSourcePixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
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

    // optimization
    m_StretchByLine     = HDOUBLE_EQUAL_EPSILON(pi_DeltaY, 0.0);
    m_StretchLineByTwo  = m_StretchByLine &&
                          HDOUBLE_EQUAL_EPSILON(pi_DeltaX, 2.0) &&
                          HDOUBLE_EQUAL_EPSILON(GetSampleDimension().GetXMax() - GetSampleDimension().GetXMin(), 2.0);
    }


/** ----------------------------------------------------------------------------
    Destructor for this class
    ----------------------------------------------------------------------------
*/
HRAAverageSamplerN8::~HRAAverageSamplerN8()
    {
    if (m_ppConvertedLines != 0)
        {
        for (size_t i = 0 ; i <= m_Height ; ++i)
            delete m_ppConvertedLines[i];
        }
    }


/** ----------------------------------------------------------------------------
    Get a specific pixel.

    @param pi_PosX
    @param pi_PosY

    @return const void* The current pixel.
    ----------------------------------------------------------------------------
*/
void const* HRAAverageSamplerN8::GetPixel(double     pi_PosX,
                                    double     pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0.0);
    HPRECONDITION(pi_PosY >= 0.0);
    HPRECONDITION(pi_PosX < (double)m_Width);
    HPRECONDITION(pi_PosY < (double)m_Height);

    // Clear everything just in case.
    memset(m_TempData, 0, sizeof(m_TempData));

    double HalfScaleIncrementX = (GetSampleDimension().GetXMax() - GetSampleDimension().GetXMin()) * 0.5;
    double HalfScaleIncrementY = (GetSampleDimension().GetYMax() - GetSampleDimension().GetYMin()) * 0.5;

    HRAAveragingGrid Grid(pi_PosX - HalfScaleIncrementX,
                          pi_PosY - HalfScaleIncrementY,
                          pi_PosX + HalfScaleIncrementX,
                          pi_PosY + HalfScaleIncrementY,
                          m_Width - 1,
                          m_Height - 1);

    uint32_t NumberOfLines = Grid.GetHeight();
    HASSERT(NumberOfLines > 0);

    uint32_t CurrentSourcePositionX = Grid.GetXMin();

    uint32_t* pTotalChannel = new uint32_t[m_BytesPerPixel];
    uint32_t NumberOfColumns;
    uint32_t NumberOfPixels;

    memset(pTotalChannel, 0, m_BytesPerPixel * sizeof(uint32_t));

    NumberOfColumns = Grid.GetWidth();
    NumberOfPixels = NumberOfColumns * NumberOfLines;

    // Add each useful pixel
    for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
        {
        Byte* pSrcData = ComputeAddress(CurrentSourcePositionX, Grid.GetYMin() + Line, NumberOfColumns);

        for (uint32_t Column = 0 ; Column < NumberOfColumns ; ++Column)
            {
            for (size_t Byte = 0 ; Byte < m_BytesPerPixel ; ++Byte)
                pTotalChannel[Byte] += pSrcData[Column*m_BytesPerPixel+Byte];
            }
        }
    // Set result
    for (size_t aByte = 0 ; aByte < m_BytesPerPixel ; ++aByte)
        m_TempData[aByte] = (Byte) (pTotalChannel[aByte] / NumberOfPixels);

    delete pTotalChannel;

    return m_TempData;
    }


/** ----------------------------------------------------------------------------
    Get pixels at specific locations.

    @param pi_pPositionsX
    @param pi_pPositionsY
    @param pi_PixelCount
    @param po_pBuffer
    ----------------------------------------------------------------------------
*/
void HRAAverageSamplerN8::GetPixels(const double*  pi_pPositionsX,
                                    const double*  pi_pPositionsY,
                                    size_t          pi_PixelCount,
                                    void*           po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);

    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaX, 0.0), L"The X scaling set into the sampler was not used\n");
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaY, 0.0), L"The Y scaling set into the sampler was not used\n");

    Byte* pOut = (Byte*) po_pBuffer;

    double HalfScaleIncrementX = (GetSampleDimension().GetXMax() - GetSampleDimension().GetXMin()) * 0.5;
    double HalfScaleIncrementY = (GetSampleDimension().GetYMax() - GetSampleDimension().GetYMin()) * 0.5;

    while (pi_PixelCount)
        {
        HRAAveragingGrid Grid(*pi_pPositionsX - HalfScaleIncrementX,
                              *pi_pPositionsY - HalfScaleIncrementY,
                              *pi_pPositionsX + HalfScaleIncrementX,
                              *pi_pPositionsY + HalfScaleIncrementY,
                              m_Width - 1,
                              m_Height - 1);

        uint32_t NumberOfLines = Grid.GetHeight();
        HASSERT(NumberOfLines > 0);

        uint32_t CurrentSourcePositionX = Grid.GetXMin();

        uint32_t* pTotalChannel = new uint32_t[m_BytesPerPixel];
        uint32_t NumberOfColumns;
        uint32_t NumberOfPixels;

        memset(pTotalChannel, 0, m_BytesPerPixel * sizeof(uint32_t));

        NumberOfColumns = Grid.GetWidth();
        NumberOfPixels = NumberOfColumns * NumberOfLines;

        // Add each useful pixel
        for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
            {
            Byte* pSrcData = ComputeAddress(CurrentSourcePositionX, Grid.GetYMin() + Line, NumberOfColumns);

            for (uint32_t Column = 0 ; Column < NumberOfColumns ; ++Column)
                {
                for (size_t Byte = 0 ; Byte < m_BytesPerPixel ; ++Byte)
                    pTotalChannel[Byte] += pSrcData[Column*m_BytesPerPixel+Byte];
                }
            }

        // Set result
        for (size_t aByte = 0 ; aByte < m_BytesPerPixel ; ++aByte)
            *pOut++ = (Byte) (pTotalChannel[aByte] / NumberOfPixels);

        --pi_PixelCount;
        ++pi_pPositionsX;
        ++pi_pPositionsY;

        delete pTotalChannel;
        }
    }


/** ----------------------------------------------------------------------------
    Get pixels

    @param pi_PositionX
    @param pi_PositionY
    @param pi_PixelCount
    @param pi_pBuffer
    ----------------------------------------------------------------------------
*/
void HRAAverageSamplerN8::GetPixels(double         pi_PositionX,
                                    double         pi_PositionY,
                                    size_t          pi_PixelCount,
                                    void*           po_pBuffer) const
    {
    HPRECONDITION(pi_PositionX >= 0.0);
    HPRECONDITION(pi_PositionY >= 0.0);
    HPRECONDITION(pi_PositionX + ((double)(pi_PixelCount - 1) * m_DeltaX) >= 0.0);
    HPRECONDITION(pi_PositionX + (double)(pi_PixelCount - 1) * m_DeltaX <= (double)m_Width + HGLOBAL_EPSILON);
    HPRECONDITION(pi_PositionY + ((double)(pi_PixelCount - 1) * m_DeltaY) >= 0.0);
    HPRECONDITION(pi_PositionY + (double)(pi_PixelCount -  1) * m_DeltaY <= (double)m_Height + HGLOBAL_EPSILON);

    // HChk MR: We could do better by choosing the preceding OR following line depending
    // on the received Y coordinate?


    Byte* pOut = (Byte*) po_pBuffer;

    if (m_StretchLineByTwo && pi_PositionY <= (double)(m_Height-1))
        {
        // Adjust X,Y positions by 1 pixel, because stretch by two will read 2x2 pixels,
        // and the received coordinate represents the sample center.

        StretchByTwo((uint32_t)MAX(pi_PositionX - 1.0, 0.0), (uint32_t)MAX(pi_PositionY - 1.0, 0.0), pi_PixelCount, (Byte*) po_pBuffer);
        }
    else if (m_StretchByLine)
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

        for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
            {
            ppSrcData[Line] = ComputeAddress(CurrentSourcePositionX, Grid.GetYMin() + Line);
            }

        uint32_t* pTotalChannel = new uint32_t[m_BytesPerPixel];
        uint32_t NumberOfColumns;
        uint32_t NumberOfPixels;

        while(pi_PixelCount > 0)
            {
            memset(pTotalChannel, 0, m_BytesPerPixel * sizeof(uint32_t));

            NumberOfColumns = Grid.GetWidth();
            NumberOfPixels = NumberOfColumns * NumberOfLines;

            Grid.TranslateX(m_DeltaX);
            uint32_t NewSourcePositionX = Grid.GetXMin();

            // Add each useful pixel
            for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
                {
                for (uint32_t Column = 0 ; Column < NumberOfColumns ; ++Column)
                    {
                    for (size_t Byte = 0 ; Byte < m_BytesPerPixel ; ++Byte)
                        pTotalChannel[Byte] += ppSrcData[Line][Column*m_BytesPerPixel+Byte];
                    }

                // Adjust source data pointer for next turn
                ppSrcData[Line] += m_BytesPerPixel * (NewSourcePositionX - CurrentSourcePositionX);
                }

            // Set result
            for (size_t aByte = 0 ; aByte < m_BytesPerPixel ; ++aByte)
                *pOut++ = (Byte) (pTotalChannel[aByte] / NumberOfPixels);

            // Advance to next destination

            CurrentSourcePositionX = NewSourcePositionX;

            --pi_PixelCount;
            }

        delete pTotalChannel;
        delete ppSrcData;
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


/** ----------------------------------------------------------------------------
    Retrieve the pixeltype in which the results of the averaging process will
    be created, if different from the source pixeltype.

    @return The output pixeltype, or NULL if no conversion is applied.
    ----------------------------------------------------------------------------
*/
HFCPtr<HRPPixelType> HRAAverageSamplerN8::GetOutputPixelType() const
    {
    return m_pWorkingPixelType;
    }


/** ----------------------------------------------------------------------------
    Specify in which pixeltype we'd like to see the results of the averaging.
    This method can only be used if the sampler was already forced to convert
    the source data apply the averaging.

    @return true if the specified pixeltype will be used as output format.

    @see GetOutputPixelType()
    ----------------------------------------------------------------------------
*/
bool HRAAverageSamplerN8::TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType)
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

//-----------------------------------------------------------------------------
// private
// StretchByTwo
// Only when dividing by 2 horizontally and vertically.
//-----------------------------------------------------------------------------
void HRAAverageSamplerN8::StretchByTwo(uint32_t pi_PositionX,
                                       uint32_t pi_PositionY,
                                       size_t  pi_PixelCount,
                                       Byte* po_pBuffer) const
    {
    HPRECONDITION(po_pBuffer != 0);

    // compute pixel over
    uint32_t PixelAvailable = (m_DataWidth - pi_PositionX) / 2;
    bool LastPixelOutside(false);

    if (PixelAvailable < pi_PixelCount)
        {
        HPRECONDITION(pi_PixelCount - PixelAvailable == 1);
        LastPixelOutside = true;
        --pi_PixelCount;
        }


    // ComputeAddress give the address of the left pixel
    // the adjustment was made by the caller
    Byte* pSrcFirstLine  = ComputeAddress(0, pi_PositionY);
    // Need to compute address because data might come from m_ppConvertedLines.
    Byte* pSrcSecondLine = (pi_PositionY >= m_DataHeight - 1) ?
                             pSrcFirstLine : ComputeAddress(0, pi_PositionY+1);
    Byte* pSrcFirstLineLeftPixel  = pSrcFirstLine + pi_PositionX * m_BytesPerPixel;
    Byte* pSrcSecondLineLeftPixel = pSrcSecondLine + pi_PositionX * m_BytesPerPixel;
    Byte* pSrcFirstLineLastPixel  = NULL;
    Byte* pSrcSecondLineLastPixel = NULL;
    Byte* pSrcFirstLineSecondToLastPixel  = NULL;
    Byte* pSrcSecondLineSecondToLastPixel = NULL;

    if (LastPixelOutside)
        {
        pSrcFirstLineLastPixel = pSrcFirstLine + (m_DataWidth - 1) * m_BytesPerPixel;
        pSrcSecondLineLastPixel = pSrcSecondLine + (m_DataWidth - 1) * m_BytesPerPixel;
        if(m_DataWidth > 1)
            {
            pSrcFirstLineSecondToLastPixel = pSrcFirstLine + (m_DataWidth - 2) * m_BytesPerPixel;
            pSrcSecondLineSecondToLastPixel = pSrcSecondLine + (m_DataWidth - 2) * m_BytesPerPixel;
            }
        else
            {
            pSrcFirstLineSecondToLastPixel = pSrcFirstLineLastPixel;
            pSrcSecondLineSecondToLastPixel = pSrcSecondLineLastPixel;
            }
        }

    size_t SrcStep = 2 * m_BytesPerPixel;

    if (m_BytesPerPixel == 4)
        {
        // Process all
        while(pi_PixelCount != 0)
            {
            po_pBuffer[0] = (pSrcFirstLineLeftPixel[0] + pSrcFirstLineLeftPixel[4] + pSrcSecondLineLeftPixel[0] + pSrcSecondLineLeftPixel[4]) >> 2;
            po_pBuffer[1] = (pSrcFirstLineLeftPixel[1] + pSrcFirstLineLeftPixel[5] + pSrcSecondLineLeftPixel[1] + pSrcSecondLineLeftPixel[5]) >> 2;
            po_pBuffer[2] = (pSrcFirstLineLeftPixel[2] + pSrcFirstLineLeftPixel[6] + pSrcSecondLineLeftPixel[2] + pSrcSecondLineLeftPixel[6]) >> 2;
            po_pBuffer[3] = (pSrcFirstLineLeftPixel[3] + pSrcFirstLineLeftPixel[7] + pSrcSecondLineLeftPixel[3] + pSrcSecondLineLeftPixel[7]) >> 2;

            pSrcFirstLineLeftPixel  += SrcStep;
            pSrcSecondLineLeftPixel += SrcStep;

            po_pBuffer              += m_BytesPerPixel;

            --pi_PixelCount;
            }

        // special case for the last one
        if (LastPixelOutside)
            {
            po_pBuffer[0] = (pSrcFirstLineSecondToLastPixel[0] + pSrcFirstLineLastPixel[0] + pSrcSecondLineSecondToLastPixel[0] + pSrcSecondLineLastPixel[0]) >> 2;
            po_pBuffer[1] = (pSrcFirstLineSecondToLastPixel[1] + pSrcFirstLineLastPixel[1] + pSrcSecondLineSecondToLastPixel[1] + pSrcSecondLineLastPixel[1]) >> 2;
            po_pBuffer[2] = (pSrcFirstLineSecondToLastPixel[2] + pSrcFirstLineLastPixel[2] + pSrcSecondLineSecondToLastPixel[2] + pSrcSecondLineLastPixel[2]) >> 2;
            po_pBuffer[3] = (pSrcFirstLineSecondToLastPixel[3] + pSrcFirstLineLastPixel[3] + pSrcSecondLineSecondToLastPixel[3] + pSrcSecondLineLastPixel[3]) >> 2;
            }
        }
    else if (m_BytesPerPixel == 3)
        {
        while(pi_PixelCount != 0)
            {
            po_pBuffer[0] = (pSrcFirstLineLeftPixel[0] + pSrcFirstLineLeftPixel[3] + pSrcSecondLineLeftPixel[0] + pSrcSecondLineLeftPixel[3]) >> 2;
            po_pBuffer[1] = (pSrcFirstLineLeftPixel[1] + pSrcFirstLineLeftPixel[4] + pSrcSecondLineLeftPixel[1] + pSrcSecondLineLeftPixel[4]) >> 2;
            po_pBuffer[2] = (pSrcFirstLineLeftPixel[2] + pSrcFirstLineLeftPixel[5] + pSrcSecondLineLeftPixel[2] + pSrcSecondLineLeftPixel[5]) >> 2;

            pSrcFirstLineLeftPixel  += SrcStep;
            pSrcSecondLineLeftPixel += SrcStep;

            po_pBuffer              += m_BytesPerPixel;

            --pi_PixelCount;
            }

        // special case for the last one
        if (LastPixelOutside)
            {
            po_pBuffer[0] = (pSrcFirstLineSecondToLastPixel[0] + pSrcFirstLineLastPixel[0] + pSrcSecondLineSecondToLastPixel[0] + pSrcSecondLineLastPixel[0]) >> 2;
            po_pBuffer[1] = (pSrcFirstLineSecondToLastPixel[1] + pSrcFirstLineLastPixel[1] + pSrcSecondLineSecondToLastPixel[1] + pSrcSecondLineLastPixel[1]) >> 2;
            po_pBuffer[2] = (pSrcFirstLineSecondToLastPixel[2] + pSrcFirstLineLastPixel[2] + pSrcSecondLineSecondToLastPixel[2] + pSrcSecondLineLastPixel[2]) >> 2;
            }
        }
    else if (m_BytesPerPixel == 2)
        {
        while(pi_PixelCount != 0)
            {
            po_pBuffer[0] = (pSrcFirstLineLeftPixel[0] + pSrcFirstLineLeftPixel[2] + pSrcSecondLineLeftPixel[0] + pSrcSecondLineLeftPixel[2]) >> 2;
            po_pBuffer[1] = (pSrcFirstLineLeftPixel[1] + pSrcFirstLineLeftPixel[3] + pSrcSecondLineLeftPixel[1] + pSrcSecondLineLeftPixel[3]) >> 2;

            pSrcFirstLineLeftPixel       += SrcStep;
            pSrcSecondLineLeftPixel    += SrcStep;

            po_pBuffer              += m_BytesPerPixel;

            --pi_PixelCount;
            }

        // special case for the last one
        if (LastPixelOutside)
            {
            po_pBuffer[0] = (pSrcFirstLineSecondToLastPixel[0] + pSrcFirstLineLastPixel[0] + pSrcSecondLineSecondToLastPixel[0] + pSrcSecondLineLastPixel[0]) >> 2;
            po_pBuffer[1] = (pSrcFirstLineSecondToLastPixel[1] + pSrcFirstLineLastPixel[1] + pSrcSecondLineSecondToLastPixel[1] + pSrcSecondLineLastPixel[1]) >> 2;
            }
        }
    else if (m_BytesPerPixel == 1)
        {
        while(pi_PixelCount != 0)
            {
            *po_pBuffer++ = (pSrcFirstLineLeftPixel[0] + pSrcFirstLineLeftPixel[1] + pSrcSecondLineLeftPixel[0] + pSrcSecondLineLeftPixel[1]) >> 2;

            pSrcFirstLineLeftPixel      += SrcStep;
            pSrcSecondLineLeftPixel     += SrcStep;

            --pi_PixelCount;
            }

        if (LastPixelOutside)
            {
            *po_pBuffer = (pSrcFirstLineSecondToLastPixel[0] + pSrcFirstLineLastPixel[0] + pSrcSecondLineSecondToLastPixel[0] + pSrcSecondLineLastPixel[0]) >> 2;
            }
        }
    else
        {
        unsigned short i;
        while(pi_PixelCount > 0)
            {
            for (i = 0; i < m_BytesPerPixel; ++i)
                {
                po_pBuffer[i] = (pSrcFirstLineLeftPixel[i] +
                                 pSrcFirstLineLeftPixel[m_BytesPerPixel + i] +
                                 pSrcSecondLineLeftPixel[i] +
                                 pSrcSecondLineLeftPixel[m_BytesPerPixel + i]) >> 2;
                }
            pSrcFirstLineLeftPixel       += SrcStep;
            pSrcSecondLineLeftPixel    += SrcStep;

            po_pBuffer              += m_BytesPerPixel;

            --pi_PixelCount;
            }

        if (LastPixelOutside)
            {
            for (i = 0; i < m_BytesPerPixel; ++i)
                {
                po_pBuffer[i] = (pSrcFirstLineSecondToLastPixel[i] +
                                 pSrcFirstLineLastPixel[i] +
                                 pSrcSecondLineSecondToLastPixel[i] +
                                 pSrcSecondLineLastPixel[i]) >> 2;
                }
            }
        }
    }


//-----------------------------------------------------------------------------
// protected
// ComputeAddress
//-----------------------------------------------------------------------------
Byte* HRAAverageSamplerN8::ComputeAddress(HUINTX  pi_PosX,
                                           HUINTX  pi_PosY,
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



