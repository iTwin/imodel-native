//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLiteExtent.hpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent::HGF2DLiteExtent ()
    : m_XMin(0.0),
      m_XMax(0.0),
      m_YMin(0.0),
      m_YMax(0.0)
    {
   m_initializedXMin = false;
   m_initializedYMin = false;
   m_initializedXMax = false;
   m_initializedYMax = false;
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent::HGF2DLiteExtent(const HGF2DPosition& pi_rOrigin,
                                        const HGF2DPosition& pi_rCorner)
    : m_XMin(pi_rOrigin.GetX()),
      m_XMax(pi_rCorner.GetX()),
      m_YMin(pi_rOrigin.GetY()),
      m_YMax(pi_rCorner.GetY())
    {
    // The origin must contain lower or equal values to corner
    HPRECONDITION(pi_rOrigin.GetX() <= pi_rCorner.GetX());
    HPRECONDITION(pi_rOrigin.GetY() <= pi_rCorner.GetY());

    m_initializedXMin = true;
    m_initializedYMin = true;
    m_initializedXMax = true;
    m_initializedYMax = true;
    }

//-----------------------------------------------------------------------------
// Constructor with raw coordinates
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent::HGF2DLiteExtent(double pi_X1,
                                        double pi_Y1,
                                        double pi_X2,
                                        double pi_Y2)
    : m_XMin(MIN(pi_X1, pi_X2)),
      m_XMax(MAX(pi_X1, pi_X2)),
      m_YMin(MIN(pi_Y1, pi_Y2)),
      m_YMax(MAX(pi_Y1, pi_Y2))
    {
    m_initializedXMin = true;
    m_initializedYMin = true;
    m_initializedXMax = true;
    m_initializedYMax = true;
    }


//-----------------------------------------------------------------------------
// The copy constructor.
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent::HGF2DLiteExtent(const HGF2DLiteExtent& pi_rObj)
    : m_XMin (pi_rObj.m_XMin),
      m_XMax (pi_rObj.m_XMax),
      m_YMin (pi_rObj.m_YMin),
      m_YMax (pi_rObj.m_YMax)
    {
    m_initializedXMin = pi_rObj.m_initializedXMin;
    m_initializedYMin = pi_rObj.m_initializedYMin;
    m_initializedXMax = pi_rObj.m_initializedXMax;
    m_initializedYMax = pi_rObj.m_initializedYMax;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent::~HGF2DLiteExtent()
    {
    }


//-----------------------------------------------------------------------------
// Copy operator.
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent& HGF2DLiteExtent::operator=(const HGF2DLiteExtent& pi_rObj)
    {
    m_XMin = pi_rObj.m_XMin;
    m_XMax = pi_rObj.m_XMax;
    m_YMin = pi_rObj.m_YMin;
    m_YMax = pi_rObj.m_YMax;

    m_initializedXMin = pi_rObj.m_initializedXMin;
    m_initializedYMin = pi_rObj.m_initializedYMin;
    m_initializedXMax = pi_rObj.m_initializedXMax;
    m_initializedYMax = pi_rObj.m_initializedYMax;

    return (*this);
    }


//-----------------------------------------------------------------------------
// Equality evaluation operator.
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::operator==(const HGF2DLiteExtent& pi_rObj) const
    {
    HPRECONDITION(pi_rObj.m_initializedXMin);
    HPRECONDITION(pi_rObj.m_initializedYMin);
    HPRECONDITION(pi_rObj.m_initializedXMax);
    HPRECONDITION(pi_rObj.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);    
    HPRECONDITION(m_initializedXMax);    
    HPRECONDITION(m_initializedYMin);    
    HPRECONDITION(m_initializedYMax);

    // Compare origin and corners
    return(((pi_rObj.m_XMin == m_XMin) && (pi_rObj.m_XMax == m_XMax)) &&
           ((pi_rObj.m_YMin == m_YMin) && (pi_rObj.m_YMax == m_YMax)));
    }


//-----------------------------------------------------------------------------
// See operator== : returns opposite value.
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::operator!=(const HGF2DLiteExtent& pi_rObj) const
    {
    HPRECONDITION(pi_rObj.m_initializedXMin);
    HPRECONDITION(pi_rObj.m_initializedYMin);
    HPRECONDITION(pi_rObj.m_initializedXMax);
    HPRECONDITION(pi_rObj.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    return (!operator==(pi_rObj));
    }

//-----------------------------------------------------------------------------
// IsEqualTo
// Equality evaluation operator with epsilon.
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::IsEqualTo(const HGF2DLiteExtent& pi_rObj) const
    {
    HPRECONDITION(pi_rObj.m_initializedXMin);
    HPRECONDITION(pi_rObj.m_initializedYMin);
    HPRECONDITION(pi_rObj.m_initializedXMax);
    HPRECONDITION(pi_rObj.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    // Compare origin and corners
    return((HNumeric<double>::EQUAL_EPSILON(pi_rObj.m_XMin, m_XMin) &&
            HNumeric<double>::EQUAL_EPSILON(pi_rObj.m_XMax, m_XMax)) &&
           (HNumeric<double>::EQUAL_EPSILON(pi_rObj.m_YMin, m_YMin) &&
            HNumeric<double>::EQUAL_EPSILON(pi_rObj.m_YMax, m_YMax)));
    }


//-----------------------------------------------------------------------------
// IsEqualTo
// Equality evaluation operator with epsilon.
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::IsEqualTo(const HGF2DLiteExtent& pi_rObj, double pi_Epsilon) const
    {
    HPRECONDITION(pi_rObj.m_initializedXMin);
    HPRECONDITION(pi_rObj.m_initializedYMin);
    HPRECONDITION(pi_rObj.m_initializedXMax);
    HPRECONDITION(pi_rObj.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Epsilon >= 0.0);

    // Compare origin and corners
    return((HNumeric<double>::EQUAL(pi_rObj.m_XMin, m_XMin, pi_Epsilon) &&
            HNumeric<double>::EQUAL(pi_rObj.m_XMax, m_XMax, pi_Epsilon)) &&
           (HNumeric<double>::EQUAL(pi_rObj.m_YMin, m_YMin, pi_Epsilon) &&
            HNumeric<double>::EQUAL(pi_rObj.m_YMax, m_YMax, pi_Epsilon)));
    }

//-----------------------------------------------------------------------------
// SetXMin
// Sets the XMin value
//-----------------------------------------------------------------------------
inline void HGF2DLiteExtent::SetXMin(double pi_XMin)
    {
    HPRECONDITION(m_XMax >= pi_XMin);

    // Set XMin
    m_XMin = pi_XMin;

    m_initializedXMin = true;
    }

//-----------------------------------------------------------------------------
// SetYMin
// Sets the YMin value
//-----------------------------------------------------------------------------
inline void HGF2DLiteExtent::SetYMin(double pi_YMin)
    {
    HPRECONDITION(m_YMax >= pi_YMin);

    // Set YMin
    m_YMin = pi_YMin;
    m_initializedYMin = true;
    }


//-----------------------------------------------------------------------------
// SetXMax
// Sets the XMax value
//-----------------------------------------------------------------------------
inline void HGF2DLiteExtent::SetXMax(double pi_XMax)
    {
    HPRECONDITION(m_XMin <= pi_XMax);

    // Set XMax
    m_XMax = pi_XMax;

    m_initializedXMax = true;
    }

//-----------------------------------------------------------------------------
// SetYMax
// Sets the YMax value
//-----------------------------------------------------------------------------
inline void HGF2DLiteExtent::SetYMax(double pi_YMax)
    {
    HPRECONDITION(m_YMin <= pi_YMax);

    // Set YMax
    m_YMax = pi_YMax;

    m_initializedYMax = true;
    }

//-----------------------------------------------------------------------------
// SetOrigin
// Sets the origin of the extent
//-----------------------------------------------------------------------------
inline void HGF2DLiteExtent::SetOrigin(const HGF2DPosition& pi_rOrigin)
    {
    // The given origin must be smaller that current corner (or not be defined)
    HPRECONDITION(pi_rOrigin.GetX() <= m_XMax);
    HPRECONDITION(pi_rOrigin.GetY() <= m_YMax);

    m_XMin = pi_rOrigin.GetX();
    m_YMin = pi_rOrigin.GetY();

    m_initializedXMin = true;
    m_initializedYMin = true;
    }


//-----------------------------------------------------------------------------
// SetCorner
// Sets the corner of the extent
//-----------------------------------------------------------------------------
inline void HGF2DLiteExtent::SetCorner(const HGF2DPosition& pi_rCorner)
    {
    // The given corner must be greater that current origin (or not be defined)
    HPRECONDITION(pi_rCorner.GetX() >= m_XMin);
    HPRECONDITION(pi_rCorner.GetY() >= m_YMin);

    m_XMax = pi_rCorner.GetX();
    m_YMax = pi_rCorner.GetY();

    m_initializedXMax = true;
    m_initializedYMax = true;
    }


/** -----------------------------------------------------------------------------
    Checks this extent for definition.  An undefined extent object is one for
    which dimension is not completely defined. It will return true if this
    extent is completely defined. To be completely defined, an extent must
    have values set for all of its XMin, XMax, YMin, or YMax values.
    But the definition of XMin implies that XMax is defined and vice-versa,
    and the same for Y.

    @return true is extent is defined, false otherwise.


    -----------------------------------------------------------------------------
*/
inline bool HGF2DLiteExtent::IsDefined () const
    {
    // Floating-point exact comparison is intentional
    return (m_initializedXMin && m_initializedXMax && m_initializedYMin && m_initializedYMax);
    }


//-----------------------------------------------------------------------------
// IsPointIn
// Indicates if a point is inside the extent
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::IsPointIn (const HGF2DPosition& pi_rPoint) const
    {
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

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
inline bool HGF2DLiteExtent::IsPointInnerIn (const HGF2DPosition& pi_rPoint) const
    {

    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    // Check if point lies inside
    return (HNumeric<double>::GREATER_EPSILON(pi_rPoint.GetX(), m_XMin) &&
            HNumeric<double>::SMALLER_EPSILON(pi_rPoint.GetX(), m_XMax) &&
            HNumeric<double>::GREATER_EPSILON(pi_rPoint.GetY(), m_YMin) &&
            HNumeric<double>::SMALLER_EPSILON(pi_rPoint.GetY(), m_YMax));

    }


//-----------------------------------------------------------------------------
// IsPointOutterIn
// Indicates if a point is inside the extent
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::IsPointOutterIn (const HGF2DPosition& pi_rPoint) const
    {
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    // Check if point lies inside (Borders are inclusive here)
    return (HNumeric<double>::GREATER_OR_EQUAL_EPSILON(pi_rPoint.GetX(), m_XMin) &&
            HNumeric<double>::SMALLER_OR_EQUAL_EPSILON(pi_rPoint.GetX(), m_XMax) &&
            HNumeric<double>::GREATER_OR_EQUAL_EPSILON(pi_rPoint.GetY(), m_YMin) &&
            HNumeric<double>::SMALLER_OR_EQUAL_EPSILON(pi_rPoint.GetY(), m_YMax));
    }


//-----------------------------------------------------------------------------
// IsPointInnerIn
// Indicates if a point is inside the extent
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::IsPointInnerIn(const HGF2DPosition& pi_rPoint,
                                             double pi_Tolerance) const
    {

    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Tolerance >= 0.0);

    // Check if point lies inside
    return (HNumeric<double>::GREATER(pi_rPoint.GetX(), m_XMin, pi_Tolerance) &&
            HNumeric<double>::SMALLER(pi_rPoint.GetX(), m_XMax, pi_Tolerance) &&
            HNumeric<double>::GREATER(pi_rPoint.GetY(), m_YMin, pi_Tolerance) &&
            HNumeric<double>::SMALLER(pi_rPoint.GetY(), m_YMax, pi_Tolerance));

    }


//-----------------------------------------------------------------------------
// IsPointOutterIn
// Indicates if a point is inside the extent
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::IsPointOutterIn(const HGF2DPosition& pi_rPoint,
                                              double pi_Tolerance) const
    {
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Tolerance >= 0.0);

    // Check if point lies inside (Borders are inclusive here)
    return (HNumeric<double>::GREATER_OR_EQUAL(pi_rPoint.GetX(), m_XMin, pi_Tolerance) &&
            HNumeric<double>::SMALLER_OR_EQUAL(pi_rPoint.GetX(), m_XMax, pi_Tolerance) &&
            HNumeric<double>::GREATER_OR_EQUAL(pi_rPoint.GetY(), m_YMin, pi_Tolerance) &&
            HNumeric<double>::SMALLER_OR_EQUAL(pi_rPoint.GetY(), m_YMax, pi_Tolerance));
    }

//-----------------------------------------------------------------------------
// GetWidth
// Returns the width of extent
//-----------------------------------------------------------------------------
inline double HGF2DLiteExtent::GetWidth() const
    {
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);

    return(m_XMax - m_XMin);
    }

//-----------------------------------------------------------------------------
// GetHeight
// Returns the height of extent
//-----------------------------------------------------------------------------
inline double HGF2DLiteExtent::GetHeight() const
    {
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    return(m_YMax - m_YMin);
    }


//-----------------------------------------------------------------------------
// GetXMin
// Returns the XMin value
//-----------------------------------------------------------------------------
inline double HGF2DLiteExtent::GetXMin() const
    {
    HPRECONDITION(m_initializedXMin);

    return(m_XMin);
    }


//-----------------------------------------------------------------------------
// GetYMin
// Returns the YMin value
//-----------------------------------------------------------------------------
inline double HGF2DLiteExtent::GetYMin() const
    {

    HPRECONDITION(m_initializedYMin);
    
    return (m_YMin);
    }


//-----------------------------------------------------------------------------
// GetXMax
// Returns the XMax value
//-----------------------------------------------------------------------------
inline double HGF2DLiteExtent::GetXMax() const
    {
    HPRECONDITION(m_initializedXMax);

    return (m_XMax);
    }


//-----------------------------------------------------------------------------
// GetYMax
// Returns the YMax value
//-----------------------------------------------------------------------------
inline double HGF2DLiteExtent::GetYMax() const
    {
    HPRECONDITION(m_initializedYMax);

    return (m_YMax);
    }

//-----------------------------------------------------------------------------
// GetOrigin
// Returns the origin of the extent
//-----------------------------------------------------------------------------
inline HGF2DPosition HGF2DLiteExtent::GetOrigin() const
    {
    HPRECONDITION(m_initializedXMin && m_initializedYMin);

    return (HGF2DPosition(m_XMin, m_YMin));
    }


//-----------------------------------------------------------------------------
// GetCorner
// Returns the corner of the extent
//-----------------------------------------------------------------------------
inline HGF2DPosition HGF2DLiteExtent::GetCorner() const
    {
    HPRECONDITION(m_initializedXMax && m_initializedYMax);

    return (HGF2DPosition(m_XMax, m_YMax));
    }

//-----------------------------------------------------------------------------
// Overlaps
// Checks if the two extents overlap
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::Overlaps(const HGF2DLiteExtent& pi_rExtent) const
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

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
inline bool HGF2DLiteExtent::OutterOverlaps(const HGF2DLiteExtent& pi_rExtent) const
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    // The second part is required for horizontal or horizontal objects (0 thickness)
    // NOTE that two null sized extents may not overlap
    return(HNumeric<double>::GREATER_OR_EQUAL_EPSILON(m_XMax, pi_rExtent.m_XMin) &&
           HNumeric<double>::SMALLER_OR_EQUAL_EPSILON(m_XMin, pi_rExtent.m_XMax) &&
           HNumeric<double>::GREATER_OR_EQUAL_EPSILON(m_YMax, pi_rExtent.m_YMin) &&
           HNumeric<double>::SMALLER_OR_EQUAL_EPSILON(m_YMin, pi_rExtent.m_YMax));

    }


//-----------------------------------------------------------------------------
// OutterOverlaps
// Checks if the two extents overlap (with EPSILON application)
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::OutterOverlaps(const HGF2DLiteExtent& pi_rExtent,
                                             double pi_Epsilon) const
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Epsilon >= 0.0);

    // The second part is required for horizontal or horizontal objects (0 thickness)
    // NOTE that two null sized extents may not overlap
    return(HNumeric<double>::GREATER_OR_EQUAL(m_XMax, pi_rExtent.m_XMin, pi_Epsilon) &&
           HNumeric<double>::SMALLER_OR_EQUAL(m_XMin, pi_rExtent.m_XMax, pi_Epsilon) &&
           HNumeric<double>::GREATER_OR_EQUAL(m_YMax, pi_rExtent.m_YMin, pi_Epsilon) &&
           HNumeric<double>::SMALLER_OR_EQUAL(m_YMin, pi_rExtent.m_YMax, pi_Epsilon));
    }


