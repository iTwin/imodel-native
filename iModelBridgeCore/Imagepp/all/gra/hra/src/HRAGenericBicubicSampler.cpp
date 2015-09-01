//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAGenericBicubicSampler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAGenericBicubicSampler.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>


// Smoothing precomputing
template <class T> const double HRAGenericBicubicSampler<T>::s_a(-0.5);
template <class T> const double HRAGenericBicubicSampler<T>::s_2a(2.0 * s_a);
template <class T> const double HRAGenericBicubicSampler<T>::s_2aP3(s_2a + 3.0);
template <class T> const double HRAGenericBicubicSampler<T>::s_aP2(s_a + 2.0);
template <class T> const double HRAGenericBicubicSampler<T>::s_aP3(s_a + 3.0);

/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-----------------------------------------------------------------------------*/
template <class T>
HRAGenericBicubicSampler<T>::HRAGenericBicubicSampler(HGSMemorySurfaceDescriptor const&   pi_rMemorySurface,
                                                      const HGF2DRectangle&               pi_rSampleDimension,
                                                      double                              pi_DeltaX,
                                                      double                              pi_DeltaY)
    : HRAGenericSampler(pi_rMemorySurface,
                        pi_rSampleDimension,
                        pi_DeltaX,
                        pi_DeltaY)
    {
    HPRECONDITION(pi_rMemorySurface->GetPacket() != 0);
    m_pPacket = pi_rMemorySurface->GetPacket();

    //numeric_limits<T>::min() returns the smallest representable value
    //for decimal type instead of greatest negative value representable.
    if (numeric_limits<T>::is_exact == true)
        {
        m_ChannelMinValue = std::numeric_limits<T>::min();
        }
    else
        {
        m_ChannelMinValue = -std::numeric_limits<T>::max();
        }

    m_ChannelMaxValue = std::numeric_limits<T>::max();
    }

/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
template <class T>
HRAGenericBicubicSampler<T>::~HRAGenericBicubicSampler()
    {
    }


