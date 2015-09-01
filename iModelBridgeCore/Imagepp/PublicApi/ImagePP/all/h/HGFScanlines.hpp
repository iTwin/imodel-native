//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFScanlines.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Retrieve the minimum Y value of the scanline region. This is the YMin
    value given to SetLimits().
    -----------------------------------------------------------------------------
*/
inline double HGFScanLines::GetYMin() const
    {
    HASSERT(m_LimitsAreSet);

    return m_YMin;
    }


/** -----------------------------------------------------------------------------
    Retrieve the maximum Y value of the scanline region. This is the YMax
    value given to SetLimits().
    -----------------------------------------------------------------------------
*/
inline double HGFScanLines::GetYMax() const
    {
    HASSERT(m_LimitsAreSet);

    return m_YMax;
    }


/** -----------------------------------------------------------------------------
    Get the first scanline position (Y pixel coordinate).
    -----------------------------------------------------------------------------
*/
inline HSINTX HGFScanLines::GetScanLineStart() const
    {
    HASSERT(m_LimitsAreSet);

    return m_FirstScanline;
    }


/** -----------------------------------------------------------------------------
    Get the last scanline position (Y pixel coordinate)
    -----------------------------------------------------------------------------
*/
inline HSINTX HGFScanLines::GetScanLineEnd() const
    {
    HASSERT(m_LimitsAreSet);

    HASSERT_X64(m_FirstScanline + m_NumberOfScanlines <= HSINTX_MAX);
    return (HSINTX)(m_FirstScanline + m_NumberOfScanlines);
    }


/** -----------------------------------------------------------------------------
    Check if the scanlines are for a rectangular region. If it is true, all
    scanlines can be processed by retrieving the first run and processing the
    same X coordinates for all scanlines.
    -----------------------------------------------------------------------------
*/
inline bool HGFScanLines::IsRectangle() const
    {
    HASSERT(m_LimitsAreSet);

    return m_IsRectangle;
    }


/** -----------------------------------------------------------------------------
    Get the number of scanlines to process.
    -----------------------------------------------------------------------------
*/
inline size_t HGFScanLines::GetScanlineCount() const
    {
    HASSERT(m_LimitsAreSet);

    return m_NumberOfScanlines;
    }


/** -----------------------------------------------------------------------------
    Retrieve the coordinate system used when creating the scanlines. For
    informational purposes only.
    -----------------------------------------------------------------------------
*/
inline const HFCPtr<HGF2DCoordSys>& HGFScanLines::GetScanlinesCoordSys() const
    {
    return m_pCoordSys;
    }


/** -----------------------------------------------------------------------------
    Set the coordinate system that the coordinates are expressed in. For
    informational purposes only.
    -----------------------------------------------------------------------------
*/
inline void HGFScanLines::SetScanlinesCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    m_pCoordSys = pi_rpCoordSys;
    }


/** -----------------------------------------------------------------------------
    Retrieve the precise coordinate (Y) of the first scanline. Subsequent
    scanline coordinates can be computed by adding 1.0 to the returned value
    for each scanline.
    -----------------------------------------------------------------------------
*/
inline double HGFScanLines::GetFirstScanlinePosition() const
    {
    HASSERT(m_LimitsAreSet);

    return m_FirstScanline + 0.5;
    }


/** -----------------------------------------------------------------------------
    Test if SetLimits has been called.

    @see HGFScanLines::SetLimits()
    -----------------------------------------------------------------------------
*/
inline bool HGFScanLines::LimitsAreSet() const
    {
    return m_LimitsAreSet;
    }


/** -----------------------------------------------------------------------------
    Retrieve the current scanline position (Y coordinate in pixels).
    -----------------------------------------------------------------------------
*/
inline HSINTX HGFScanLines::GetCurrentScanLine() const
    {
    HASSERT(m_CurrentScanLine < m_NumberOfScanlines);

    HASSERT_X64(m_FirstScanline + m_CurrentScanLine <= HSINTX_MAX);
    return (HSINTX)(m_FirstScanline + m_CurrentScanLine);
    }


/** -----------------------------------------------------------------------------
    Retrieve the current run start position (X coordinate in pixels).
    -----------------------------------------------------------------------------
*/
inline HSINTX HGFScanLines::GetCurrentRunXMin() const
    {
    HASSERT(m_CurrentScanLine < m_NumberOfScanlines);

    if (m_IsRectangle)
        return m_pPixelSelector->ConvertXMin(m_pScanLines->at(m_PositionInScanline));
    else
        return m_pPixelSelector->ConvertXMin(m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline));
    }


/** -----------------------------------------------------------------------------
    Retrieve the current run length in pixels.
    -----------------------------------------------------------------------------
*/
inline size_t HGFScanLines::GetCurrentRunLength() const
    {
    if (m_IsRectangle)
        return m_pPixelSelector->ConvertXMax(m_pScanLines->at(m_PositionInScanline+1)) - m_pPixelSelector->ConvertXMin(m_pScanLines->at(m_PositionInScanline));
    else
        return m_pPixelSelector->ConvertXMax(m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline+1)) - m_pPixelSelector->ConvertXMin(m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline));
    }