//-----------------------------------------------------------------------------
// InnerOverlaps
// Checks if the two extents overlap (with EPSILON application on inside)
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::InnerOverlaps(const HGF2DLiteExtent& pi_rExtent) const
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    return(HNumeric<double>::GREATER_EPSILON(m_XMax, pi_rExtent.m_XMin) &&
           HNumeric<double>::SMALLER_EPSILON(m_XMin, pi_rExtent.m_XMax) &&
           HNumeric<double>::GREATER_EPSILON(m_YMax, pi_rExtent.m_YMin) &&
           HNumeric<double>::SMALLER_EPSILON(m_YMin, pi_rExtent.m_YMax));
    }


//-----------------------------------------------------------------------------
// InnerOverlaps
// Checks if the two extents overlap (with EPSILON application)
//-----------------------------------------------------------------------------
inline bool HGF2DLiteExtent::InnerOverlaps(const HGF2DLiteExtent& pi_rExtent,
                                            double pi_Epsilon) const
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Epsilon >= 0.0);

    // The second part is required for horizontal or horizontal objects (0 thickness)
    // NOTE that two null sized extents may not overlap
    return(HNumeric<double>::GREATER(m_XMax, pi_rExtent.m_XMin, pi_Epsilon) &&
           HNumeric<double>::SMALLER(m_XMin, pi_rExtent.m_XMax, pi_Epsilon) &&
           HNumeric<double>::GREATER(m_YMax, pi_rExtent.m_YMin, pi_Epsilon) &&
           HNumeric<double>::SMALLER(m_YMin, pi_rExtent.m_YMax, pi_Epsilon));
    }


