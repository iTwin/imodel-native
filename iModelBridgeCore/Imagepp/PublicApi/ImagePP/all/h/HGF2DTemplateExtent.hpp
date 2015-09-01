//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DTemplateExtent.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
template <class DataType, class COORD> HGF2DTemplateExtent<DataType, COORD>::HGF2DTemplateExtent ()
    : m_XMin(0.0),
      m_XMax(0.0),
      m_YMin(0.0),
      m_YMax(0.0)
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template <class DataType, class COORD> HGF2DTemplateExtent<DataType, COORD>::HGF2DTemplateExtent(const COORD& pi_rOrigin,
        const COORD& pi_rCorner)
    : m_XMin(pi_rOrigin.GetX()),
      m_XMax(pi_rCorner.GetX()),
      m_YMin(pi_rOrigin.GetY()),
      m_YMax(pi_rCorner.GetY())
    {
    // The origin must contain lower or equal values to corner
    HPRECONDITION(pi_rOrigin.GetX() <= pi_rCorner.GetX());
    HPRECONDITION(pi_rOrigin.GetY() <= pi_rCorner.GetY());
    }

//-----------------------------------------------------------------------------
// Constructor with raw coordinates
//-----------------------------------------------------------------------------
template <class DataType, class COORD> HGF2DTemplateExtent<DataType, COORD>::HGF2DTemplateExtent(DataType pi_X1,
        DataType pi_Y1,
        DataType pi_X2,
        DataType pi_Y2)
    : m_XMin(MIN(pi_X1, pi_X2)),
      m_XMax(MAX(pi_X1, pi_X2)),
      m_YMin(MIN(pi_Y1, pi_Y2)),
      m_YMax(MAX(pi_Y1, pi_Y2))
    {
    }


//-----------------------------------------------------------------------------
// The copy constructor.
//-----------------------------------------------------------------------------
template <class DataType, class COORD> HGF2DTemplateExtent<DataType, COORD>::HGF2DTemplateExtent(const HGF2DTemplateExtent& pi_rObj)
    : m_XMin (pi_rObj.m_XMin),
      m_XMax (pi_rObj.m_XMax),
      m_YMin (pi_rObj.m_YMin),
      m_YMax (pi_rObj.m_YMax)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
template <class DataType, class COORD> HGF2DTemplateExtent<DataType, COORD>::~HGF2DTemplateExtent()
    {
    }


//-----------------------------------------------------------------------------
// Copy operator.
//-----------------------------------------------------------------------------
template <class DataType, class COORD> HGF2DTemplateExtent<DataType, COORD>& HGF2DTemplateExtent<DataType, COORD>::operator=(const HGF2DTemplateExtent& pi_rObj)
    {
    m_XMin = pi_rObj.m_XMin;
    m_XMax = pi_rObj.m_XMax;
    m_YMin = pi_rObj.m_YMin;
    m_YMax = pi_rObj.m_YMax;

    return (*this);
    }


//-----------------------------------------------------------------------------
// Equality evaluation operator.
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::operator==(const HGF2DTemplateExtent<DataType, COORD>& pi_rObj) const
    {
    // Compare origin and corners
    return(((pi_rObj.m_XMin == m_XMin) && (pi_rObj.m_XMax == m_XMax)) &&
           ((pi_rObj.m_YMin == m_YMin) && (pi_rObj.m_YMax == m_YMax)));
    }


//-----------------------------------------------------------------------------
// See operator== : returns opposite value.
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::operator!=(const HGF2DTemplateExtent<DataType, COORD>& pi_rObj) const
    {
    return (!operator==(pi_rObj));
    }

//-----------------------------------------------------------------------------
// IsEqualTo
// Equality evaluation operator with epsilon.
//-----------------------------------------------------------------------------
template <class DataType, class COORD> int HGF2DTemplateExtent<DataType, COORD>::IsEqualTo(const HGF2DTemplateExtent<DataType, COORD>& pi_rObj) const
    {
    // Compare origin and corners
    return((HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_XMin, m_XMin) &&
            HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_XMax, m_XMax)) &&
           (HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_YMin, m_YMin) &&
            HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_YMax, m_YMax)));
    }


