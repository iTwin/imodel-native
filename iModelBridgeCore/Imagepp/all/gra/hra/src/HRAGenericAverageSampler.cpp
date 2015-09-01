//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAGenericAverageSampler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRAGenericSampler.h>
#include <Imagepp/all/h/HRAGenericAverageSampler.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRPPixelConverter.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HRAAveragingGrid.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>


/////////////////////////////////////////////////////////////////////////////////////
// Class HRAGenericAverageSampler                                                  //
/////////////////////////////////////////////////////////////////////////////////////

/** ----------------------------------------------------------------------------
    Constructor for this class

    @param pi_pAttributes
    @param pi_pSurfaceImplementation
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
HRAGenericAverageSampler<T, TS>::HRAGenericAverageSampler(HGSMemorySurfaceDescriptor const&  pi_rMemorySurface,
                                                          const HGF2DRectangle&              pi_rSampleDimension,
                                                          double                             pi_DeltaX,
                                                          double                             pi_DeltaY)
    : HRAGenericSampler(pi_rMemorySurface,
                        pi_rSampleDimension,
                        pi_DeltaX,
                        pi_DeltaY)
    {
    // compute some useful information
    HPRECONDITION(pi_rMemorySurface.GetPacket() != 0);
    m_pPacket = pi_rMemorySurface.GetPacket();

    HFCPtr<HRPPixelType> pSourcePixelType = pi_rMemorySurface.GetPixelType();

    HPRECONDITION(pSourcePixelType != 0);
    m_SourceBytesPerPixel = pSourcePixelType->CountPixelRawDataBits() / 8;
    m_BytesPerPixel       = m_SourceBytesPerPixel;
    m_SourceBytesPerLine  = pi_rMemorySurface.GetBytesPerRow();
    m_NbChannels          = pSourcePixelType->GetChannelOrg().CountChannels();
    m_NbBytesPerChannel   = (unsigned short)(m_BytesPerPixel / m_NbChannels);

    m_SLO4                = pi_rMemorySurface.GetSLO() == HGF_UPPER_LEFT_HORIZONTAL;

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
    m_StretchLineByTwo  = false;
    }


/** ----------------------------------------------------------------------------
    Destructor for this class
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
HRAGenericAverageSampler<T, TS>::~HRAGenericAverageSampler()
    {
    if (m_ppConvertedLines != 0)
        {
        for (size_t i = 0 ; i <= m_Height ; ++i)
            delete m_ppConvertedLines[i];
        }
    }


/** ---------------------------------------------------------------------------
    Get data pointer. Lines will be converted (pixeltype) if necessary.
    ---------------------------------------------------------------------------
*/
template <class T, class TS>
Byte* HRAGenericAverageSampler<T, TS>::ComputeAddress(const HFCPtr<HCDPacket>& pi_rpPacket,
                                                       HUINTX                   pi_PosX,
                                                       HUINTX                   pi_PosY,
                                                       size_t                   pi_NeededPixels) const
    {
    HPRECONDITION(pi_PosX <= m_Width);
    HPRECONDITION(pi_PosY <= m_Height);

    pi_PosX = MIN(pi_PosX, m_Width - 1);
    pi_PosY = MIN(pi_PosY, m_Height - 1);

    HPRECONDITION(pi_rpPacket->GetBufferAddress() != 0);

    // For N1 sources, convert full lines only
    if (m_SourceBytesPerPixel == 0)
        pi_NeededPixels = ULONG_MAX;

    // Compute start of source line
    Byte* pSrcData = pi_rpPacket->GetBufferAddress();
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
        HPOSTCONDITION(pResult <= pi_rpPacket->GetBufferAddress() + pi_rpPacket->GetDataSize());
        }

    return pResult;
    }