/**----------------------------------------------------------------------------
 Get a specific pixel.

 @param pi_PosX
 @param pi_PosY

 @return const void* The current pixel.
-----------------------------------------------------------------------------*/
template <class T>
void* HRAGenericBicubicSampler<T>::GetPixel(double pi_PosX,
                                            double pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0.0);
    HPRECONDITION(pi_PosY >= 0.0);
    HPRECONDITION(pi_PosX <= (double)m_Width + 0.5);
    HPRECONDITION(pi_PosY <= (double)m_Height + 0.5);

    // Clear everything just in case.
    memset(m_pTempData, 0, sizeof(m_pTempData));

    Sample CurrentSample(pi_PosX, pi_PosY);

    const uint32_t Column0 = CurrentSample.GetColumn0();
    const uint32_t Column1 = CurrentSample.GetColumn1();

    // Obtain pointers to start of each line
    T* pSrcLine0 = (T*)ComputeAddress(m_pPacket,
                                      Column0,
                                      CurrentSample.GetLine0());
    T* pSrcLine1 = (T*)ComputeAddress(m_pPacket,
                                      Column0,
                                      MIN(CurrentSample.GetLine1(), m_Height-1));
    T* pSrcLine2 = (T*)ComputeAddress(m_pPacket,
                                      Column0,
                                      MIN(CurrentSample.GetLine2(), m_Height-1));
    T* pSrcLine3 = (T*)ComputeAddress(m_pPacket,
                                      Column0,
                                      MIN(CurrentSample.GetLine3(), m_Height-1));

    // Compute byte offsets for each column
    int OffsetColumn1;
    int OffsetColumn2;
    int OffsetColumn3;
    if (Column1 > Column0)
        {
        HASSERT(Column0 < m_Width - 1);
        OffsetColumn1 = m_NbChannels;
        }
    else
        OffsetColumn1 = 0;
    if (CurrentSample.GetColumn2() > Column1 && Column1 < m_Width - 1)
        OffsetColumn2 = OffsetColumn1 + m_NbChannels;
    else
        OffsetColumn2 = OffsetColumn1;
    if (CurrentSample.GetColumn3() < m_Width)
        OffsetColumn3 = OffsetColumn2 + m_NbChannels;
    else
        OffsetColumn3 = OffsetColumn2;

    double Dx = CurrentSample.GetXDelta();
    double Dy = CurrentSample.GetYDelta();
    HASSERT(Dx >= 0.0 && Dx <= 1.0);
    HASSERT(Dy >= 0.0 && Dy <= 1.0);

    double T2, T3;
    double CoefX0, CoefX1, CoefX2, CoefX3;
    double CoefY0, CoefY1, CoefY2, CoefY3;
    double Horiz0, Horiz1, Horiz2, Horiz3;

    // calculate the coefficients
    T2 = Dx * Dx;
    T3 = Dx * T2;

    // coeficient for X
    CoefX0 = T3 * s_a - T2 * s_2a + s_a * Dx;
    CoefX1 = T3 * s_aP2 - T2 * s_aP3 + 1;
    CoefX2 = T2 * s_2aP3 - T3 * s_aP2 - s_a * Dx;
    CoefX3 = s_a * (T2 - T3);

    // coeficient for Y
    T2 = Dy * Dy;
    T3 = Dy * T2;
    CoefY0 = T3 * s_a - T2 * s_2a + s_a * Dy;
    CoefY1 = T3 * s_aP2 - T2 * s_aP3 + 1;
    CoefY2 = T2 * s_2aP3 - T3 * s_aP2 - s_a * Dy;
    CoefY3 = s_a * (T2 - T3);

    double ChannelResult;

    for (unsigned short ChannelInd = 0; ChannelInd < m_NbChannels; ChannelInd++)
        {
        Horiz0 = pSrcLine0[ChannelInd] * CoefX0 +
                 pSrcLine0[ChannelInd + OffsetColumn1] * CoefX1 +
                 pSrcLine0[ChannelInd + OffsetColumn2] * CoefX2 +
                 pSrcLine0[ChannelInd + OffsetColumn3] * CoefX3;
        Horiz1 = pSrcLine1[ChannelInd] * CoefX0 +
                 pSrcLine1[ChannelInd + OffsetColumn1] * CoefX1 +
                 pSrcLine1[ChannelInd + OffsetColumn2] * CoefX2 +
                 pSrcLine1[ChannelInd + OffsetColumn3] * CoefX3;
        Horiz2 = pSrcLine2[ChannelInd] * CoefX0 +
                 pSrcLine2[ChannelInd + OffsetColumn1] * CoefX1 +
                 pSrcLine2[ChannelInd + OffsetColumn2] * CoefX2 +
                 pSrcLine2[ChannelInd + OffsetColumn3] * CoefX3;
        Horiz3 = pSrcLine3[ChannelInd] * CoefX0 +
                 pSrcLine3[ChannelInd + OffsetColumn1] * CoefX1 +
                 pSrcLine3[ChannelInd + OffsetColumn2] * CoefX2 +
                 pSrcLine3[ChannelInd + OffsetColumn3] * CoefX3;

        ChannelResult = Horiz0 * CoefY0 + Horiz1 * CoefY1 + Horiz2 * CoefY2 + Horiz3 * CoefY3;

        m_pTempData[ChannelInd] = (T)MIN(MAX(ChannelResult, (double)m_ChannelMinValue),
                                         (double)m_ChannelMaxValue);
        }

    return m_pTempData;
    }