//-----------------------------------------------------------------------------
// IsEqualTo
// Equality evaluation operator with epsilon.
//-----------------------------------------------------------------------------
template <class DataType, class COORD> int HGF2DTemplateExtent<DataType, COORD>::IsEqualTo(const HGF2DTemplateExtent<DataType, COORD>& pi_rObj, DataType pi_Epsilon) const
    {
    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Epsilon >= 0.0);

    // Compare origin and corners
    return((HNumeric<DataType>::EQUAL(pi_rObj.m_XMin, m_XMin, pi_Epsilon) &&
            HNumeric<DataType>::EQUAL(pi_rObj.m_XMax, m_XMax, pi_Epsilon)) &&
           (HNumeric<DataType>::EQUAL(pi_rObj.m_YMin, m_YMin, pi_Epsilon) &&
            HNumeric<DataType>::EQUAL(pi_rObj.m_YMax, m_YMax, pi_Epsilon)));
    }

//-----------------------------------------------------------------------------
// SetXMin
// Sets the XMin value
//-----------------------------------------------------------------------------
template <class DataType, class COORD> void HGF2DTemplateExtent<DataType, COORD>::SetXMin(DataType pi_XMin)
    {
    HPRECONDITION(m_XMax >= pi_XMin);

    // Set XMin
    m_XMin = pi_XMin;
    }

//-----------------------------------------------------------------------------
// SetYMin
// Sets the YMin value
//-----------------------------------------------------------------------------
template <class DataType, class COORD> void HGF2DTemplateExtent<DataType, COORD>::SetYMin(DataType pi_YMin)
    {
    HPRECONDITION(m_YMax >= pi_YMin);

    // Set YMin
    m_YMin = pi_YMin;
    }


//-----------------------------------------------------------------------------
// SetXMax
// Sets the XMax value
//-----------------------------------------------------------------------------
template <class DataType, class COORD> void HGF2DTemplateExtent<DataType, COORD>::SetXMax(DataType pi_XMax)
    {
    HPRECONDITION(m_XMin <= pi_XMax);

    // Set XMax
    m_XMax = pi_XMax;
    }

//-----------------------------------------------------------------------------
// SetYMax
// Sets the YMax value
//-----------------------------------------------------------------------------
template <class DataType, class COORD> void HGF2DTemplateExtent<DataType, COORD>::SetYMax(DataType pi_YMax)
    {
    HPRECONDITION(m_YMin <= pi_YMax);

    // Set YMax
    m_YMax = pi_YMax;
    }

//-----------------------------------------------------------------------------
// SetOrigin
// Sets the origin of the extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> void HGF2DTemplateExtent<DataType, COORD>::SetOrigin(const COORD& pi_rOrigin)
    {
    // The given origin must be smaller that current corner (or not be defined)
    HPRECONDITION(pi_rOrigin.GetX() <= m_XMax);
    HPRECONDITION(pi_rOrigin.GetY() <= m_YMax);

    m_XMin = pi_rOrigin.GetX();
    m_YMin = pi_rOrigin.GetY();
    }


//-----------------------------------------------------------------------------
// SetCorner
// Sets the corner of the extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> void HGF2DTemplateExtent<DataType, COORD>::SetCorner(const COORD& pi_rCorner)
    {
    // The given corner must be greater that current origin (or not be defined)
    HPRECONDITION(pi_rCorner.GetX() >= m_XMin);
    HPRECONDITION(pi_rCorner.GetY() >= m_YMin);

    m_XMax = pi_rCorner.GetX();
    m_YMax = pi_rCorner.GetY();
    }



