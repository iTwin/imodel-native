//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABlitterAveragingGrid.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#define HRAAVGGRID_MINPIXELOVERLAP      0.2
#define HRAAVGGRID_MINPIXELOVERLAPX2    0.4

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
inline HRABlitterAveragingGrid::HRABlitterAveragingGrid (double pi_XMin,
                                                         double pi_YMin,
                                                         double pi_XMax,
                                                         double pi_YMax,
                                                         uint32_t pi_XLimit,
                                                         uint32_t pi_YLimit)
    {
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_XMin, pi_XMax) || HDOUBLE_EQUAL_EPSILON(pi_XMin, pi_XMax));
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_YMin, pi_YMax) || HDOUBLE_EQUAL_EPSILON(pi_YMin, pi_YMax));

    m_XMin      = min(pi_XMin, pi_XMax);
    m_YMin      = min(pi_YMin, pi_YMax);
    m_XMax      = max(pi_XMax, pi_XMin);
    m_YMax      = max(pi_YMax, pi_YMin);

    m_XLimit = pi_XLimit;
    m_YLimit = pi_YLimit;

    ComputeResults();
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
inline HRABlitterAveragingGrid::HRABlitterAveragingGrid(const HRABlitterAveragingGrid& pi_rObj)
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
inline HRABlitterAveragingGrid::~HRABlitterAveragingGrid()
    {
    }


//-----------------------------------------------------------------------------
// public
// operator=
// Assignment operator.  It duplicates another line object.
//-----------------------------------------------------------------------------
inline HRABlitterAveragingGrid& HRABlitterAveragingGrid::operator=(const HRABlitterAveragingGrid& pi_rObj)
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
inline void HRABlitterAveragingGrid::SetExtent(double pi_XMin,
                                               double pi_YMin,
                                               double pi_XMax,
                                               double pi_YMax)
    {
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_XMin, pi_XMax) || HDOUBLE_EQUAL_EPSILON(pi_XMin, pi_XMax));
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_YMin, pi_YMax) || HDOUBLE_EQUAL_EPSILON(pi_YMin, pi_YMax));

    m_XMin = min(pi_XMin, pi_XMax);
    m_YMin = min(pi_YMin, pi_YMax);
    m_XMax = max(pi_XMax, pi_XMin);
    m_YMax = max(pi_YMax, pi_YMin);

    ComputeResults();
    }


//-----------------------------------------------------------------------------
// public
// TranslateX
//-----------------------------------------------------------------------------
inline void HRABlitterAveragingGrid::TranslateX(double pi_Translation)
    {
    m_XMin += pi_Translation;
    m_XMax += pi_Translation;

    // As ComputeResults(), but only on X
    m_ResultXMin = (uint32_t)(max(m_XMin, 0.0) + HRAAVGGRID_MINPIXELOVERLAP);
    m_ResultXMax = min((uint32_t)max(m_XMax - HRAAVGGRID_MINPIXELOVERLAP, 0.0), m_XLimit);

    if (m_ResultXMin > m_ResultXMax)
        {
        uint32_t Temp = m_ResultXMin;
        m_ResultXMin = m_ResultXMax;
        m_ResultXMax = min(Temp, m_XLimit);
        }
    }


//-----------------------------------------------------------------------------
// public
// SetLimits
// These are used in the GetXMax, GetYMax, GetWidth and GetHeight methods.
// The implicit lower limits are always 0.
//-----------------------------------------------------------------------------
inline void HRABlitterAveragingGrid::SetLimits(uint32_t pi_XLimit,
                                               uint32_t pi_YLimit)
    {
    m_XLimit = pi_XLimit;
    m_YLimit = pi_YLimit;

    ComputeResults();
    }


//-----------------------------------------------------------------------------
// public
// ComputeResults
// Pre-calculate the integer results.
//-----------------------------------------------------------------------------
inline void HRABlitterAveragingGrid::ComputeResults()
    {
    m_ResultXMin = (uint32_t)(max(m_XMin, 0.0) + HRAAVGGRID_MINPIXELOVERLAP);
    m_ResultYMin = (uint32_t)(max(m_YMin, 0.0) + HRAAVGGRID_MINPIXELOVERLAP);

    m_ResultXMax = min((uint32_t)max(m_XMax - HRAAVGGRID_MINPIXELOVERLAP, 0.0), m_XLimit);
    m_ResultYMax = min((uint32_t)max(m_YMax - HRAAVGGRID_MINPIXELOVERLAP, 0.0), m_YLimit);

    uint32_t Temp;
    if (m_ResultXMin > m_ResultXMax)
        {
        Temp = m_ResultXMin;
        m_ResultXMin = m_ResultXMax;
        m_ResultXMax = min(Temp, m_XLimit);
        }
    if (m_ResultYMin > m_ResultYMax)
        {
        Temp = m_ResultYMin;
        m_ResultYMin = m_ResultYMax;
        m_ResultYMax = min(Temp, m_YLimit);
        }
    }


//-----------------------------------------------------------------------------
// public
// GetExtent
// Return the extent
//-----------------------------------------------------------------------------
inline void HRABlitterAveragingGrid::GetExtent(double* po_pXMin,
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
inline uint32_t HRABlitterAveragingGrid::ConvertValue(double     pi_Value,
                                                    double     pi_Precision)
    {
    return (uint32_t)(floor(pi_Value + pi_Precision) + pi_Precision);
    }



//-----------------------------------------------------------------------------
// public
// ConvertValue
// Convert a long value into double
//-----------------------------------------------------------------------------
inline double HRABlitterAveragingGrid::ConvertValue(uint32_t pi_Value)
    {
    return (double)pi_Value + 0.5;
    }

//-----------------------------------------------------------------------------
// public
// GetXMin
// Gets the minimum X value of grid
//-----------------------------------------------------------------------------
inline uint32_t HRABlitterAveragingGrid::GetXMin() const
    {
    return m_ResultXMin;
    }

//-----------------------------------------------------------------------------
// public
// GetYMin
// Gets the minimum Y value of grid
//-----------------------------------------------------------------------------
inline uint32_t HRABlitterAveragingGrid::GetYMin() const
    {
    return m_ResultYMin;
    }

//-----------------------------------------------------------------------------
// public
// GetXMax
// Gets the maximum X value of grid
//-----------------------------------------------------------------------------
inline uint32_t HRABlitterAveragingGrid::GetXMax() const
    {
    return m_ResultXMax;
    }

//-----------------------------------------------------------------------------
// public
// GetYMax
// Gets the maximum Y value of grid
//-----------------------------------------------------------------------------
inline uint32_t HRABlitterAveragingGrid::GetYMax() const
    {
    return m_ResultYMax;
    }

//-----------------------------------------------------------------------------
// public
// GetWidth
// Gets the width of the grid
//-----------------------------------------------------------------------------
inline uint32_t HRABlitterAveragingGrid::GetWidth() const
    {
    return m_ResultXMax - m_ResultXMin + 1;
    }

//-----------------------------------------------------------------------------
// public
// GetHeight
// Gets the height of the grid
//-----------------------------------------------------------------------------
inline uint32_t HRABlitterAveragingGrid::GetHeight() const
    {
    return m_ResultYMax - m_ResultYMin + 1;
    }

