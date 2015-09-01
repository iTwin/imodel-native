//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRABicubicSamplerN8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRABicubicSamplerN8.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>

// Smoothing precomputing
const double HRABicubicSamplerN8::s_a(-0.5);
const double HRABicubicSamplerN8::s_2a(2.0 * s_a);
const double HRABicubicSamplerN8::s_2aP3(s_2a + 3.0);
const double HRABicubicSamplerN8::s_aP2(s_a + 2.0);
const double HRABicubicSamplerN8::s_aP3(s_a + 3.0);

/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-----------------------------------------------------------------------------*/
HRABicubicSamplerN8::HRABicubicSamplerN8(HGSMemorySurfaceDescriptor const&   pi_rMemorySurface,
                                         const HGF2DRectangle&               pi_rSampleDimension,
                                         double                              pi_DeltaX,
                                         double                              pi_DeltaY)
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
    m_StretchByLine = HDOUBLE_EQUAL_EPSILON(pi_DeltaY, 0.0);
    }

/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
HRABicubicSamplerN8::~HRABicubicSamplerN8()
    {
    if (m_ppConvertedLines != 0)
        {
        for (size_t i = 0 ; i <= m_Height ; ++i)
            delete m_ppConvertedLines[i];
        }
    }


/**----------------------------------------------------------------------------
 Get a specific pixel.

 @param pi_PosX
 @param pi_PosY

 @return const void* The current pixel.
-----------------------------------------------------------------------------*/
void const* HRABicubicSamplerN8::GetPixel(double     pi_PosX,
                                    double     pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0.0);
    HPRECONDITION(pi_PosY >= 0.0);
    HPRECONDITION(pi_PosX <= (double)m_Width + 0.5);
    HPRECONDITION(pi_PosY <= (double)m_Height + 0.5);

    // Clear everything just in case.
    memset(m_TempData, 0, sizeof(m_TempData));

    Sample CurrentSample(pi_PosX, pi_PosY);

    const uint32_t Column0 = CurrentSample.GetColumn0();
    const uint32_t Column1 = CurrentSample.GetColumn1();

    // Obtain pointers to start of each line
    Byte* pSrcLine0 = ComputeAddress(Column0, CurrentSample.GetLine0());
    Byte* pSrcLine1 = ComputeAddress(Column0, MIN(CurrentSample.GetLine1(), m_Height-1));
    Byte* pSrcLine2 = ComputeAddress(Column0, MIN(CurrentSample.GetLine2(), m_Height-1));
    Byte* pSrcLine3 = ComputeAddress(Column0, MIN(CurrentSample.GetLine3(), m_Height-1));

    // Compute byte offsets for each column
    int OffsetColumn1;
    int OffsetColumn2;
    int OffsetColumn3;
    if (Column1 > Column0)
        {
        HASSERT(Column0 < m_Width - 1);
        OffsetColumn1 = m_BytesPerPixel;
        }
    else
        OffsetColumn1 = 0;
    if (CurrentSample.GetColumn2() > Column1 && Column1 < m_Width - 1)
        OffsetColumn2 = OffsetColumn1 + m_BytesPerPixel;
    else
        OffsetColumn2 = OffsetColumn1;
    if (CurrentSample.GetColumn3() < m_Width)
        OffsetColumn3 = OffsetColumn2 + m_BytesPerPixel;
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

    for (size_t Channel = 0 ; Channel < m_BytesPerPixel ; ++Channel)
        {
        Horiz0 = pSrcLine0[Channel] * CoefX0 +
                 pSrcLine0[Channel + OffsetColumn1] * CoefX1 +
                 pSrcLine0[Channel + OffsetColumn2] * CoefX2 +
                 pSrcLine0[Channel + OffsetColumn3] * CoefX3;
        Horiz1 = pSrcLine1[Channel] * CoefX0 +
                 pSrcLine1[Channel + OffsetColumn1] * CoefX1 +
                 pSrcLine1[Channel + OffsetColumn2] * CoefX2 +
                 pSrcLine1[Channel + OffsetColumn3] * CoefX3;
        Horiz2 = pSrcLine2[Channel] * CoefX0 +
                 pSrcLine2[Channel + OffsetColumn1] * CoefX1 +
                 pSrcLine2[Channel + OffsetColumn2] * CoefX2 +
                 pSrcLine2[Channel + OffsetColumn3] * CoefX3;
        Horiz3 = pSrcLine3[Channel] * CoefX0 +
                 pSrcLine3[Channel + OffsetColumn1] * CoefX1 +
                 pSrcLine3[Channel + OffsetColumn2] * CoefX2 +
                 pSrcLine3[Channel + OffsetColumn3] * CoefX3;

        ChannelResult = Horiz0 * CoefY0 + Horiz1 * CoefY1 + Horiz2 * CoefY2 + Horiz3 * CoefY3;

        m_TempData[Channel] = (Byte) MIN( MAX(ChannelResult, 0.0), 255.0);
        }

    return m_TempData;
    }

