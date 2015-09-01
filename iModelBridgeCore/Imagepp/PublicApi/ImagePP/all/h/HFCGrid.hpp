//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCGrid.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
inline HFCGrid::HFCGrid (double pi_Precision)
    : m_Precision(pi_Precision)
    {
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
inline HFCGrid::HFCGrid (double pi_XMin,
                         double pi_YMin,
                         double pi_XMax,
                         double pi_YMax,
                         double pi_Precision)
    {
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_XMin, pi_XMax) || HDOUBLE_EQUAL_EPSILON(pi_XMin, pi_XMax));
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_YMin, pi_YMax) || HDOUBLE_EQUAL_EPSILON(pi_YMin, pi_YMax));

    m_XMin      = MIN(pi_XMin, pi_XMax);
    m_YMin      = MIN(pi_YMin, pi_YMax);
    m_XMax      = MAX(pi_XMax, pi_XMin);
    m_YMax      = MAX(pi_YMax, pi_YMin);
    m_Precision = pi_Precision;
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
inline HFCGrid::HFCGrid(const HFCGrid& pi_rObj)
    {
    m_XMin      = pi_rObj.m_XMin;
    m_YMin      = pi_rObj.m_YMin;
    m_XMax      = pi_rObj.m_XMax;
    m_YMax      = pi_rObj.m_YMax;
    m_Precision = pi_rObj.m_Precision;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
inline HFCGrid::~HFCGrid()
    {
    }


//-----------------------------------------------------------------------------
// public
// operator=
// Assignment operator.  It duplicates another line object.
//-----------------------------------------------------------------------------
inline HFCGrid& HFCGrid::operator=(const HFCGrid& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        m_XMin      = pi_rObj.m_XMin;
        m_YMin      = pi_rObj.m_YMin;
        m_XMax      = pi_rObj.m_XMax;
        m_YMax      = pi_rObj.m_YMax;
        m_Precision = pi_rObj.m_Precision;
        }

    // Return reference to self
    return (*this);
    }

//-----------------------------------------------------------------------------
// public
// SetExtent
// Set a new extent
//-----------------------------------------------------------------------------
inline void HFCGrid::SetExtent(double pi_XMin,
                               double pi_YMin,
                               double pi_XMax,
                               double pi_YMax)
    {
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_XMin, pi_XMax) || HDOUBLE_EQUAL_EPSILON(pi_XMin, pi_XMax));
    HPRECONDITION(HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_YMin, pi_YMax) || HDOUBLE_EQUAL_EPSILON(pi_YMin, pi_YMax));

    m_XMin      = MIN(pi_XMin, pi_XMax);
    m_YMin      = MIN(pi_YMin, pi_YMax);
    m_XMax      = MAX(pi_XMax, pi_XMin);
    m_YMax      = MAX(pi_YMax, pi_YMin);
    }

//-----------------------------------------------------------------------------
// public
// GetExtent
// Return the extent
//-----------------------------------------------------------------------------
inline void HFCGrid::GetExtent(double* po_pXMin,
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
inline int32_t HFCGrid::ConvertValue(double     pi_Value,
                                    double     pi_Precision)
    {
    return (int32_t)(floor(pi_Value + pi_Precision) + pi_Precision);
    }



//-----------------------------------------------------------------------------
// public
// ConvertValue
// Convert a long value into double
//-----------------------------------------------------------------------------
inline double HFCGrid::ConvertValue(int32_t   pi_Value)
    {
    return (double)pi_Value + 0.5;
    }

//-----------------------------------------------------------------------------
// public
// SetPrecision
// Set a new precision
//-----------------------------------------------------------------------------
inline void HFCGrid::SetPrecision(double pi_Precision)
    {
    m_Precision = pi_Precision;
    }

//-----------------------------------------------------------------------------
// public
// GetPrecision
// Return the current precision
//-----------------------------------------------------------------------------
inline double HFCGrid::GetPrecision() const
    {
    return m_Precision;
    }

//-----------------------------------------------------------------------------
// public
// GetXMin
// Gets the minimum X value of grid
//-----------------------------------------------------------------------------
inline int64_t HFCGrid::GetXMin() const
    {
    return (int64_t)floor(m_XMin + m_Precision);
    }

//-----------------------------------------------------------------------------
// public
// GetYMin
// Gets the minimum Y value of grid
//-----------------------------------------------------------------------------
inline int64_t HFCGrid::GetYMin() const
    {
    return (int64_t)floor(m_YMin + m_Precision);
    }

//-----------------------------------------------------------------------------
// public
// GetXMax
// Gets the maximum X value of grid
//-----------------------------------------------------------------------------
inline int64_t HFCGrid::GetXMax() const
    {
    if (HDOUBLE_EQUAL_EPSILON(m_XMin, m_XMax))
        return GetXMin();
    else
        return (int64_t)floor(m_XMax - m_Precision);
    }

//-----------------------------------------------------------------------------
// public
// GetYMax
// Gets the maximum Y value of grid
//-----------------------------------------------------------------------------
inline int64_t HFCGrid::GetYMax() const
    {
    if (HDOUBLE_EQUAL_EPSILON(m_YMin, m_YMax))
        return GetYMin();
    else
        return (int64_t)floor(m_YMax - m_Precision);
    }

//-----------------------------------------------------------------------------
// public
// GetWidth
// Gets the width of the grid
//-----------------------------------------------------------------------------
inline int64_t HFCGrid::GetWidth() const
    {
#if defined (ANDROID) || defined (__APPLE__)
        return llabs(GetXMax() - GetXMin() + 1);       //DM-Android move that on BeNumeric? 
#elif defined (_WIN32)
        return _abs64(GetXMax() - GetXMin() + 1);
#endif
    }

//-----------------------------------------------------------------------------
// public
// GetHeight
// Gets the height of the grid
//-----------------------------------------------------------------------------
inline int64_t HFCGrid::GetHeight() const
    {
#if defined (ANDROID) || defined (__APPLE__)
        return llabs(GetYMax() - GetYMin() + 1);
#elif defined (_WIN32)
        return _abs64(GetYMax() - GetYMin() + 1);
#endif
    }

END_IMAGEPP_NAMESPACE