//-----------------------------------------------------------------------------
// IsPointIn
// Indicates if a point is inside the extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::IsPointIn (const COORD& pi_rPoint) const
    {
    // Check if point lies inside (Borders are inclusive here)
    return ((pi_rPoint.GetX() >= m_XMin) &&
            (pi_rPoint.GetX() <= m_XMax) &&
            (pi_rPoint.GetY() >= m_YMin) &&
            (pi_rPoint.GetY() <= m_YMax));
    }

//-----------------------------------------------------------------------------
// IsPointInnerIn
// Indicates if a point is inside the extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::IsPointInnerIn (const COORD& pi_rPoint) const
    {
    // Check if point lies inside
    return (HNumeric<DataType>::GREATER_EPSILON(pi_rPoint.GetX(), m_XMin) &&
            HNumeric<DataType>::SMALLER_EPSILON(pi_rPoint.GetX(), m_XMax) &&
            HNumeric<DataType>::GREATER_EPSILON(pi_rPoint.GetY(), m_YMin) &&
            HNumeric<DataType>::SMALLER_EPSILON(pi_rPoint.GetY(), m_YMax));

    }


//-----------------------------------------------------------------------------
// IsPointOutterIn
// Indicates if a point is inside the extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::IsPointOutterIn (const COORD& pi_rPoint) const
    {
    // Check if point lies inside (Borders are inclusive here)
    return (HNumeric<DataType>::GREATER_OR_EQUAL_EPSILON(pi_rPoint.GetX(), m_XMin) &&
            HNumeric<DataType>::SMALLER_OR_EQUAL_EPSILON(pi_rPoint.GetX(), m_XMax) &&
            HNumeric<DataType>::GREATER_OR_EQUAL_EPSILON(pi_rPoint.GetY(), m_YMin) &&
            HNumeric<DataType>::SMALLER_OR_EQUAL_EPSILON(pi_rPoint.GetY(), m_YMax));
    }


//-----------------------------------------------------------------------------
// IsPointInnerIn
// Indicates if a point is inside the extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::IsPointInnerIn(const COORD& pi_rPoint,
        DataType pi_Tolerance) const
    {
    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Tolerance >= 0.0);

    // Check if point lies inside
    return (HNumeric<DataType>::GREATER(pi_rPoint.GetX(), m_XMin, pi_Tolerance) &&
            HNumeric<DataType>::SMALLER(pi_rPoint.GetX(), m_XMax, pi_Tolerance) &&
            HNumeric<DataType>::GREATER(pi_rPoint.GetY(), m_YMin, pi_Tolerance) &&
            HNumeric<DataType>::SMALLER(pi_rPoint.GetY(), m_YMax, pi_Tolerance));

    }


//-----------------------------------------------------------------------------
// IsPointOutterIn
// Indicates if a point is inside the extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::IsPointOutterIn(const COORD& pi_rPoint,
        DataType pi_Tolerance) const
    {
    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Tolerance >= 0.0);

    // Check if point lies inside (Borders are inclusive here)
    return (HNumeric<DataType>::GREATER_OR_EQUAL(pi_rPoint.GetX(), m_XMin, pi_Tolerance) &&
            HNumeric<DataType>::SMALLER_OR_EQUAL(pi_rPoint.GetX(), m_XMax, pi_Tolerance) &&
            HNumeric<DataType>::GREATER_OR_EQUAL(pi_rPoint.GetY(), m_YMin, pi_Tolerance) &&
            HNumeric<DataType>::SMALLER_OR_EQUAL(pi_rPoint.GetY(), m_YMax, pi_Tolerance));
    }

//-----------------------------------------------------------------------------
// GetWidth
// Returns the width of extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> DataType HGF2DTemplateExtent<DataType, COORD>::GetWidth() const
    {
    return(m_XMax - m_XMin);
    }

//-----------------------------------------------------------------------------
// GetHeight
// Returns the height of extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> DataType HGF2DTemplateExtent<DataType, COORD>::GetHeight() const
    {
    return(m_YMax - m_YMin);
    }


//-----------------------------------------------------------------------------
// GetXMin
// Returns the XMin value
//-----------------------------------------------------------------------------
template <class DataType, class COORD> DataType HGF2DTemplateExtent<DataType, COORD>::GetXMin() const
    {
    return(m_XMin);
    }


