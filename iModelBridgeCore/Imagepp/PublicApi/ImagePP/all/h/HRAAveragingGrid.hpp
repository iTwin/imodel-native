//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAAveragingGrid.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
#define HRAAVGGRID_MINPIXELOVERLAP      0.2
#define HRAAVGGRID_MINPIXELOVERLAPX2    0.4


//-----------------------------------------------------------------------------
// public
// ComputeResults
// Pre-calculate the integer results.
//-----------------------------------------------------------------------------
inline void HRAAveragingGrid::ComputeResults()
    {
    m_ResultXMin = (uint32_t)(MAX(m_XMin, 0.0) + HRAAVGGRID_MINPIXELOVERLAP);
    m_ResultYMin = (uint32_t)(MAX(m_YMin, 0.0) + HRAAVGGRID_MINPIXELOVERLAP);

    m_ResultXMax = MIN((uint32_t)MAX(m_XMax - HRAAVGGRID_MINPIXELOVERLAP, 0.0), m_XLimit);
    m_ResultYMax = MIN((uint32_t)MAX(m_YMax - HRAAVGGRID_MINPIXELOVERLAP, 0.0), m_YLimit);

    uint32_t Temp;
    if (m_ResultXMin > m_ResultXMax)
        {
        Temp = m_ResultXMin;
        m_ResultXMin = m_ResultXMax;
        m_ResultXMax = MIN(Temp, m_XLimit);
        }
    if (m_ResultYMin > m_ResultYMax)
        {
        Temp = m_ResultYMin;
        m_ResultYMin = m_ResultYMax;
        m_ResultYMax = MIN(Temp, m_YLimit);
        }
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
inline HRAAveragingGrid::HRAAveragingGrid (double pi_XMin,
                                           double pi_YMin,
                                           double pi_XMax,
                                           double pi_YMax,
                                           uint32_t pi_XLimit,
                                           uint32_t pi_YLimit)
    {
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_XMin, pi_XMax) || HDOUBLE_EQUAL_EPSILON(pi_XMin, pi_XMax));
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_YMin, pi_YMax) || HDOUBLE_EQUAL_EPSILON(pi_YMin, pi_YMax));

    m_XMin      = MIN(pi_XMin, pi_XMax);
    m_YMin      = MIN(pi_YMin, pi_YMax);
    m_XMax      = MAX(pi_XMax, pi_XMin);
    m_YMax      = MAX(pi_YMax, pi_YMin);

    m_XLimit = pi_XLimit;
    m_YLimit = pi_YLimit;

    ComputeResults();
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
inline HRAAveragingGrid::HRAAveragingGrid(const HRAAveragingGrid& pi_rObj)
    {
    m_XMin      = pi_rObj.m_XMin;
    m_YMin      = pi_rObj.m_YMin;
    m_XMax      = pi_rObj.m_XMax;
    m_YMax      = pi_rObj.m_YMax;
    m_XLimit    = pi_rObj.m_XLimit;
    m_YLimit    = pi_rObj.m_YLimit;

    m_ResultXMin = pi_rObj.m_ResultXMin;
    m_ResultYMin = pi_rObj.m_ResultYMin;
    m_ResultXMax = pi_rObj.m_ResultXMax;
    m_ResultYMax = pi_rObj.m_ResultYMax;
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
inline HRAAveragingGrid::~HRAAveragingGrid()
    {
    }


//-----------------------------------------------------------------------------
// public
// operator=
// Assignment operator.  It duplicates another line object.
//-----------------------------------------------------------------------------
inline HRAAveragingGrid& HRAAveragingGrid::operator=(const HRAAveragingGrid& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        m_XMin      = pi_rObj.m_XMin;
        m_YMin      = pi_rObj.m_YMin;
        m_XMax      = pi_rObj.m_XMax;
        m_YMax      = pi_rObj.m_YMax;
        m_XLimit    = pi_rObj.m_XLimit;
        m_YLimit    = pi_rObj.m_YLimit;

        m_ResultXMin = pi_rObj.m_ResultXMin;
        m_ResultYMin = pi_rObj.m_ResultYMin;
        m_ResultXMax = pi_rObj.m_ResultXMax;
        m_ResultYMax = pi_rObj.m_ResultYMax;
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// public
// SetExtent
// Set a new extent
//-----------------------------------------------------------------------------
inline void HRAAveragingGrid::SetExtent(double pi_XMin,
                                        double pi_YMin,
                                        double pi_XMax,
                                        double pi_YMax)
    {
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_XMin, pi_XMax) || HDOUBLE_EQUAL_EPSILON(pi_XMin, pi_XMax));
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_YMin, pi_YMax) || HDOUBLE_EQUAL_EPSILON(pi_YMin, pi_YMax));

    m_XMin = MIN(pi_XMin, pi_XMax);
    m_YMin = MIN(pi_YMin, pi_YMax);
    m_XMax = MAX(pi_XMax, pi_XMin);
    m_YMax = MAX(pi_YMax, pi_YMin);

    ComputeResults();
    }