/** -----------------------------------------------------------------------------
    Adjust the Y component of a segment endpoint so that it is never directly
    on a 0.5 boundary. If it is, an epsilon value is ADDED to it. This is
    necessary to handle special flirting cases.

    @note This method must absolutely be used for all Y values that
          represent segment endpoints. Otherwise, false crossing points may
          be computed, and this could cause errors in the run generation
          algorithm.
    -----------------------------------------------------------------------------
*/
inline double HGFScanLines::AdjustEndpointYValue(double pi_YValue) const
    {
    return HDOUBLE_EQUAL_EPSILON(fabs(pi_YValue - static_cast<int32_t>(pi_YValue)), 0.5) ?
           pi_YValue + HGLOBAL_EPSILON :
           pi_YValue;
    }


/** -----------------------------------------------------------------------------
    Retrieve all information about the current run.

    @see GetCurrentScanLine()
    @see GetCurrentRunXMin()
    @see GetCurrentRunLength()
    -----------------------------------------------------------------------------
*/
inline void HGFScanLines::GetCurrentRun(HSINTX* po_pXMin, HSINTX* po_pScanline, size_t* po_pLength) const
    {
    HASSERT(po_pScanline != 0);
    HASSERT(po_pXMin != 0);
    HASSERT(po_pLength != 0);
    HASSERT(m_CurrentScanLine < m_NumberOfScanlines);

    HASSERT_X64(m_FirstScanline + m_CurrentScanLine <= HSINTX_MAX);
    *po_pScanline = (HSINTX)(m_FirstScanline + m_CurrentScanLine);

    if (m_IsRectangle)
        {
        HASSERT(m_pPixelSelector->IsAValidRun(m_pScanLines->at(m_PositionInScanline), m_pScanLines->at(m_PositionInScanline+1)));

        *po_pXMin = m_pPixelSelector->ConvertXMin(m_pScanLines->at(m_PositionInScanline));
        *po_pLength = m_pPixelSelector->ConvertXMax(m_pScanLines->at(m_PositionInScanline+1)) - *po_pXMin;
        }
    else
        {
        HASSERT(m_pPixelSelector->IsAValidRun(m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline), m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline+1)));

        *po_pXMin = m_pPixelSelector->ConvertXMin(m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline));
        *po_pLength = m_pPixelSelector->ConvertXMax(m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline+1)) - *po_pXMin;
        }

    HASSERT(*po_pLength > 0);
    }


/** -----------------------------------------------------------------------------
    Modify the received X value if necessary to ensure that it is within the
    current run. This is to make sure that user-decided sample points lie
    within the runs.

    @see GetCurrentRun()
    @see AdjustYValueForCurrentRun()
    -----------------------------------------------------------------------------
*/
inline double HGFScanLines::AdjustXValueForCurrentRun(double pi_X) const
    {
    if (m_IsRectangle)
        {
        HASSERT(pi_X >= m_pScanLines->at(m_PositionInScanline) - 1.0);
        HASSERT(pi_X <= m_pScanLines->at(m_PositionInScanline+1) + 1.0);

        if (pi_X <= m_pScanLines->at(m_PositionInScanline))
            return m_pScanLines->at(m_PositionInScanline) + HGLOBAL_EPSILON;

        if (pi_X >= m_pScanLines->at(m_PositionInScanline+1))
            return m_pScanLines->at(m_PositionInScanline+1) - HGLOBAL_EPSILON;
        }
    else
        {
        HASSERT(pi_X >= m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline) - 1.0);
        HASSERT(pi_X <= m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline+1) + 1.0);

        if (pi_X <= m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline))
            return m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline) + HGLOBAL_EPSILON;

        if (pi_X >= m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline+1))
            return m_pScanLines[m_CurrentScanLine].at(m_PositionInScanline+1) - HGLOBAL_EPSILON;
        }

    return pi_X;
    }


/** -----------------------------------------------------------------------------
    Modify the received Y value if necessary to ensure that it is within the
    Y limits. This is to make sure that user-decided sample points lie
    within the runs.

    @see GetCurrentRun()
    @see AdjustXValueForCurrentRun()
    -----------------------------------------------------------------------------
*/
inline double HGFScanLines::AdjustYValueForCurrentRun(double pi_Y) const
    {
    HASSERT(pi_Y >= m_YMin - 1.0);
    HASSERT(pi_Y <= m_YMax + 1.0);

    if (pi_Y <= m_YMin)
        return m_YMin + HGLOBAL_EPSILON;

    if (pi_Y >= m_YMax)
        return m_YMax - HGLOBAL_EPSILON;

    return pi_Y;
    }

/** -----------------------------------------------------------------------------
    GetSpanArray()
    -----------------------------------------------------------------------------
*/
inline HGFScanLines::SpanArray* HGFScanLines::GetSpanArray() const
    {
    return m_pScanLines;
    }

/** -----------------------------------------------------------------------------
SetStrategy()
-----------------------------------------------------------------------------
*/
inline void HGFScanLines::SetStrategy(IPixelSelectionStrategy* pi_pStrategy)
    {
    m_pPixelSelector = pi_pStrategy;
    }
END_IMAGEPP_NAMESPACE