//-----------------------------------------------------------------------------
// GetYMin
// Returns the YMin value
//-----------------------------------------------------------------------------
template <class DataType, class COORD> DataType HGF2DTemplateExtent<DataType, COORD>::GetYMin() const
    {
    return (m_YMin);
    }


//-----------------------------------------------------------------------------
// GetXMax
// Returns the XMax value
//-----------------------------------------------------------------------------
template <class DataType, class COORD> DataType HGF2DTemplateExtent<DataType, COORD>::GetXMax() const
    {
    return (m_XMax);
    }


//-----------------------------------------------------------------------------
// GetYMax
// Returns the YMax value
//-----------------------------------------------------------------------------
template <class DataType, class COORD> DataType HGF2DTemplateExtent<DataType, COORD>::GetYMax() const
    {
    return (m_YMax);
    }

//-----------------------------------------------------------------------------
// GetOrigin
// Returns the origin of the extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> COORD HGF2DTemplateExtent<DataType, COORD>::GetOrigin() const
    {
    return (COORD(m_XMin, m_YMin));
    }


//-----------------------------------------------------------------------------
// GetCorner
// Returns the corner of the extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> COORD HGF2DTemplateExtent<DataType, COORD>::GetCorner() const
    {
    return (COORD(m_XMax, m_YMax));
    }

//-----------------------------------------------------------------------------
// Overlaps
// Checks if the two extents overlap
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::Overlaps(const HGF2DTemplateExtent& pi_rExtent) const
    {
    // The second part is required for horizontal or horizontal objects (0 thickness)
    // NOTE that two null sized extents may not overlap
    return((m_XMax > pi_rExtent.m_XMin) &&
           (m_XMin < pi_rExtent.m_XMax) &&
           (m_YMax > pi_rExtent.m_YMin) &&
           (m_YMin < pi_rExtent.m_YMax));
    }


//-----------------------------------------------------------------------------
// OutterOverlaps
// Checks if the two extents overlap (with EPSILON application on outside)
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::OutterOverlaps(const HGF2DTemplateExtent& pi_rExtent) const
    {
    // The second part is required for horizontal or horizontal objects (0 thickness)
    // NOTE that two null sized extents may not overlap
    return(HNumeric<DataType>::GREATER_OR_EQUAL_EPSILON(m_XMax, pi_rExtent.m_XMin) &&
           HNumeric<DataType>::SMALLER_OR_EQUAL_EPSILON(m_XMin, pi_rExtent.m_XMax) &&
           HNumeric<DataType>::GREATER_OR_EQUAL_EPSILON(m_YMax, pi_rExtent.m_YMin) &&
           HNumeric<DataType>::SMALLER_OR_EQUAL_EPSILON(m_YMin, pi_rExtent.m_YMax));

    }


//-----------------------------------------------------------------------------
// OutterOverlaps
// Checks if the two extents overlap (with EPSILON application)
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::OutterOverlaps(const HGF2DTemplateExtent& pi_rExtent,
        DataType pi_Epsilon) const
    {
    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Epsilon >= 0.0);

    // The second part is required for horizontal or horizontal objects (0 thickness)
    // NOTE that two null sized extents may not overlap
    return(HNumeric<DataType>::GREATER_OR_EQUAL(m_XMax, pi_rExtent.m_XMin, pi_Epsilon) &&
           HNumeric<DataType>::SMALLER_OR_EQUAL(m_XMin, pi_rExtent.m_XMax, pi_Epsilon) &&
           HNumeric<DataType>::GREATER_OR_EQUAL(m_YMax, pi_rExtent.m_YMin, pi_Epsilon) &&
           HNumeric<DataType>::SMALLER_OR_EQUAL(m_YMin, pi_rExtent.m_YMax, pi_Epsilon));
    }