//-----------------------------------------------------------------------------
// public
// TranslateX
//-----------------------------------------------------------------------------
inline void HRAAveragingGrid::TranslateX(double pi_Translation)
    {
    m_XMin += pi_Translation;
    m_XMax += pi_Translation;

    // As ComputeResults(), but only on X
    m_ResultXMin = (uint32_t)(MAX(m_XMin, 0.0) + HRAAVGGRID_MINPIXELOVERLAP);
    m_ResultXMax = MIN((uint32_t)MAX(m_XMax - HRAAVGGRID_MINPIXELOVERLAP, 0.0), m_XLimit);

    if (m_ResultXMin > m_ResultXMax)
        {
        uint32_t Temp = m_ResultXMin;
        m_ResultXMin = m_ResultXMax;
        m_ResultXMax = MIN(Temp, m_XLimit);
        }
    }


//-----------------------------------------------------------------------------
// public
// TranslateY
//-----------------------------------------------------------------------------
inline void HRAAveragingGrid::TranslateY(double pi_Translation)
    {
    m_YMin += pi_Translation;
    m_YMax += pi_Translation;

    // As ComputeResults(), but only on X
    m_ResultYMin = (uint32_t)(MAX(m_YMin, 0.0) + HRAAVGGRID_MINPIXELOVERLAP);
    m_ResultYMax = MIN((uint32_t)MAX(m_YMax - HRAAVGGRID_MINPIXELOVERLAP, 0.0), m_YLimit);

    if (m_ResultYMin > m_ResultYMax)
        {
        uint32_t Temp = m_ResultYMin;
        m_ResultYMin = m_ResultYMax;
        m_ResultYMax = MIN(Temp, m_YLimit);
        }
    }


//-----------------------------------------------------------------------------
// public
// SetLimits
// These are used in the GetXMax, GetYMax, GetWidth and GetHeight methods.
// The implicit lower limits are always 0.
//-----------------------------------------------------------------------------
inline void HRAAveragingGrid::SetLimits(uint32_t pi_XLimit,
                                        uint32_t pi_YLimit)
    {
    m_XLimit = pi_XLimit;
    m_YLimit = pi_YLimit;

    ComputeResults();
    }


//-----------------------------------------------------------------------------
// public
// GetExtent
// Return the extent
//-----------------------------------------------------------------------------
inline void HRAAveragingGrid::GetExtent(double* po_pXMin,
                                        double* po_pYMin,
                                        double* po_pXMax,
                                        double* po_pYMax) const
    {
    HPRECONDITION(po_pXMin != 0);
    HPRECONDITION(po_pYMin != 0);
    HPRECONDITION(po_pXMax != 0);
    HPRECONDITION(po_pYMax != 0);

    *po_pXMin = m_XMin;
    *po_pYMin = m_YMin;
    *po_pXMax = m_XMax;
    *po_pYMax = m_YMax;
    }


//-----------------------------------------------------------------------------
// public
// ConvertValue
// Convert a double value into long
//-----------------------------------------------------------------------------
inline uint32_t HRAAveragingGrid::ConvertValue(double     pi_Value,
                                             double     pi_Precision)
    {
    return (uint32_t)(floor(pi_Value + pi_Precision) + pi_Precision);
    }


//-----------------------------------------------------------------------------
// public
// ConvertValue
// Convert a long value into double
//-----------------------------------------------------------------------------
inline double HRAAveragingGrid::ConvertValue(uint32_t pi_Value)
    {
    return (double)pi_Value + 0.5;
    }


//-----------------------------------------------------------------------------
// public
// GetXMin
// Gets the minimum X value of grid
//-----------------------------------------------------------------------------
inline uint32_t HRAAveragingGrid::GetXMin() const
    {
    return m_ResultXMin;
    }


//-----------------------------------------------------------------------------
// public
// GetYMin
// Gets the minimum Y value of grid
//-----------------------------------------------------------------------------
inline uint32_t HRAAveragingGrid::GetYMin() const
    {
    return m_ResultYMin;
    }


//-----------------------------------------------------------------------------
// public
// GetXMax
// Gets the maximum X value of grid
//-----------------------------------------------------------------------------
inline uint32_t HRAAveragingGrid::GetXMax() const
    {
    return m_ResultXMax;
    }


//-----------------------------------------------------------------------------
// public
// GetYMax
// Gets the maximum Y value of grid
//-----------------------------------------------------------------------------
inline uint32_t HRAAveragingGrid::GetYMax() const
    {
    return m_ResultYMax;
    }


//-----------------------------------------------------------------------------
// public
// GetWidth
// Gets the width of the grid
//-----------------------------------------------------------------------------
inline uint32_t HRAAveragingGrid::GetWidth() const
    {
    return m_ResultXMax - m_ResultXMin + 1;
    }


//-----------------------------------------------------------------------------
// public
// GetHeight
// Gets the height of the grid
//-----------------------------------------------------------------------------
inline uint32_t HRAAveragingGrid::GetHeight() const
    {
    return m_ResultYMax - m_ResultYMin + 1;
    }

END_IMAGEPP_NAMESPACE