//-----------------------------------------------------------------------------
// Set
// Sets the extent
//-----------------------------------------------------------------------------
inline void HGF2DLiteExtent::Set(double pi_X1,
                                 double pi_Y1,
                                 double pi_X2,
                                 double pi_Y2)
    {
    m_initializedXMin = true;
    m_initializedYMin = true;
    m_initializedXMax = true;
    m_initializedYMax = true;

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
inline void HGF2DLiteExtent::Add(const HGF2DPosition& pi_rLocation)
    {
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

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
inline void HGF2DLiteExtent::Add (const HGF2DLiteExtent& pi_rExtent)
    {
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    if (m_XMin > pi_rExtent.m_XMin)
        m_XMin = pi_rExtent.m_XMin;

    if (m_XMax < pi_rExtent.m_XMax)
        m_XMax = pi_rExtent.m_XMax;

    if (m_YMin > pi_rExtent.m_YMin)
        m_YMin = pi_rExtent.m_YMin;

    if (m_YMax < pi_rExtent.m_YMax)
        m_YMax = pi_rExtent.m_YMax;
    }

//-----------------------------------------------------------------------------
// Union
// Changes extent so that contains both extents
//-----------------------------------------------------------------------------
inline void HGF2DLiteExtent::Union(const HGF2DLiteExtent& pi_rExtent)
    {
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    Add(pi_rExtent);
    }
END_IMAGEPP_NAMESPACE