/**----------------------------------------------------------------------------
 Get pixels at specific location.

 @param pi_pPositionsX
 @param pi_pPositionsY
 @param pi_PixelCount
 @param po_pBuffer
-----------------------------------------------------------------------------*/
void HRABicubicSamplerN8::GetPixels(const double*  pi_pPositionsX,
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

    double T2, T3;
    double CoefX0, CoefX1, CoefX2, CoefX3;
    double CoefY0, CoefY1, CoefY2, CoefY3;
    double Horiz0, Horiz1, Horiz2, Horiz3;
    double Dx, Dy;
    double ChannelResult;
    uint32_t Column0, Column1;
    Byte* pSrcLine0;
    Byte* pSrcLine1;
    Byte* pSrcLine2;
    Byte* pSrcLine3;
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
        pSrcLine0 = ComputeAddress(Column0, CurrentSample.GetLine0());
        pSrcLine1 = ComputeAddress(Column0, MIN(CurrentSample.GetLine1(), m_Height-1));
        pSrcLine2 = ComputeAddress(Column0, MIN(CurrentSample.GetLine2(), m_Height-1));
        pSrcLine3 = ComputeAddress(Column0, MIN(CurrentSample.GetLine3(), m_Height-1));

        // Compute byte offsets for each column
        if (Column1 > Column0)
            {
            HASSERT(Column0 < m_Width - 1);
            OffsetColumn1 = m_BytesPerPixel;
            }
        else
            OffsetColumn1 = 0;
        if (CurrentSample.GetColumn2() > Column1 && Column1 < m_Width - 1)
            OffsetColumn2 = OffsetColumn1 + m_BytesPerPixel;
        else
            OffsetColumn2 = OffsetColumn1;
        if (CurrentSample.GetColumn3() < m_Width)
            OffsetColumn3 = OffsetColumn2 + m_BytesPerPixel;
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

        for (size_t Channel = 0 ; Channel < m_BytesPerPixel ; ++Channel)
            {
            Horiz0 = pSrcLine0[Channel] * CoefX0 +
                     pSrcLine0[Channel + OffsetColumn1] * CoefX1 +
                     pSrcLine0[Channel + OffsetColumn2] * CoefX2 +
                     pSrcLine0[Channel + OffsetColumn3] * CoefX3;
            Horiz1 = pSrcLine1[Channel] * CoefX0 +
                     pSrcLine1[Channel + OffsetColumn1] * CoefX1 +
                     pSrcLine1[Channel + OffsetColumn2] * CoefX2 +
                     pSrcLine1[Channel + OffsetColumn3] * CoefX3;
            Horiz2 = pSrcLine2[Channel] * CoefX0 +
                     pSrcLine2[Channel + OffsetColumn1] * CoefX1 +
                     pSrcLine2[Channel + OffsetColumn2] * CoefX2 +
                     pSrcLine2[Channel + OffsetColumn3] * CoefX3;
            Horiz3 = pSrcLine3[Channel] * CoefX0 +
                     pSrcLine3[Channel + OffsetColumn1] * CoefX1 +
                     pSrcLine3[Channel + OffsetColumn2] * CoefX2 +
                     pSrcLine3[Channel + OffsetColumn3] * CoefX3;

            ChannelResult = Horiz0 * CoefY0 + Horiz1 * CoefY1 + Horiz2 * CoefY2 + Horiz3 * CoefY3;

            *pOut++ = (Byte) MIN( MAX(ChannelResult, 0.0), 255.0);
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
void HRABicubicSamplerN8::GetPixels(double         pi_PositionX,
                                    double         pi_PositionY,
                                    size_t          pi_PixelCount,
                                    void*           po_pBuffer) const
    {
    HPRECONDITION(pi_PositionX >= 0.0);
    HPRECONDITION(pi_PositionY >= 0.0);
    HPRECONDITION(pi_PositionX + ((double)(pi_PixelCount - 1) * m_DeltaX) >= 0.0);
    HPRECONDITION(pi_PositionX + (double)(pi_PixelCount - 1) * m_DeltaX <= (double)m_Width + 0.5);
    HPRECONDITION(pi_PositionY + ((double)(pi_PixelCount - 1) * m_DeltaY) >= 0.0);
    HPRECONDITION(pi_PositionY + (double)(pi_PixelCount -  1) * m_DeltaY <= (double)m_Height + 0.5);

    Byte* pOut = (Byte*)po_pBuffer;

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
        Byte* pSrcLine0 = ComputeAddress(Column0, CurrentSample.GetLine0());
        Byte* pSrcLine1 = ComputeAddress(Column0, MIN(CurrentSample.GetLine1(), m_Height-1));
        Byte* pSrcLine2 = ComputeAddress(Column0, MIN(CurrentSample.GetLine2(), m_Height-1));
        Byte* pSrcLine3 = ComputeAddress(Column0, MIN(CurrentSample.GetLine3(), m_Height-1));

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
                OffsetColumn1 = m_BytesPerPixel;
                }
            else
                OffsetColumn1 = 0;
            if (CurrentSample.GetColumn2() > Column1 && Column1 < m_Width - 1)
                OffsetColumn2 = OffsetColumn1 + m_BytesPerPixel;
            else
                OffsetColumn2 = OffsetColumn1;
            if (CurrentSample.GetColumn3() < m_Width)
                OffsetColumn3 = OffsetColumn2 + m_BytesPerPixel;
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

            for (size_t Channel = 0 ; Channel < m_BytesPerPixel ; ++Channel)
                {
                Horiz0 = pSrcLine0[Channel] * CoefX0 +
                    pSrcLine0[Channel + OffsetColumn1] * CoefX1 +
                    pSrcLine0[Channel + OffsetColumn2] * CoefX2 +
                    pSrcLine0[Channel + OffsetColumn3] * CoefX3;
                Horiz1 = pSrcLine1[Channel] * CoefX0 +
                    pSrcLine1[Channel + OffsetColumn1] * CoefX1 +
                    pSrcLine1[Channel + OffsetColumn2] * CoefX2 +
                    pSrcLine1[Channel + OffsetColumn3] * CoefX3;
                Horiz2 = pSrcLine2[Channel] * CoefX0 +
                    pSrcLine2[Channel + OffsetColumn1] * CoefX1 +
                    pSrcLine2[Channel + OffsetColumn2] * CoefX2 +
                    pSrcLine2[Channel + OffsetColumn3] * CoefX3;
                Horiz3 = pSrcLine3[Channel] * CoefX0 +
                    pSrcLine3[Channel + OffsetColumn1] * CoefX1 +
                    pSrcLine3[Channel + OffsetColumn2] * CoefX2 +
                    pSrcLine3[Channel + OffsetColumn3] * CoefX3;

                ChannelResult = Horiz0 * CoefY0 + Horiz1 * CoefY1 + Horiz2 * CoefY2 + Horiz3 * CoefY3;

                *pOut++ = (Byte) MIN( MAX(ChannelResult, 0.0), 255.0);
                }

            CurrentSample.TranslateX(m_DeltaX);
            BytesToAdd = (CurrentSample.GetColumn0() - Column0) * m_BytesPerPixel;
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
HFCPtr<HRPPixelType> HRABicubicSamplerN8::GetOutputPixelType() const
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
bool HRABicubicSamplerN8::TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType)
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


/** ---------------------------------------------------------------------------
    Get data pointer. Lines will be converted (pixeltype) if necessary.
    ---------------------------------------------------------------------------
*/
Byte* HRABicubicSamplerN8::ComputeAddress(HUINTX  pi_PosX,
                                           HUINTX  pi_PosY,
                                           size_t  pi_NeededPixels) const
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