/** ----------------------------------------------------------------------------
    Get a specific pixel.

    @param pi_PosX
    @param pi_PosY

    @return const void* The current pixel.
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
void const* HRAGenericAverageSampler<T, TS>::GetPixel(double pi_PosX, double pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0.0);
    HPRECONDITION(pi_PosY >= 0.0);
    HPRECONDITION(pi_PosX < (double)m_Width);
    HPRECONDITION(pi_PosY < (double)m_Height);

    // Clear everything just in case.
    memset(m_pTempData, 0, m_BytesPerPixel);

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

    TS*    pChannelsSum = new TS[m_NbChannels];
    uint32_t NumberOfColumns;
    uint32_t NumberOfPixels;

    memset(pChannelsSum, 0, sizeof(TS) * m_NbChannels);

    NumberOfColumns = Grid.GetWidth();
    NumberOfPixels  = NumberOfColumns * NumberOfLines;

    // Add each useful pixel
    for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
        {
        T* pSrcData = (T*)ComputeAddress(m_pPacket,
                                         CurrentSourcePositionX,
                                         Grid.GetYMin() + Line,
                                         NumberOfColumns);

        for (uint32_t Column = 0; Column < NumberOfColumns; ++Column)
            {
            for (size_t ChannelInd = 0; ChannelInd < m_NbChannels; ChannelInd++)
                {
                pChannelsSum[ChannelInd] += pSrcData[Column * m_NbChannels + ChannelInd];
                }
            }
        }

    T* pOut = (T*)m_pTempData;

    // Set result
    for (size_t ChannelInd = 0; ChannelInd < m_NbChannels; ChannelInd++)
        {
        *pOut++ = (T)(pChannelsSum[ChannelInd] / NumberOfPixels);
        }

    delete pChannelsSum;

    return m_pTempData;
    }


/** ----------------------------------------------------------------------------
    Get pixels at specific locations.

    @param pi_pPositionsX
    @param pi_pPositionsY
    @param pi_PixelCount
    @param po_pBuffer
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
void HRAGenericAverageSampler<T, TS>::GetPixels(const double* pi_pPositionsX,
                                                const double* pi_pPositionsY,
                                                size_t         pi_PixelCount,
                                                void*          po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);

    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaX, 0.0), L"The X scaling set into the sampler was not used\n");
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaY, 0.0), L"The Y scaling set into the sampler was not used\n");

    T* pOut = (T*)po_pBuffer;

    uint32_t CurrentSourcePositionX;
    uint32_t NumberOfColumns;
    uint32_t NumberOfPixels;
    uint32_t NumberOfLines;
    double HalfScaleIncrementX = (GetSampleDimension().GetXMax() - GetSampleDimension().GetXMin()) * 0.5;
    double HalfScaleIncrementY = (GetSampleDimension().GetYMax() - GetSampleDimension().GetYMin()) * 0.5;
    TS*     pChannelsSum = new TS[m_NbChannels];

    while (pi_PixelCount)
        {
        HRAAveragingGrid Grid(*pi_pPositionsX - HalfScaleIncrementX,
                              *pi_pPositionsY - HalfScaleIncrementY,
                              *pi_pPositionsX + HalfScaleIncrementX,
                              *pi_pPositionsY + HalfScaleIncrementY,
                              m_Width - 1,
                              m_Height - 1);

        NumberOfLines = Grid.GetHeight();
        HASSERT(NumberOfLines > 0);

        CurrentSourcePositionX = Grid.GetXMin();

        memset(pChannelsSum, 0, m_NbChannels * sizeof(TS));

        NumberOfColumns = Grid.GetWidth();
        NumberOfPixels  = NumberOfColumns * NumberOfLines;

        // Add each useful pixel
        for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
            {
            T* pSrcData = (T*)this->ComputeAddress(m_pPacket,
                                             CurrentSourcePositionX,
                                             Grid.GetYMin() + Line,
                                             NumberOfColumns);

            for (uint32_t Column = 0 ; Column < NumberOfColumns ; ++Column)
                {
                for (size_t ChannelInd = 0; ChannelInd < m_NbChannels; ChannelInd++)
                    pChannelsSum[ChannelInd] += pSrcData[Column * m_NbChannels + ChannelInd];
                }
            }

        // Set result
        for (size_t ChannelInd = 0; ChannelInd < m_NbChannels; ChannelInd++)
            {
            *pOut++ = (T)(pChannelsSum[ChannelInd] / NumberOfPixels);
            }

        --pi_PixelCount;
        ++pi_pPositionsX;
        ++pi_pPositionsY;
        }

    delete pChannelsSum;
    }


/** ----------------------------------------------------------------------------
    Get pixels

    @param pi_PositionX
    @param pi_PositionY
    @param pi_PixelCount
    @param pi_pBuffer
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
void HRAGenericAverageSampler<T, TS>::GetPixels(double pi_PositionX,
                                                double pi_PositionY,
                                                size_t  pi_PixelCount,
                                                void*   po_pBuffer) const
    {
    HPRECONDITION(pi_PositionX >= 0.0);
    HPRECONDITION(pi_PositionY >= 0.0);
    HPRECONDITION(pi_PositionX + ((double)(pi_PixelCount - 1) * m_DeltaX) >= 0.0);
    HPRECONDITION(pi_PositionX + (double)(pi_PixelCount - 1) * m_DeltaX <= (double)m_Width + HGLOBAL_EPSILON);
    HPRECONDITION(pi_PositionY + ((double)(pi_PixelCount - 1) * m_DeltaY) >= 0.0);
    HPRECONDITION(pi_PositionY + (double)(pi_PixelCount -  1) * m_DeltaY <= (double)m_Height + HGLOBAL_EPSILON);

    // HChk MR: We could do better by choosing the preceding OR following line depending
    // on the received Y coordinate?
    T* pOut = (T*)po_pBuffer;

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
        T** ppSrcData = new T*[NumberOfLines];

        for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
            {
            ppSrcData[Line] = (T*)this->ComputeAddress(m_pPacket,
                                                 CurrentSourcePositionX,
                                                 Grid.GetYMin() + Line);
            }

        TS*    pChannelsSum = new TS[m_NbChannels];
        uint32_t NewSourcePositionX;
        uint32_t NumberOfColumns;
        uint32_t NumberOfPixels;

        while(pi_PixelCount > 0)
            {
            memset(pChannelsSum, 0, m_NbChannels * sizeof(TS));

            NumberOfColumns = Grid.GetWidth();
            NumberOfPixels  = NumberOfColumns * NumberOfLines;

            Grid.TranslateX(m_DeltaX);
            NewSourcePositionX = Grid.GetXMin();

            // Add each useful pixel
            for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
                {
                for (uint32_t Column = 0 ; Column < NumberOfColumns ; ++Column)
                    {
                    for (size_t ChannelInd = 0; ChannelInd < m_NbChannels; ChannelInd++)
                        {
                        pChannelsSum[ChannelInd] += ppSrcData[Line][Column * m_NbChannels + ChannelInd];
                        }
                    }

                // Adjust source data pointer for next turn
                ppSrcData[Line] += m_NbChannels * (NewSourcePositionX - CurrentSourcePositionX);
                }

            // Set result
            for (size_t ChannelInd = 0; ChannelInd < m_NbChannels; ChannelInd++)
                {
                *pOut++ = (T)(pChannelsSum[ChannelInd] / NumberOfPixels);
                }

            // Advance to next destination

            CurrentSourcePositionX = NewSourcePositionX;

            --pi_PixelCount;
            }

        delete [] pChannelsSum;
        delete [] ppSrcData;
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
template <class T, class TS>
HFCPtr<HRPPixelType> HRAGenericAverageSampler<T, TS>::GetOutputPixelType() const
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
template <class T, class TS>
bool HRAGenericAverageSampler<T, TS>::TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType)
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
// private
// StretchByTwo
// Only when dividing by 2 horizontally and vertically.
//-----------------------------------------------------------------------------
template <class T, class TS>
void HRAGenericAverageSampler<T, TS>::StretchByTwo(uint32_t pi_PositionX,
                                                   uint32_t pi_PositionY,
                                                   size_t  pi_PixelCount,
                                                   Byte* po_pBuffer) const
    {
    //This implementation of this virtual function must never be called.
    HASSERT(0);
    }


/////////////////////////////////////////////////////////////////////////////////////
// Class HRAGenericAverageSamplerInteger                                                 //
/////////////////////////////////////////////////////////////////////////////////////

/** ----------------------------------------------------------------------------
    Constructor for this class

    @param pi_pAttributes
    @param pi_pSurfaceImplementation
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
HRAGenericAverageSamplerInteger<T, TS>::HRAGenericAverageSamplerInteger(HGSMemorySurfaceDescriptor const&  pi_rMemorySurface,
                                                                        const HGF2DRectangle&              pi_rSampleDimension,
                                                                        double                             pi_DeltaX,
                                                                        double                             pi_DeltaY)
    : HRAGenericAverageSampler<T, TS>(pi_rMemorySurface,
                                      pi_rSampleDimension,
                                      pi_DeltaX,
                                      pi_DeltaY)
    {
    m_DataWidth           = pi_rMemorySurface.GetDataWidth();
    m_DataHeight          = pi_rMemorySurface.GetDataHeight();

    // optimization
    HRAGenericAverageSampler<T, TS>::m_StretchLineByTwo = HRAGenericSampler::m_StretchByLine &&
                          HDOUBLE_EQUAL_EPSILON(pi_DeltaX, 2.0) &&
                          HDOUBLE_EQUAL_EPSILON(HRAGenericSampler::GetSampleDimension().GetXMax() - HRAGenericSampler::GetSampleDimension().GetXMin(), 2.0);
    }


/** ----------------------------------------------------------------------------
    Destructor for this class
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
HRAGenericAverageSamplerInteger<T, TS>::~HRAGenericAverageSamplerInteger()
    {
    }


//-----------------------------------------------------------------------------
// private
// StretchByTwo
// Only when dividing by 2 horizontally and vertically.
//-----------------------------------------------------------------------------
template <class T, class TS>
void HRAGenericAverageSamplerInteger<T, TS>::StretchByTwo(uint32_t pi_PositionX,
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
    T* pSrcFirstLine = (T*)this->ComputeAddress(HRAGenericAverageSampler<T, TS>::m_pPacket, 0, pi_PositionY);

    // Need to compute address because data might come from m_ppConvertedLines.
    T* pSrcSecondLine         = (pi_PositionY >= m_DataHeight - 1) ?
                                pSrcFirstLine :
                                (T*)this->ComputeAddress(HRAGenericAverageSampler<T, TS>::m_pPacket, 0, pi_PositionY + 1);

    T* pSrcFirstLineLeftPixel = pSrcFirstLine + pi_PositionX * HRAGenericAverageSampler<T, TS>::m_NbChannels;
    T* pSrcSecondLineLeftPixel = pSrcSecondLine + pi_PositionX * HRAGenericAverageSampler<T, TS>::m_NbChannels;;
    T* pSrcFirstLineLastPixel = NULL;
    T* pSrcSecondLineLastPixel = NULL;

    if (LastPixelOutside)
        {
        pSrcFirstLineLastPixel = pSrcFirstLine + (m_DataWidth - 1) * HRAGenericAverageSampler<T, TS>::m_NbChannels;
        pSrcSecondLineLastPixel = pSrcSecondLine + (m_DataWidth - 1) * HRAGenericAverageSampler<T, TS>::m_NbChannels;
        }

    size_t SrcStep = 2 * HRAGenericAverageSampler<T, TS>::m_NbChannels;
    T*     pOutBuffer = (T*)po_pBuffer;

    if (HRAGenericAverageSampler<T, TS>::m_NbChannels == 4)
        {
        // Process all
        while (pi_PixelCount != 0)
            {
            //Type cast the first element to the type that should be used to hold the sum,
            //otherwise the compiler might use a type that might not be able to hold the
            //result of the sum.
            pOutBuffer[0] = (T)(((TS)pSrcFirstLineLeftPixel[0] + pSrcFirstLineLeftPixel[4] +
                                 pSrcSecondLineLeftPixel[0] + pSrcSecondLineLeftPixel[4]) >> 2);
            pOutBuffer[1] = (T)(((TS)pSrcFirstLineLeftPixel[1] + pSrcFirstLineLeftPixel[5] +
                                 pSrcSecondLineLeftPixel[1] + pSrcSecondLineLeftPixel[5]) >> 2);
            pOutBuffer[2] = (T)(((TS)pSrcFirstLineLeftPixel[2] + pSrcFirstLineLeftPixel[6] +
                                 pSrcSecondLineLeftPixel[2] + pSrcSecondLineLeftPixel[6]) >> 2);
            pOutBuffer[3] = (T)(((TS)pSrcFirstLineLeftPixel[3] + pSrcFirstLineLeftPixel[7] +
                                 pSrcSecondLineLeftPixel[3] + pSrcSecondLineLeftPixel[7]) >> 2);

            pSrcFirstLineLeftPixel  += SrcStep;
            pSrcSecondLineLeftPixel += SrcStep;

            pOutBuffer += HRAGenericAverageSampler<T, TS>::m_NbChannels;

            --pi_PixelCount;
            }

        // special case for the last one
        if (LastPixelOutside)
            {
            pOutBuffer[0] = (T)(((TS)pSrcFirstLineLastPixel[0] + pSrcFirstLineLastPixel[4] +
                                 pSrcSecondLineLastPixel[0] + pSrcSecondLineLastPixel[4]) >> 2);
            pOutBuffer[1] = (T)(((TS)pSrcFirstLineLastPixel[1] + pSrcFirstLineLastPixel[5] +
                                 pSrcSecondLineLastPixel[1] + pSrcSecondLineLastPixel[5]) >> 2);
            pOutBuffer[2] = (T)(((TS)pSrcFirstLineLastPixel[2] + pSrcFirstLineLastPixel[6] +
                                 pSrcSecondLineLastPixel[2] + pSrcSecondLineLastPixel[6]) >> 2);
            pOutBuffer[3] = (T)(((TS)pSrcFirstLineLastPixel[3] + pSrcFirstLineLastPixel[7] +
                                 pSrcSecondLineLastPixel[3] + pSrcSecondLineLastPixel[7]) >> 2);
            }
        }
    else if (HRAGenericAverageSampler<T, TS>::m_NbChannels == 3)
        {
        while(pi_PixelCount != 0)
            {
            pOutBuffer[0] =(T)(((TS)pSrcFirstLineLeftPixel[0] + pSrcFirstLineLeftPixel[3] +
                                pSrcSecondLineLeftPixel[0] + pSrcSecondLineLeftPixel[3]) >> 2);
            pOutBuffer[1] = (T)(((TS)pSrcFirstLineLeftPixel[1] + pSrcFirstLineLeftPixel[4] +
                                 pSrcSecondLineLeftPixel[1] + pSrcSecondLineLeftPixel[4]) >> 2);
            pOutBuffer[2] = (T)(((TS)pSrcFirstLineLeftPixel[2] + pSrcFirstLineLeftPixel[5] +
                                 pSrcSecondLineLeftPixel[2] + pSrcSecondLineLeftPixel[5]) >> 2);

            pSrcFirstLineLeftPixel  += SrcStep;
            pSrcSecondLineLeftPixel += SrcStep;

            pOutBuffer += HRAGenericAverageSampler<T, TS>::m_NbChannels;

            --pi_PixelCount;
            }

        // special case for the last one
        if (LastPixelOutside)
            {
            pOutBuffer[0] = (T)(((TS)pSrcFirstLineLastPixel[0] + pSrcFirstLineLastPixel[3] +
                                 pSrcSecondLineLastPixel[0] + pSrcSecondLineLastPixel[3]) >> 2);
            pOutBuffer[1] = (T)(((TS)pSrcFirstLineLastPixel[1] + pSrcFirstLineLastPixel[4] +
                                 pSrcSecondLineLastPixel[1] + pSrcSecondLineLastPixel[4]) >> 2);
            pOutBuffer[2] = (T)(((TS)pSrcFirstLineLastPixel[2] + pSrcFirstLineLastPixel[5] +
                                 pSrcSecondLineLastPixel[2] + pSrcSecondLineLastPixel[5]) >> 2);
            }
        }
    else if (HRAGenericAverageSampler<T, TS>::m_NbChannels == 2)
        {
        while(pi_PixelCount != 0)
            {
            pOutBuffer[0] = (T)(((TS)pSrcFirstLineLeftPixel[0] + pSrcFirstLineLeftPixel[2] +
                                 pSrcSecondLineLeftPixel[0] + pSrcSecondLineLeftPixel[2]) >> 2);
            pOutBuffer[1] = (T)(((TS)pSrcFirstLineLeftPixel[1] + pSrcFirstLineLeftPixel[3] +
                                 pSrcSecondLineLeftPixel[1] + pSrcSecondLineLeftPixel[3]) >> 2);

            pSrcFirstLineLeftPixel  += SrcStep;
            pSrcSecondLineLeftPixel += SrcStep;

            pOutBuffer += HRAGenericAverageSampler<T, TS>::m_NbChannels;

            --pi_PixelCount;
            }

        // special case for the last one
        if (LastPixelOutside)
            {
            pOutBuffer[0] = (T)(((TS)pSrcFirstLineLastPixel[0] + pSrcFirstLineLastPixel[2] +
                                 pSrcSecondLineLastPixel[0] + pSrcSecondLineLastPixel[2]) >> 2);
            pOutBuffer[1] = (T)(((TS)pSrcFirstLineLastPixel[1] + pSrcFirstLineLastPixel[3] +
                                 pSrcSecondLineLastPixel[1] + pSrcSecondLineLastPixel[3]) >> 2);
            }
        }
    else if (HRAGenericAverageSampler<T, TS>::m_NbChannels == 1)
        {
        while(pi_PixelCount != 0)
            {
            *pOutBuffer++ = (T)(((TS)pSrcFirstLineLeftPixel[0] + pSrcFirstLineLeftPixel[1] +
                                 pSrcSecondLineLeftPixel[0] + pSrcSecondLineLeftPixel[1]) >> 2);

            pSrcFirstLineLeftPixel  += SrcStep;
            pSrcSecondLineLeftPixel += SrcStep;

            --pi_PixelCount;
            }

        if (LastPixelOutside)
            {
            *pOutBuffer = (T)(((TS)pSrcFirstLineLastPixel[0] + pSrcFirstLineLastPixel[1] +
                               pSrcSecondLineLastPixel[0] + pSrcSecondLineLastPixel[1]) >> 2);
            }
        }
    else
        {
        unsigned short ChannelInd;
        while(pi_PixelCount > 0)
            {
            for (ChannelInd = 0; ChannelInd < HRAGenericAverageSampler<T, TS>::m_NbChannels; ChannelInd++)
                {
                pOutBuffer[ChannelInd] = (T)(((TS)pSrcFirstLineLeftPixel[ChannelInd] +
                                              pSrcFirstLineLeftPixel[HRAGenericAverageSampler<T, TS>::m_NbChannels + ChannelInd] +
                                              pSrcSecondLineLeftPixel[ChannelInd] +
                                              pSrcSecondLineLeftPixel[HRAGenericAverageSampler<T, TS>::m_NbChannels + ChannelInd]) >> 2);
                }
            pSrcFirstLineLeftPixel  += SrcStep;
            pSrcSecondLineLeftPixel += SrcStep;

            pOutBuffer += HRAGenericAverageSampler<T, TS>::m_NbChannels;

            --pi_PixelCount;
            }

        if (LastPixelOutside)
            {
            for (ChannelInd = 0; ChannelInd < HRAGenericAverageSampler<T, TS>::m_NbChannels; ChannelInd++)
                {
                pOutBuffer[ChannelInd] = (T)(((TS)pSrcFirstLineLastPixel[ChannelInd] +
                                              pSrcFirstLineLastPixel[HRAGenericAverageSampler<T, TS>::m_NbChannels + ChannelInd] +
                                              pSrcSecondLineLastPixel[ChannelInd] +
                                              pSrcSecondLineLastPixel[HRAGenericAverageSampler<T, TS>::m_NbChannels + ChannelInd]) >> 2);
                }
            }
        }
    }


/////////////////////////////////////////////////////////////////////////////////////
// Class HRAGenericAverageSparseDataSampler                                        //
//                                                                                 //
// This class should for data which can contains pixel which shouldn't be used     //
// during the averaging. For example, a DEM often contains pixels for which an     //
// elevation could not have been measured (the pixel represents no particular      //
// data).                                                                          //
/////////////////////////////////////////////////////////////////////////////////////

/** ----------------------------------------------------------------------------
    Constructor for this class

    @param pi_pAttributes
    @param pi_pSurfaceImplementation
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
HRAGenericAverageSparseDataSampler<T, TS>::HRAGenericAverageSparseDataSampler(
    HGSMemorySurfaceDescriptor const&  pi_rMemorySurface,
    const HGF2DRectangle&              pi_rSampleDimension,
    double                             pi_DeltaX,
    double                             pi_DeltaY)
    : HRAGenericAverageSampler<T, TS>(pi_rMemorySurface,
                                      pi_rSampleDimension,
                                      pi_DeltaX,
                                      pi_DeltaY)
    {
    /*
    HPRECONDITION((const HFCPtr<HGSMemorySurfaceDescriptor>&)pi_pSurfaceImplementation->
                  GetSurfaceDescriptor()->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->
                  GetNoDataValue() != 0);
      */
    // compute some useful information

    HFCPtr<HRPPixelType> pSourcePixelType = pi_rMemorySurface.GetPixelType();

    m_NoDataValue = (T)*(pSourcePixelType->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue());
    }


