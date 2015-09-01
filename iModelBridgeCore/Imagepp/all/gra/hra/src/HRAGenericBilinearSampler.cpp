//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAGenericBilinearSampler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAGenericBilinearSampler.h>

/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-----------------------------------------------------------------------------*/
template <class T>
HRAGenericBilinearSampler<T>::HRAGenericBilinearSampler(HGSMemorySurfaceDescriptor const&   pi_rMemorySurface,
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
HRAGenericBilinearSampler<T>::~HRAGenericBilinearSampler()
    {
    }

/**----------------------------------------------------------------------------
 Get a specific pixel.

 @param pi_PosX
 @param pi_PosY

 @return const void* The current pixel.
-----------------------------------------------------------------------------*/
template <class T>
void* HRAGenericBilinearSampler<T>::GetPixel(double pi_PosX,
                                             double pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0.0);
    HPRECONDITION(pi_PosY >= 0.0);
    HPRECONDITION(pi_PosX <= (double)m_Width + 0.5);
    HPRECONDITION(pi_PosY <= (double)m_Height + 0.5);

    // Clear everything just in case.
    memset(m_pTempData, 0, sizeof(m_pTempData));

    Sample CurrentSample(pi_PosX, pi_PosY);

    Byte* pSrcFirstLine;
    Byte* pSrcSecondLine;
    T       ChannelResult;

    uint32_t XPosition = CurrentSample.GetFirstColumn();
    double Dx = CurrentSample.GetXDeltaOfFirstPixel();
    double Dy = CurrentSample.GetYDeltaOfFirstPixel();
    HASSERT(Dx >= 0.0 && Dx <= 1.0);
    HASSERT(Dy >= 0.0 && Dy <= 1.0);
    double DxComplement = 1.0 - Dx;
    double DyComplement = 1.0 - Dy;
    int     NextPixelOffset;

    pSrcFirstLine = ComputeAddress(m_pPacket,
                                   XPosition,
                                   CurrentSample.GetFirstLine());
    pSrcSecondLine = ComputeAddress(m_pPacket,
                                    XPosition,
                                    MIN(CurrentSample.GetSecondLine(), m_Height - 1));
    HASSERT(CurrentSample.GetSecondLine() >= CurrentSample.GetFirstLine());

    if (CurrentSample.GetSecondColumn() > XPosition && XPosition < m_Width - 1)
        NextPixelOffset = m_NbChannels;
    else
        NextPixelOffset = 0;

    for (size_t Channel = 0 ; Channel < m_NbChannels; ++Channel)
        {
        ChannelResult = (T)(DyComplement *
                            (DxComplement * (double)pSrcFirstLine[Channel] +
                             Dx * (double)pSrcFirstLine[Channel + NextPixelOffset]) +
                            Dy * (DxComplement * (double)pSrcSecondLine[Channel] +
                                  Dx * (double)pSrcSecondLine[Channel + NextPixelOffset]));

        m_pTempData[Channel] = (T)MIN(MAX(ChannelResult, m_ChannelMinValue), m_ChannelMaxValue);
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
void HRAGenericBilinearSampler<T>::GetPixels(const double* pi_pPositionsX,
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

    Sample CurrentSample(0.0, 0.0);

    uint32_t XPosition;
    T*      pSrcFirstLine;
    T*      pSrcSecondLine;

    double Dx;
    double DxComplement;
    double Dy;
    double DyComplement;
    T       ChannelResult;
    int     NextPixelOffset;

    while (pi_PixelCount)
        {
        CurrentSample.SetPosition(*pPosX, *pPosY);

        XPosition = CurrentSample.GetFirstColumn();
        Dx = CurrentSample.GetXDeltaOfFirstPixel();
        Dy = CurrentSample.GetYDeltaOfFirstPixel();
        HASSERT(Dx >= 0.0 && Dx <= 1.0);
        HASSERT(Dy >= 0.0 && Dy <= 1.0);
        DxComplement = 1.0 - Dx;
        DyComplement = 1.0 - Dy;

        pSrcFirstLine  = (T*)ComputeAddress(m_pPacket,
                                            XPosition,
                                            CurrentSample.GetFirstLine());
        pSrcSecondLine = (T*)ComputeAddress(m_pPacket,
                                            XPosition,
                                            MIN(CurrentSample.GetSecondLine(), m_Height-1));
        HASSERT(CurrentSample.GetSecondLine() >= CurrentSample.GetFirstLine());

        if ((CurrentSample.GetSecondColumn() > XPosition) && (XPosition < m_Width - 1))
            NextPixelOffset = m_NbChannels;
        else
            NextPixelOffset = 0;

        for (size_t Channel = 0; Channel < m_NbChannels; ++Channel)
            {
            ChannelResult = (T)(DyComplement *
                                (DxComplement * (double)pSrcFirstLine[Channel] +
                                 Dx * (double) pSrcFirstLine[Channel + NextPixelOffset]) +
                                Dy * (DxComplement * (double) pSrcSecondLine[Channel] +
                                      Dx * (double) pSrcSecondLine[Channel + NextPixelOffset]));

            *pOut++ = (T)MIN(MAX(ChannelResult, m_ChannelMinValue), m_ChannelMaxValue);
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
void HRAGenericBilinearSampler<T>::GetPixels(double pi_PositionX,
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
        Sample CurrentSample(pi_PositionX, pi_PositionY);

        // Obtain two source lines
        uint32_t XPosition = CurrentSample.GetFirstColumn();

        T* pSrcFirstLine  = (T*)ComputeAddress(m_pPacket,
                                               XPosition,
                                               CurrentSample.GetFirstLine());
        T* pSrcSecondLine = (T*)ComputeAddress(m_pPacket,
                                               XPosition,
                                               MIN(CurrentSample.GetSecondLine(), m_Height-1));

        HASSERT(CurrentSample.GetSecondLine() >= CurrentSample.GetFirstLine());

        double Dx;
        double DxComplement;
        double Dy = CurrentSample.GetYDeltaOfFirstPixel();
        HASSERT(Dy >= 0.0 && Dy <= 1.0);
        double DyComplement = 1.0 - Dy;
        T       ChannelResult;
        int     NextPixelOffset = m_BytesPerPixel;
        int     BytesToAdd;

        while (pi_PixelCount)
            {
            Dx = CurrentSample.GetXDeltaOfFirstPixel();
            HASSERT(Dx >= 0.0 && Dx <= 1.0);
            DxComplement = 1.0 - Dx;

            if ((CurrentSample.GetSecondColumn() > XPosition) && (XPosition < m_Width - 1))
                NextPixelOffset = m_NbChannels;
            else
                NextPixelOffset = 0;

            for (size_t Channel = 0; Channel < m_NbChannels; ++Channel)
                {
                ChannelResult = (T)(DyComplement * (DxComplement * (double)pSrcFirstLine[Channel] +
                                                    Dx * (double)pSrcFirstLine[Channel + NextPixelOffset]) +
                                    Dy * (DxComplement * (double)pSrcSecondLine[Channel] +
                                          Dx * (double) pSrcSecondLine[Channel + NextPixelOffset]));

                *pOut++ = (T)MIN(MAX(ChannelResult, m_ChannelMinValue), m_ChannelMaxValue);
                }

            CurrentSample.TranslateX(m_DeltaX);
            BytesToAdd = (CurrentSample.GetFirstColumn() - XPosition) * m_NbChannels;
            XPosition = CurrentSample.GetFirstColumn();

            pSrcFirstLine += BytesToAdd;
            pSrcSecondLine += BytesToAdd;

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
HFCPtr<HRPPixelType> HRAGenericBilinearSampler<T>::GetOutputPixelType() const
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
bool HRAGenericBilinearSampler<T>::TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType)
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