/**----------------------------------------------------------------------------
 Get pixels at specific location.

 @param pi_pPositionsX
 @param pi_pPositionsY
 @param pi_PixelCount
 @param po_pBuffer
-----------------------------------------------------------------------------*/
template <class T>
void HRAGenericBicubicSampler<T>::GetPixels(const double* pi_pPositionsX,
                                            const double* pi_pPositionsY,
                                            size_t         pi_PixelCount,
                                            void*          po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaX, 0.0), L"The X scaling set into the sampler was not used\n");
    HWARNING(HDOUBLE_EQUAL_EPSILON(m_DeltaY, 0.0), L"The Y scaling set into the sampler was not used\n");

    const double* pPosX = pi_pPositionsX;
    const double* pPosY = pi_pPositionsY;
    T*             pOut = (T*)po_pBuffer;

    double T2, T3;
    double CoefX0, CoefX1, CoefX2, CoefX3;
    double CoefY0, CoefY1, CoefY2, CoefY3;
    double Horiz0, Horiz1, Horiz2, Horiz3;
    double Dx, Dy;
    double ChannelResult;
    uint32_t Column0;
    T*      pSrcLine0;
    T*      pSrcLine1;
    T*      pSrcLine2;
    T*      pSrcLine3;
    Sample  CurrentSample;
    int     OffsetColumn1;
    int     OffsetColumn2;
    int     OffsetColumn3;

    while (pi_PixelCount)
        {
        CurrentSample.SetPosition(*pPosX, *pPosY);

        Column0 = CurrentSample.GetColumn0();
        Column1 = CurrentSample.GetColumn1();

        // Obtain pointers to start of each line
        pSrcLine0 = (T*)ComputeAddress(m_pPacket,
                                       Column0,
                                       CurrentSample.GetLine0());
        pSrcLine1 = (T*)ComputeAddress(m_pPacket,
                                       Column0,
                                       MIN(CurrentSample.GetLine1(), m_Height - 1));
        pSrcLine2 = (T*)ComputeAddress(m_pPacket,
                                       Column0,
                                       MIN(CurrentSample.GetLine2(), m_Height - 1));
        pSrcLine3 = (T*)ComputeAddress(m_pPacket,
                                       Column0,
                                       MIN(CurrentSample.GetLine3(), m_Height - 1));

        // Compute byte offsets for each column
        if (Column1 > Column0)
            {
            HASSERT(Column0 < m_Width - 1);
            OffsetColumn1 = m_NbChannels;
            }
        else
            OffsetColumn1 = 0;
        if (CurrentSample.GetColumn2() > Column1 && Column1 < m_Width - 1)
            OffsetColumn2 = OffsetColumn1 + m_NbChannels;
        else
            OffsetColumn2 = OffsetColumn1;
        if (CurrentSample.GetColumn3() < m_Width)
            OffsetColumn3 = OffsetColumn2 + m_NbChannels;
        else
            OffsetColumn3 = OffsetColumn2;

        Dx = CurrentSample.GetXDelta();
        Dy = CurrentSample.GetYDelta();
        HASSERT(Dx >= 0.0 && Dx <= 1.0);
        HASSERT(Dy >= 0.0 && Dy <= 1.0);

        // calculate the coefficients
        T2 = Dx * Dx;
        T3 = Dx * T2;

        // coeficient for X
        CoefX0 = T3 * s_a - T2 * s_2a + s_a * Dx;
        CoefX1 = T3 * s_aP2 - T2 * s_aP3 + 1;
        CoefX2 = T2 * s_2aP3 - T3 * s_aP2 - s_a * Dx;
        CoefX3 = s_a * (T2 - T3);

        // coeficient for Y
        T2 = Dy * Dy;
        T3 = Dy * T2;
        CoefY0 = T3 * s_a - T2 * s_2a + s_a * Dy;
        CoefY1 = T3 * s_aP2 - T2 * s_aP3 + 1;
        CoefY2 = T2 * s_2aP3 - T3 * s_aP2 - s_a * Dy;
        CoefY3 = s_a * (T2 - T3);

        for (size_t ChannelInd = 0; ChannelInd < m_NbChannels; ChannelInd++)
            {
            Horiz0 = pSrcLine0[ChannelInd] * CoefX0 +
                     pSrcLine0[ChannelInd + OffsetColumn1] * CoefX1 +
                     pSrcLine0[ChannelInd + OffsetColumn2] * CoefX2 +
                     pSrcLine0[ChannelInd + OffsetColumn3] * CoefX3;
            Horiz1 = pSrcLine1[ChannelInd] * CoefX0 +
                     pSrcLine1[ChannelInd + OffsetColumn1] * CoefX1 +
                     pSrcLine1[ChannelInd + OffsetColumn2] * CoefX2 +
                     pSrcLine1[ChannelInd + OffsetColumn3] * CoefX3;
            Horiz2 = pSrcLine2[ChannelInd] * CoefX0 +
                     pSrcLine2[ChannelInd + OffsetColumn1] * CoefX1 +
                     pSrcLine2[ChannelInd + OffsetColumn2] * CoefX2 +
                     pSrcLine2[ChannelInd + OffsetColumn3] * CoefX3;
            Horiz3 = pSrcLine3[ChannelInd] * CoefX0 +
                     pSrcLine3[ChannelInd + OffsetColumn1] * CoefX1 +
                     pSrcLine3[ChannelInd + OffsetColumn2] * CoefX2 +
                     pSrcLine3[ChannelInd + OffsetColumn3] * CoefX3;

            ChannelResult = Horiz0 * CoefY0 + Horiz1 * CoefY1 + Horiz2 * CoefY2 + Horiz3 * CoefY3;

            *pOut++ = (T)MIN(MAX(ChannelResult, (double)m_ChannelMinValue),
                             (double)m_ChannelMaxValue);
            }

        ++pPosX;
        ++pPosY;
        --pi_PixelCount;
        }
    }