//-----------------------------------------------------------------------------
// InnerOverlaps
// Checks if the two extents overlap (with EPSILON application on inside)
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::InnerOverlaps(const HGF2DTemplateExtent& pi_rExtent) const
    {
    return(HNumeric<DataType>::GREATER_EPSILON(m_XMax, pi_rExtent.m_XMin) &&
           HNumeric<DataType>::SMALLER_EPSILON(m_XMin, pi_rExtent.m_XMax) &&
           HNumeric<DataType>::GREATER_EPSILON(m_YMax, pi_rExtent.m_YMin) &&
           HNumeric<DataType>::SMALLER_EPSILON(m_YMin, pi_rExtent.m_YMax));
    }


//-----------------------------------------------------------------------------
// InnerOverlaps
// Checks if the two extents overlap (with EPSILON application)
//-----------------------------------------------------------------------------
template <class DataType, class COORD> bool HGF2DTemplateExtent<DataType, COORD>::InnerOverlaps(const HGF2DTemplateExtent& pi_rExtent,
        DataType pi_Epsilon) const
    {
    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Epsilon >= 0.0);

    // The second part is required for horizontal or horizontal objects (0 thickness)
    // NOTE that two null sized extents may not overlap
    return(HNumeric<DataType>::GREATER(m_XMax, pi_rExtent.m_XMin, pi_Epsilon) &&
           HNumeric<DataType>::SMALLER(m_XMin, pi_rExtent.m_XMax, pi_Epsilon) &&
           HNumeric<DataType>::GREATER(m_YMax, pi_rExtent.m_YMin, pi_Epsilon) &&
           HNumeric<DataType>::SMALLER(m_YMin, pi_rExtent.m_YMax, pi_Epsilon));
    }


//-----------------------------------------------------------------------------
// Set
// Sets the extent
//-----------------------------------------------------------------------------
template <class DataType, class COORD> void HGF2DTemplateExtent<DataType, COORD>::Set(DataType pi_X1,
        DataType pi_Y1,
        DataType pi_X2,
        DataType pi_Y2)
    {
    // Set X values
    if (pi_X1 < pi_X2)
        {
        m_XMin = pi_X1;
        m_XMax = pi_X2;
        }
    else
        {
        m_XMin = pi_X2;
        m_XMax = pi_X1;
        }

    // Set Y values
    if (pi_Y1 < pi_Y2)
        {
        m_YMin = pi_Y1;
        m_YMax = pi_Y2;
        }
    else
        {
        m_YMin = pi_Y2;
        m_YMax = pi_Y1;
        }
    }




//-----------------------------------------------------------------------------
// Add
// Changes extent so that contains both extent and location
//-----------------------------------------------------------------------------
template <class DataType, class COORD> void HGF2DTemplateExtent<DataType, COORD>::Add(const COORD& pi_rLocation)
    {
    if (m_XMin > pi_rLocation.GetX())
        m_XMin = pi_rLocation.GetX();

    if (m_XMax < pi_rLocation.GetX())
        m_XMax = pi_rLocation.GetX();

    if (m_YMin > pi_rLocation.GetY())
        m_YMin = pi_rLocation.GetY();

    if (m_YMax < pi_rLocation.GetY())
        m_YMax = pi_rLocation.GetY();

    }


//-----------------------------------------------------------------------------
// Add
// Changes extent so that contains both extents
//-----------------------------------------------------------------------------
template <class DataType, class COORD> void HGF2DTemplateExtent<DataType, COORD>::Add (const HGF2DTemplateExtent& pi_rExtent)
    {
    if (m_XMin > pi_rExtent.m_XMin)
        m_XMin = pi_rExtent.m_XMin;

    if (m_XMax < pi_rExtent.m_XMax)
        m_XMax = pi_rExtent.m_XMax;

    if (m_YMin > pi_rExtent.m_YMin)
        m_YMin = pi_rExtent.m_YMin;

    if (m_YMax < pi_rExtent.m_YMax)
        m_YMax = pi_rExtent.m_YMax;
    }

END_IMAGEPP_NAMESPACE