/** ----------------------------------------------------------------------------
    Destructor for this class
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
HRAGenericAverageSparseDataSampler<T, TS>::~HRAGenericAverageSparseDataSampler()
    {
    }


/** ----------------------------------------------------------------------------
    Get a specific pixel.

    @param pi_PosX
    @param pi_PosY

    @return const void* The current pixel.
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
void const* HRAGenericAverageSparseDataSampler<T, TS>::GetPixel(double pi_PosX,
                                                          double pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0.0);
    HPRECONDITION(pi_PosY >= 0.0);
    HPRECONDITION_T(pi_PosX < (double)HRAGenericAverageSampler<T , TS>::m_Width);
    HPRECONDITION_T(pi_PosY < (double)HRAGenericAverageSampler<T,TS>::m_Height);

    // Clear everything just in case.
    memset(HRAGenericAverageSampler<T, TS>::m_pTempData, 0, HRAGenericAverageSampler<T, TS>::m_BytesPerPixel);

    double HalfScaleIncrementX = (HRAGenericSampler::GetSampleDimension().GetXMax() - HRAGenericSampler::GetSampleDimension().GetXMin()) * 0.5;
    double HalfScaleIncrementY = (HRAGenericSampler::GetSampleDimension().GetYMax() - HRAGenericSampler::GetSampleDimension().GetYMin()) * 0.5;

    HRAAveragingGrid Grid(pi_PosX - HalfScaleIncrementX,
                          pi_PosY - HalfScaleIncrementY,
                          pi_PosX + HalfScaleIncrementX,
                          pi_PosY + HalfScaleIncrementY,
                          HRAGenericAverageSampler<T, TS>::m_Width - 1,
                          HRAGenericAverageSampler<T, TS>::m_Height - 1);

    uint32_t NumberOfLines = Grid.GetHeight();
    HASSERT(NumberOfLines > 0);

    uint32_t CurrentSourcePositionX = Grid.GetXMin();
    TS*     pChannelsSum = new TS[HRAGenericAverageSampler<T, TS>::m_NbChannels];
    uint32_t* pNumberOfValidPixels = new uint32_t[HRAGenericAverageSampler<T, TS>::m_NbChannels];
    uint32_t NumberOfColumns;
    uint32_t NumberOfPixels;
    T       SrcData;

    memset(pChannelsSum, 0, sizeof(TS)* HRAGenericAverageSampler<T, TS>::m_NbChannels);
    memset(pNumberOfValidPixels, 0, sizeof(uint32_t)* HRAGenericAverageSampler<T, TS>::m_NbChannels);

    NumberOfColumns = Grid.GetWidth();
    NumberOfPixels  = NumberOfColumns * NumberOfLines;

    // Add each useful pixel
    for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
        {
        T* pSrcData = (T*)this->ComputeAddress(HRAGenericAverageSampler<T, TS>::m_pPacket,
                                         CurrentSourcePositionX,
                                         Grid.GetYMin() + Line,
                                         NumberOfColumns);

        for (uint32_t Column = 0; Column < NumberOfColumns; ++Column)
            {
            for (size_t ChannelInd = 0; ChannelInd < HRAGenericAverageSampler<T, TS>::m_NbChannels; ChannelInd++)
                {
                SrcData = pSrcData[Column * HRAGenericAverageSampler<T, TS>::m_NbChannels + ChannelInd];

                if (SrcData != m_NoDataValue)
                    {
                    pChannelsSum[ChannelInd] += SrcData;
                    pNumberOfValidPixels[ChannelInd]++;
                    }
                }
            }
        }

    T* pOut = (T*)HRAGenericAverageSampler<T, TS>::m_pTempData;

    // Set result
    for (size_t ChannelInd = 0; ChannelInd < HRAGenericAverageSampler<T, TS>::m_NbChannels; ChannelInd++)
        {
        if (pNumberOfValidPixels[ChannelInd] == 0)
            {
            *pOut++ = m_NoDataValue;
            }
        else
            {
            *pOut++ = (T)(pChannelsSum[ChannelInd] / pNumberOfValidPixels[ChannelInd]);
            }
        }

    delete [] pChannelsSum;
    delete [] pNumberOfValidPixels;

    return HRAGenericAverageSampler<T, TS>::m_pTempData;
    }


/** ----------------------------------------------------------------------------
    Get pixels at specific locations.

    @param pi_pPositionsX
    @param pi_pPositionsY
    @param pi_PixelCount
    @param po_pBuffer
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
void HRAGenericAverageSparseDataSampler<T, TS>::GetPixels(const double* pi_pPositionsX,
                                                          const double* pi_pPositionsY,
                                                          size_t         pi_PixelCount,
                                                          void*          po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);
    
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaX, 0.0), L"The X scaling set into the sampler was not used\n");
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaY, 0.0), L"The Y scaling set into the sampler was not used\n");

    T* pOut = (T*)po_pBuffer;

    uint32_t CurrentSourcePositionX;
    uint32_t NumberOfColumns;
    uint32_t NumberOfPixels;
    uint32_t NumberOfLines;
    T       SrcData;
    double HalfScaleIncrementX = (HRAGenericSampler::GetSampleDimension().GetXMax() - HRAGenericSampler::GetSampleDimension().GetXMin()) * 0.5;
    double HalfScaleIncrementY = (HRAGenericSampler::GetSampleDimension().GetYMax() - HRAGenericSampler::GetSampleDimension().GetYMin()) * 0.5;
    TS*     pChannelsSum = new TS[HRAGenericAverageSampler<T, TS>::m_NbChannels];
    uint32_t* pNumberOfValidPixels = new uint32_t[HRAGenericAverageSampler<T, TS>::m_NbChannels];

    while (pi_PixelCount)
        {
        HRAAveragingGrid Grid(*pi_pPositionsX - HalfScaleIncrementX,
                              *pi_pPositionsY - HalfScaleIncrementY,
                              *pi_pPositionsX + HalfScaleIncrementX,
                              *pi_pPositionsY + HalfScaleIncrementY,
                              HRAGenericAverageSampler<T, TS>::m_Width - 1,
                              HRAGenericAverageSampler<T, TS>::m_Height - 1);

        NumberOfLines = Grid.GetHeight();
        HASSERT(NumberOfLines > 0);

        CurrentSourcePositionX = Grid.GetXMin();

        memset(pChannelsSum, 0, HRAGenericAverageSampler<T, TS>::m_NbChannels * sizeof(TS));
        memset(pNumberOfValidPixels, 0, HRAGenericAverageSampler<T, TS>::m_NbChannels * sizeof(uint32_t));

        NumberOfColumns = Grid.GetWidth();
        NumberOfPixels  = NumberOfColumns * NumberOfLines;

        // Add each useful pixel
        for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
            {
            T* pSrcData = (T*)this->ComputeAddress(HRAGenericAverageSampler<T, TS>::m_pPacket,
                                             CurrentSourcePositionX,
                                             Grid.GetYMin() + Line,
                                             NumberOfColumns);

            for (uint32_t Column = 0 ; Column < NumberOfColumns ; ++Column)
                {
                for (size_t ChannelInd = 0; ChannelInd < HRAGenericAverageSampler<T, TS>::m_NbChannels; ChannelInd++)
                    {
                    SrcData = pSrcData[Column * HRAGenericAverageSampler<T, TS>::m_NbChannels + ChannelInd];

                    if (SrcData != m_NoDataValue)
                        {
                        pChannelsSum[ChannelInd] += SrcData;
                        pNumberOfValidPixels[ChannelInd]++;
                        }
                    }
                }
            }

        // Set result
        for (size_t ChannelInd = 0; ChannelInd < HRAGenericAverageSampler<T, TS>::m_NbChannels; ChannelInd++)
            {
            if (pNumberOfValidPixels[ChannelInd] == 0)
                {
                *pOut++ = m_NoDataValue;
                }
            else
                {
                *pOut++ = (T)(pChannelsSum[ChannelInd] / pNumberOfValidPixels[ChannelInd]);
                }
            }

        --pi_PixelCount;
        ++pi_pPositionsX;
        ++pi_pPositionsY;
        }

    delete [] pChannelsSum;
    delete [] pNumberOfValidPixels;
    }


/** ----------------------------------------------------------------------------
    Get pixels

    @param pi_PositionX
    @param pi_PositionY
    @param pi_PixelCount
    @param pi_pBuffer
    ----------------------------------------------------------------------------
*/
template <class T, class TS>
void HRAGenericAverageSparseDataSampler<T, TS>::GetPixels(double pi_PositionX,
                                                          double pi_PositionY,
                                                          size_t  pi_PixelCount,
                                                          void*   po_pBuffer) const
    {
    HPRECONDITION(pi_PositionX >= 0.0);
    HPRECONDITION(pi_PositionY >= 0.0);
    HPRECONDITION_T(pi_PositionX + ((double)(pi_PixelCount - 1) * HRAGenericAverageSampler<T,TS>::m_DeltaX) >= 0.0);
    HPRECONDITION_T(pi_PositionX + (double)(pi_PixelCount - 1) * HRAGenericAverageSampler<T,TS>::m_DeltaX <= (double)HRAGenericAverageSampler<T,TS>::m_Width + HGLOBAL_EPSILON);
    HPRECONDITION_T(pi_PositionY + ((double)(pi_PixelCount - 1) * HRAGenericAverageSampler<T,TS>::m_DeltaY) >= 0.0);
    HPRECONDITION_T(pi_PositionY + (double)(pi_PixelCount - 1) * HRAGenericAverageSampler<T,TS>::m_DeltaY <= (double)HRAGenericAverageSampler<T,TS>::m_Height + HGLOBAL_EPSILON);
    HPRECONDITION_T(HRAGenericAverageSampler<T,TS>::m_StretchLineByTwo == false);

    // HChk MR: We could do better by choosing the preceding OR following line depending
    // on the received Y coordinate?
    T* pOut = (T*)po_pBuffer;

    if (HRAGenericAverageSampler<T, TS>::m_StretchByLine)
        {
        double HalfScaleIncrementX = (HRAGenericSampler::GetSampleDimension().GetXMax() - HRAGenericSampler::GetSampleDimension().GetXMin()) * 0.5;
        double HalfScaleIncrementY = (HRAGenericSampler::GetSampleDimension().GetYMax() - HRAGenericSampler::GetSampleDimension().GetYMin()) * 0.5;

        HRAAveragingGrid Grid(pi_PositionX - HalfScaleIncrementX,
                              pi_PositionY - HalfScaleIncrementY,
                              pi_PositionX + HalfScaleIncrementX,
                              pi_PositionY + HalfScaleIncrementY,
                              HRAGenericAverageSampler<T, TS>::m_Width - 1,
                              HRAGenericAverageSampler<T, TS>::m_Height - 1);

        uint32_t NumberOfLines = Grid.GetHeight();
        HASSERT(NumberOfLines > 0);

        uint32_t CurrentSourcePositionX = Grid.GetXMin();

        // Compute source data pointers for each line
        T** ppSrcData = new T*[NumberOfLines];

        for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
            {
            ppSrcData[Line] = (T*)this->ComputeAddress(HRAGenericAverageSampler<T, TS>::m_pPacket,
                                                 CurrentSourcePositionX,
                                                 Grid.GetYMin() + Line);
            }

        TS*     pChannelsSum = new TS[HRAGenericAverageSampler<T, TS>::m_NbChannels];
        uint32_t* pNumberOfValidPixels = new uint32_t[HRAGenericAverageSampler<T, TS>::m_NbChannels];
        uint32_t NewSourcePositionX;
        uint32_t NumberOfColumns;
        uint32_t NumberOfPixels;
        T       SrcData;

        while(pi_PixelCount > 0)
            {
            memset(pChannelsSum, 0, HRAGenericAverageSampler<T, TS>::m_NbChannels * sizeof(TS));
            memset(pNumberOfValidPixels, 0, HRAGenericAverageSampler<T, TS>::m_NbChannels * sizeof(uint32_t));

            NumberOfColumns = Grid.GetWidth();
            NumberOfPixels  = NumberOfColumns * NumberOfLines;

            Grid.TranslateX(HRAGenericAverageSampler<T, TS>::m_DeltaX);
            NewSourcePositionX = Grid.GetXMin();

            // Add each useful pixel
            for (uint32_t Line = 0 ; Line < NumberOfLines ; ++Line)
                {
                for (uint32_t Column = 0 ; Column < NumberOfColumns ; ++Column)
                    {
                    for (size_t ChannelInd = 0; ChannelInd < HRAGenericAverageSampler<T, TS>::m_NbChannels; ChannelInd++)
                        {
                        SrcData = ppSrcData[Line][Column * HRAGenericAverageSampler<T, TS>::m_NbChannels + ChannelInd];

                        if (SrcData != m_NoDataValue)
                            {
                            pChannelsSum[ChannelInd] += SrcData;
                            pNumberOfValidPixels[ChannelInd]++;
                            }
                        }
                    }

                // Adjust source data pointer for next turn
                ppSrcData[Line] += HRAGenericAverageSampler<T, TS>::m_NbChannels * (NewSourcePositionX - CurrentSourcePositionX);
                }

            // Set result
            for (size_t ChannelInd = 0; ChannelInd < HRAGenericAverageSampler<T, TS>::m_NbChannels; ChannelInd++)
                {
                if (pNumberOfValidPixels[ChannelInd] == 0)
                    {
                    *pOut++ = m_NoDataValue;
                    }
                else
                    {
                    *pOut++ = (T)(pChannelsSum[ChannelInd] / pNumberOfValidPixels[ChannelInd]);
                    }
                }

            // Advance to next destination

            CurrentSourcePositionX = NewSourcePositionX;

            --pi_PixelCount;
            }

        delete [] pChannelsSum;
        delete [] ppSrcData;
        delete [] pNumberOfValidPixels;
        }
    else
        {
        HAutoPtr<double> pXPositions(new double[pi_PixelCount]);
        HAutoPtr<double> pYPositions(new double[pi_PixelCount]);

        for (size_t Position = 0 ; Position < pi_PixelCount ; ++Position)
            {
            pXPositions[Position] = pi_PositionX;
            pYPositions[Position] = pi_PositionY;

            pi_PositionX += HRAGenericAverageSampler<T, TS>::m_DeltaX;
            pi_PositionY += HRAGenericAverageSampler<T, TS>::m_DeltaY;
            }
        GetPixels(pXPositions, pYPositions, pi_PixelCount, po_pBuffer);
        }
    }


template class HRAGenericAverageSamplerInteger<short, int64_t>;
template class HRAGenericAverageSampler<float,  double>;
template class HRAGenericAverageSparseDataSampler<float,  double>;
template class HRAGenericAverageSparseDataSampler<short,  int64_t>;