/**----------------------------------------------------------------------------
 Get pixels

 @param pi_PositionX
 @param pi_PositionY
 @param pi_PixelCount
 @param pi_pBuffer
-----------------------------------------------------------------------------*/
template <class T>
void HRAGenericBicubicSampler<T>::GetPixels(double pi_PositionX,
                                            double pi_PositionY,
                                            size_t  pi_PixelCount,
                                            void*   po_pBuffer) const
    {
    HPRECONDITION(pi_PositionX >= 0.0);
    HPRECONDITION(pi_PositionY >= 0.0);
    HPRECONDITION(pi_PositionX + ((double)(pi_PixelCount - 1) * m_DeltaX) >= 0.0);
    HPRECONDITION(pi_PositionX + (double)(pi_PixelCount - 1) * m_DeltaX <= (double)m_Width + 0.5);
    HPRECONDITION(pi_PositionY + ((double)(pi_PixelCount - 1) * m_DeltaY) >= 0.0);
    HPRECONDITION(pi_PositionY + (double)(pi_PixelCount -  1) * m_DeltaY <= (double)m_Height + 0.5);

    T* pOut = (T*)po_pBuffer;

    if (m_StretchByLine)
        {
        double T2, T3;
        double CoefX0, CoefX1, CoefX2, CoefX3;
        double Horiz0, Horiz1, Horiz2, Horiz3;
        double Dx;
        double ChannelResult;
        int     OffsetColumn1;
        int     OffsetColumn2;
        int     OffsetColumn3;

        Sample  CurrentSample(pi_PositionX, pi_PositionY);

        // Obtain two source lines
        uint32_t Column0 = CurrentSample.GetColumn0();
        uint32_t Column1 = CurrentSample.GetColumn1();

        // Obtain pointers to start of each line
        T* pSrcLine0 = (T*)ComputeAddress(m_pPacket,
                                          Column0,
                                          CurrentSample.GetLine0());
        T* pSrcLine1 = (T*)ComputeAddress(m_pPacket,
                                          Column0,
                                          MIN(CurrentSample.GetLine1(), m_Height-1));
        T* pSrcLine2 = (T*)ComputeAddress(m_pPacket,
                                          Column0,
                                          MIN(CurrentSample.GetLine2(), m_Height-1));
        T* pSrcLine3 = (T*)ComputeAddress(m_pPacket,
                                          Column0,
                                          MIN(CurrentSample.GetLine3(), m_Height-1));

        int NextPixelOffset = m_NbChannels;
        int BytesToAdd;

        double Dy = CurrentSample.GetYDelta();
        HASSERT(Dy >= 0.0 && Dy <= 1.0);

        // Compute coeficients for Y
        T2 = Dy * Dy;
        T3 = Dy * T2;
        double CoefY0 = T3 * s_a - T2 * s_2a + s_a * Dy;
        double CoefY1 = T3 * s_aP2 - T2 * s_aP3 + 1;
        double CoefY2 = T2 * s_2aP3 - T3 * s_aP2 - s_a * Dy;
        double CoefY3 = s_a * (T2 - T3);

        while (pi_PixelCount)
            {
            // Compute byte offsets for each column
            if (Column1 > Column0)
                {
                HASSERT(Column0 < m_Width - 1);
                OffsetColumn1 = m_NbChannels;
                }
            else
                OffsetColumn1 = 0;
            if (CurrentSample.GetColumn2() > Column1 && Column1 < m_Width - 1)
                OffsetColumn2 = OffsetColumn1 + m_NbChannels;
            else
                OffsetColumn2 = OffsetColumn1;
            if (CurrentSample.GetColumn3() < m_Width)
                OffsetColumn3 = OffsetColumn2 + m_NbChannels;
            else
                OffsetColumn3 = OffsetColumn2;

            Dx = CurrentSample.GetXDelta();
            HASSERT(Dx >= 0.0 && Dx <= 1.0);

            // calculate the coefficients for X
            T2 = Dx * Dx;
            T3 = Dx * T2;
            CoefX0 = T3 * s_a - T2 * s_2a + s_a * Dx;
            CoefX1 = T3 * s_aP2 - T2 * s_aP3 + 1;
            CoefX2 = T2 * s_2aP3 - T3 * s_aP2 - s_a * Dx;
            CoefX3 = s_a * (T2 - T3);

            for (unsigned short ChannelInd = 0; ChannelInd < m_NbChannels; ChannelInd++)
                {
                Horiz0 = pSrcLine0[ChannelInd] * CoefX0 +
                         pSrcLine0[ChannelInd + OffsetColumn1] * CoefX1 +
                         pSrcLine0[ChannelInd + OffsetColumn2] * CoefX2 +
                         pSrcLine0[ChannelInd + OffsetColumn3] * CoefX3;
                Horiz1 = pSrcLine1[ChannelInd] * CoefX0 +
                         pSrcLine1[ChannelInd + OffsetColumn1] * CoefX1 +
                         pSrcLine1[ChannelInd + OffsetColumn2] * CoefX2 +
                         pSrcLine1[ChannelInd + OffsetColumn3] * CoefX3;
                Horiz2 = pSrcLine2[ChannelInd] * CoefX0 +
                         pSrcLine2[ChannelInd + OffsetColumn1] * CoefX1 +
                         pSrcLine2[ChannelInd + OffsetColumn2] * CoefX2 +
                         pSrcLine2[ChannelInd + OffsetColumn3] * CoefX3;
                Horiz3 = pSrcLine3[ChannelInd] * CoefX0 +
                         pSrcLine3[ChannelInd + OffsetColumn1] * CoefX1 +
                         pSrcLine3[ChannelInd + OffsetColumn2] * CoefX2 +
                         pSrcLine3[ChannelInd + OffsetColumn3] * CoefX3;

                ChannelResult = Horiz0 * CoefY0 + Horiz1 * CoefY1 + Horiz2 * CoefY2 + Horiz3 * CoefY3;

                *pOut++ = (T)MIN(MAX(ChannelResult, (double)m_ChannelMinValue), (double)m_ChannelMaxValue);
                }

            CurrentSample.TranslateX(m_DeltaX);
            BytesToAdd = (CurrentSample.GetColumn0() - Column0) * m_NbChannels;
            Column0 = CurrentSample.GetColumn0();
            Column1 = CurrentSample.GetColumn1();

            pSrcLine0 += BytesToAdd;
            pSrcLine1 += BytesToAdd;
            pSrcLine2 += BytesToAdd;
            pSrcLine3 += BytesToAdd;

            --pi_PixelCount;
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


/** ----------------------------------------------------------------------------
    Retrieve the pixeltype in which the results of the averaging process will
    be created, if different from the source pixeltype.

    @return The output pixeltype, or NULL if no conversion is applied.
    ----------------------------------------------------------------------------
*/
template <class T>
HFCPtr<HRPPixelType> HRAGenericBicubicSampler<T>::GetOutputPixelType() const
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
template <class T>
bool HRAGenericBicubicSampler<T>::TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType)
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