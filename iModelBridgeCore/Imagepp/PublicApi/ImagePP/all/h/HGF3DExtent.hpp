/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HGF3DExtent.hpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : Extent3D
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default constructor
// The initial extrent is puntual at coordinate (0,0,0)
//-----------------------------------------------------------------------------
template <class DataType> HGF3DExtent<DataType>::HGF3DExtent()
    : m_XMin(0),
      m_YMin(0),
      m_ZMin(0),
      m_XMax(0),
      m_YMax(0),
      m_ZMax(0)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor by corners
// The extent is created and extent boundary set according to origin and corner
// provided. The origin represents the corner of extent box with the smallest
// dimensions for all values while the corner represents the point
// furthest from this origin. It follows that all origin point coordinate
// values must be smaller than corresponding values in the given corner.
//-----------------------------------------------------------------------------
template <class DataType> HGF3DExtent<DataType>::HGF3DExtent(const HGF3DCoord<DataType>& i_rOrigin,
                                                             const HGF3DCoord<DataType>& i_rCorner)
    : m_XMin(i_rOrigin.GetX()),
      m_YMin(i_rOrigin.GetY()),
      m_ZMin(i_rOrigin.GetZ()),
      m_XMax(i_rCorner.GetX()),
      m_YMax(i_rCorner.GetY()),
      m_ZMax(i_rCorner.GetZ())
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor by point
// The extent is created and extent boundary set according to origin
// provided. The extent is thus punctual at this point.
//-----------------------------------------------------------------------------
template <class DataType> HGF3DExtent<DataType>::HGF3DExtent(const HGF3DCoord<DataType>& i_rOrigin)
    : m_XMin(i_rOrigin.GetX()),
      m_YMin(i_rOrigin.GetY()),
      m_ZMin(i_rOrigin.GetZ()),
      m_XMax(i_rOrigin.GetX()),
      m_YMax(i_rOrigin.GetY()),
      m_ZMax(i_rOrigin.GetZ())
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor by values
// The extent is created and extent boundary set according to provided values.
// The minimum values for each dimensions must be smaller or equal to
// corresponding maximum values.
//-----------------------------------------------------------------------------
template <class DataType> HGF3DExtent<DataType>::HGF3DExtent(DataType i_XMin,
                                                             DataType i_YMin,
                                                             DataType i_ZMin,
                                                             DataType i_XMax,
                                                             DataType i_YMax,
                                                             DataType i_ZMax)
    : m_XMin(i_XMin),
      m_YMin(i_YMin),
      m_ZMin(i_ZMin),
      m_XMax(i_XMax),
      m_YMax(i_YMax),
      m_ZMax(i_ZMax)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
template <class DataType> HGF3DExtent<DataType>::HGF3DExtent(const HGF3DExtent<DataType>& i_rObject)
    : m_XMin(i_rObject.m_XMin),
      m_YMin(i_rObject.m_YMin),
      m_ZMin(i_rObject.m_ZMin),
      m_XMax(i_rObject.m_XMax),
      m_YMax(i_rObject.m_YMax),
      m_ZMax(i_rObject.m_ZMax)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Destroyer
//-----------------------------------------------------------------------------
template <class DataType> HGF3DExtent<DataType>::~HGF3DExtent()
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Assignment operator
//-----------------------------------------------------------------------------
template <class DataType> HGF3DExtent<DataType>& HGF3DExtent<DataType>::operator=(const HGF3DExtent<DataType>& i_rObject)
    {
    HINVARIANTS;

    // Check given is different from self
    if (this != &i_rObject)
        {
        // Copy members
        m_XMin = i_rObject.m_XMin;
        m_YMin = i_rObject.m_YMin;
        m_ZMin = i_rObject.m_ZMin;
        m_XMax = i_rObject.m_XMax;
        m_YMax = i_rObject.m_YMax;
        m_ZMax = i_rObject.m_ZMax;
        }

    // Return reference to self to be used as a l-value
    return(*this);
    }


//-----------------------------------------------------------------------------
// Strict equality operator
// Extents are considered strictly equal if all boundary values are
// exactly the same.
// This method returns true is extents are equal and false otherwise
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::operator==(const HGF3DExtent<DataType>& i_rObject) const
    {
    HINVARIANTS;
    return((m_XMin == i_rObject.m_XMin) &&
           (m_YMin == i_rObject.m_YMin) &&
           (m_ZMin == i_rObject.m_ZMin) &&
           (m_XMax == i_rObject.m_XMax) &&
           (m_YMax == i_rObject.m_YMax) &&
           (m_ZMax == i_rObject.m_ZMax));
    }

//-----------------------------------------------------------------------------
// Strict inquality operator
// Extents are considered strictly equal if all boundary values are
// exactly the same.
// This method returns true is extents are different and false otherwise
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::operator!=(const HGF3DExtent<DataType>& i_rObject) const
    {
    HINVARIANTS;
    return(!operator==(i_rObject));
    }


//-----------------------------------------------------------------------------
// Equality operator
// Extents are considered equal if all boundary values are the same within the
// specified tolerance.
// This method returns true is extents are equal and false otherwise
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::Equals(const HGF3DExtent<DataType>& i_rObject, DataType i_Tolerance) const
    {
    HINVARIANTS;
    return(HNumeric<DataType>::EQUAL(m_XMin, i_rObject.m_XMin, i_Tolerance) &&
           HNumeric<DataType>::EQUAL(m_YMin, i_rObject.m_YMin, i_Tolerance) &&
           HNumeric<DataType>::EQUAL(m_ZMin, i_rObject.m_ZMin, i_Tolerance) &&
           HNumeric<DataType>::EQUAL(m_XMax, i_rObject.m_XMax, i_Tolerance) &&
           HNumeric<DataType>::EQUAL(m_YMax, i_rObject.m_YMax, i_Tolerance) &&
           HNumeric<DataType>::EQUAL(m_ZMax, i_rObject.m_ZMax, i_Tolerance));

    }

//-----------------------------------------------------------------------------
// IsPointIn
// Indicates if a point is inside the extent. If properly indicates, then the
// boundary may be included inside the extent, and this boundary may be given a size
// by specification of an optional tolerance. The default tolerance is 0 and
// the boundary is considered not inside the extent.
//
// The method returns true if the point is inside (inside/on boundary) the
// extent, and false otherwise
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::IsPointIn(const HGF3DCoord<DataType>& i_rPoint,
                                                                DataType                 i_Tolerance,
                                                                bool                     pi_ExcludeBoundary) const
    {
    HINVARIANTS;
    bool   ReturnValue = false;

    // Check if point lies inside (Borders are exclusive here)
    if (pi_ExcludeBoundary)
        ReturnValue =  (HNumeric<DataType>::GREATER(i_rPoint.GetX(), m_XMin, i_Tolerance) &&
                        HNumeric<DataType>::SMALLER(i_rPoint.GetX(), m_XMax, i_Tolerance) &&
                        HNumeric<DataType>::GREATER(i_rPoint.GetY(), m_YMin, i_Tolerance) &&
                        HNumeric<DataType>::SMALLER(i_rPoint.GetY(), m_YMax, i_Tolerance) &&
                        HNumeric<DataType>::GREATER(i_rPoint.GetZ(), m_ZMin, i_Tolerance) &&
                        HNumeric<DataType>::SMALLER(i_rPoint.GetZ(), m_ZMax, i_Tolerance));
    else
        // Check if point lies inside (Borders are inclusive here)
        ReturnValue =  (HNumeric<DataType>::GREATER_OR_EQUAL(i_rPoint.GetX(), m_XMin, i_Tolerance) &&
                        HNumeric<DataType>::SMALLER_OR_EQUAL(i_rPoint.GetX(), m_XMax, i_Tolerance) &&
                        HNumeric<DataType>::GREATER_OR_EQUAL(i_rPoint.GetY(), m_YMin, i_Tolerance) &&
                        HNumeric<DataType>::SMALLER_OR_EQUAL(i_rPoint.GetY(), m_YMax, i_Tolerance) &&
                        HNumeric<DataType>::GREATER_OR_EQUAL(i_rPoint.GetZ(), m_ZMin, i_Tolerance) &&
                        HNumeric<DataType>::SMALLER_OR_EQUAL(i_rPoint.GetZ(), m_ZMax, i_Tolerance));


    return(ReturnValue);
    }

//-----------------------------------------------------------------------------
// IsPointIn2D
// Indicates if a point is inside the extent. If properly indicates, then the
// boundary may be included inside the extent, and this boundary may be given a size
// by specification of an optional tolerance. The default tolerance is 0 and
// the boundary is considered not inside the extent.
//
// The method returns true if the point is inside (inside/on boundary) the
// extent, and false otherwise
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::IsPointIn2D(const HGF3DCoord<DataType>& i_rPoint,
                                                                  DataType                 i_Tolerance,
                                                                  bool                     pi_ExcludeBoundary) const
    {
    HINVARIANTS;
    bool   ReturnValue = false;

    // Check if point lies inside (Borders are exclusive here)
    if (pi_ExcludeBoundary)
        ReturnValue =  (HNumeric<DataType>::GREATER(i_rPoint.GetX(), m_XMin, i_Tolerance) &&
                        HNumeric<DataType>::SMALLER(i_rPoint.GetX(), m_XMax, i_Tolerance) &&
                        HNumeric<DataType>::GREATER(i_rPoint.GetY(), m_YMin, i_Tolerance) &&
                        HNumeric<DataType>::SMALLER(i_rPoint.GetY(), m_YMax, i_Tolerance));
    else
        // Check if point lies inside (Borders are inclusive here)
        ReturnValue =  (HNumeric<DataType>::GREATER_OR_EQUAL(i_rPoint.GetX(), m_XMin, i_Tolerance) &&
                        HNumeric<DataType>::SMALLER_OR_EQUAL(i_rPoint.GetX(), m_XMax, i_Tolerance) &&
                        HNumeric<DataType>::GREATER_OR_EQUAL(i_rPoint.GetY(), m_YMin, i_Tolerance) &&
                        HNumeric<DataType>::SMALLER_OR_EQUAL(i_rPoint.GetY(), m_YMax, i_Tolerance));

    return(ReturnValue);
    }



//-----------------------------------------------------------------------------
// GetXMin
// Returns the minimum X dimension boundary value of the extent.
//-----------------------------------------------------------------------------
template <class DataType> DataType HGF3DExtent<DataType>::GetXMin() const
    {
    HINVARIANTS;
    return(m_XMin);
    }

//-----------------------------------------------------------------------------
// GetYMin
// Returns the minimum Y dimension boundary value of the extent.
//-----------------------------------------------------------------------------
template <class DataType> DataType HGF3DExtent<DataType>::GetYMin() const
    {
    HINVARIANTS;
    return(m_YMin);
    }

//-----------------------------------------------------------------------------
// GetZMin
// Returns the minimum Z dimension boundary value of the extent.
//-----------------------------------------------------------------------------
template <class DataType> DataType HGF3DExtent<DataType>::GetZMin() const
    {
    HINVARIANTS;
    return(m_ZMin);
    }

//-----------------------------------------------------------------------------
// GetXMax
// Returns the maximum X dimension boundary value of the extent.
//-----------------------------------------------------------------------------
template <class DataType> DataType HGF3DExtent<DataType>::GetXMax() const
    {
    HINVARIANTS;
    return(m_XMax);
    }

//-----------------------------------------------------------------------------
// GetYMax
// Returns the maximum Y dimension boundary value of the extent.
//-----------------------------------------------------------------------------
template <class DataType> DataType HGF3DExtent<DataType>::GetYMax() const
    {
    HINVARIANTS;
    return(m_YMax);
    }

//-----------------------------------------------------------------------------
// GetZMax
// Returns the maximum Z dimension boundary value of the extent.
//-----------------------------------------------------------------------------
template <class DataType> DataType HGF3DExtent<DataType>::GetZMax() const
    {
    HINVARIANTS;
    return(m_ZMax);
    }


//-----------------------------------------------------------------------------
// SetXMin
// Sets the minimum X dimension boundary value of the extent.
// the value provided must be smaller than or equal to the current
// maximum X boundary value.
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::SetXMin(DataType i_XMin)
    {
    HINVARIANTS;

    // The given XMin must be smaller than or equal to XMax
    HPRECONDITION(i_XMin <= m_XMax);

    m_XMin = i_XMin;
    }

//-----------------------------------------------------------------------------
// SetYMin
// Sets the minimum Y dimension boundary value of the extent.
// the value provided must be smaller than or equal to the current
// maximum Y boundary value.
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::SetYMin(DataType i_YMin)
    {
    HINVARIANTS;

    // The given YMin must be smaller than or equal to YMax
    HPRECONDITION(i_YMin <= m_YMax);

    m_YMin = i_YMin;
    }

//-----------------------------------------------------------------------------
// SetZMin
// Sets the minimum Z dimension boundary value of the extent.
// the value provided must be smaller than or equal to the current
// maximum Z boundary value.
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::SetZMin(DataType i_ZMin)
    {
    HINVARIANTS;

    // The given ZMin must be smaller than or equal to ZMax
    HPRECONDITION(i_ZMin <= m_ZMax);

    m_ZMin = i_ZMin;
    }

//-----------------------------------------------------------------------------
// SetXMax
// Sets the maximum X dimension boundary value of the extent.
// the value provided must be greater than or equal to the current
// minimum X boundary value.
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::SetXMax(DataType i_XMax)
    {
    HINVARIANTS;

    // The given XMax must be greater than or equal to XMin
    HPRECONDITION(i_XMax >= m_XMin);

    m_XMax = i_XMax;
    }

//-----------------------------------------------------------------------------
// SetYMax
// Sets the maximum Y dimension boundary value of the extent.
// the value provided must be greater than or equal to the current
// minimum Y boundary value.
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::SetYMax(DataType i_YMax)
    {
    HINVARIANTS;

    // The given YMax must be greater than or equal to YMin
    HPRECONDITION(i_YMax >= m_YMin);

    m_YMax = i_YMax;
    }

//-----------------------------------------------------------------------------
// SetZMax
// Sets the maximum Z dimension boundary value of the extent.
// the value provided must be greater than or equal to the current
// minimum Z boundary value.
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::SetZMax(DataType i_ZMax)
    {
    HINVARIANTS;

    // The given ZMax must be greater than or equal to ZMin
    HPRECONDITION(i_ZMax >= m_ZMin);

    m_ZMax = i_ZMax;
    }


//-----------------------------------------------------------------------------
// GetOrigin
// Returns the origin of the extent. The origin is the point having the smallest
// values for all 3 dimensions.
//-----------------------------------------------------------------------------
template <class DataType> HGF3DCoord<DataType> HGF3DExtent<DataType>::GetOrigin() const
    {
    HINVARIANTS;

    return(HGF3DCoord<DataType>(m_XMin, m_YMin, m_ZMin));
    }

//-----------------------------------------------------------------------------
// GetCorner
// Returns the corner of the extent. The corner is the point having the greatest
// values for all 3 dimensions.
//-----------------------------------------------------------------------------
template <class DataType> HGF3DCoord<DataType> HGF3DExtent<DataType>::GetCorner() const
    {
    HINVARIANTS;
    return(HGF3DCoord<DataType>(m_XMax, m_YMax, m_ZMax));
    }

//-----------------------------------------------------------------------------
// SetOrigin
// Sets the origin of the extent. The origin is the point having the smallest
// values for all 3 dimensions. The given origin must therefore have all values
// for any dimnesion smallest than the current corner.
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::SetOrigin(const HGF3DCoord<DataType>& i_rNewOrigin)
    {
    HINVARIANTS;

    // The given point must be smaller than or equal to corner
    HPRECONDITION(i_rNewOrigin.GetX() <= m_XMax);
    HPRECONDITION(i_rNewOrigin.GetY() <= m_YMax);
    HPRECONDITION(i_rNewOrigin.GetZ() <= m_ZMax);

    m_XMin = i_rNewOrigin.GetX();
    m_YMin = i_rNewOrigin.GetY();
    m_ZMin = i_rNewOrigin.GetZ();
    }

//-----------------------------------------------------------------------------
// SetCorner
// Sets the corner of the extent. The corner is the point having the greatest
// values for all 3 dimensions. The given corner must therefore have all values
// for any dimnesion greatest than the current origin.
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::SetCorner(const HGF3DCoord<DataType>& i_rNewCorner)
    {
    HINVARIANTS;

    // The given point must be smaller than or equal to corner
    HPRECONDITION(i_rNewCorner.GetX() >= m_XMin);
    HPRECONDITION(i_rNewCorner.GetY() >= m_YMin);
    HPRECONDITION(i_rNewCorner.GetZ() >= m_ZMin);

    m_XMax = i_rNewCorner.GetX();
    m_YMax = i_rNewCorner.GetY();
    m_ZMax = i_rNewCorner.GetZ();
    }

//-----------------------------------------------------------------------------
// Set
// Sets the extent
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::Set(DataType pi_X1,
                                                          DataType pi_Y1,
                                                          DataType pi_Z1,
                                                          DataType pi_X2,
                                                          DataType pi_Y2,
                                                          DataType pi_Z2)
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

    // Set Z values
    if (pi_Z1 < pi_Z2)
        {
        m_ZMin = pi_Z1;
        m_ZMax = pi_Z2;
        }
    else
        {
        m_ZMin = pi_Z2;
        m_ZMax = pi_Z1;
        }


    }



//-----------------------------------------------------------------------------
// GetWidth
// Returns the width (size in the X dimension) of the extent.
//-----------------------------------------------------------------------------
template <class DataType> DataType HGF3DExtent<DataType>::GetWidth() const
    {
    HINVARIANTS;

    return(m_XMax - m_XMin);
    }

//-----------------------------------------------------------------------------
// GetHeight
// Returns the height (size in the Y dimension) of the extent.
//-----------------------------------------------------------------------------
template <class DataType> DataType HGF3DExtent<DataType>::GetHeight() const
    {
    HINVARIANTS;

    return(m_YMax - m_YMin);
    }

//-----------------------------------------------------------------------------
// GetThickness
// Returns the thickness (size in the Z dimension) of the extent.
//-----------------------------------------------------------------------------
template <class DataType> DataType HGF3DExtent<DataType>::GetThickness() const
    {
    HINVARIANTS;

    return(m_ZMax - m_ZMin);
    }

//-----------------------------------------------------------------------------
// CalculateVolume
// Returns the volume occupied by the extent box into space.
//-----------------------------------------------------------------------------
template <class DataType> DataType HGF3DExtent<DataType>::CalculateVolume() const
    {
    HINVARIANTS;

    return((m_XMax - m_XMin) * (m_YMax - m_YMin) * (m_ZMax - m_ZMin));
    }


//-----------------------------------------------------------------------------
// Add
// Potentially increases extent boundary to include the provided point.
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::Add(const HGF3DCoord<DataType>& i_rPoint)
    {
    HINVARIANTS;

    m_XMin  = MIN(i_rPoint.GetX(), m_XMin);
    m_YMin  = MIN(i_rPoint.GetY(), m_YMin);
    m_ZMin  = MIN(i_rPoint.GetZ(), m_ZMin);
    m_XMax  = MAX(i_rPoint.GetX(), m_XMax);
    m_YMax  = MAX(i_rPoint.GetY(), m_YMax);
    m_ZMax  = MAX(i_rPoint.GetZ(), m_ZMax);
    }

//-----------------------------------------------------------------------------
// Add
// Potentially increases extent boundary to fully include the provided extent.
//-----------------------------------------------------------------------------
template <class DataType> void HGF3DExtent<DataType>::Add(const HGF3DExtent<DataType>& i_rExtent)
    {
    HINVARIANTS;

    m_XMin  = MIN(i_rExtent.m_XMin, m_XMin);
    m_YMin  = MIN(i_rExtent.m_YMin, m_YMin);
    m_ZMin  = MIN(i_rExtent.m_ZMin, m_ZMin);
    m_XMax  = MAX(i_rExtent.m_XMax, m_XMax);
    m_YMax  = MAX(i_rExtent.m_YMax, m_YMax);
    m_ZMax  = MAX(i_rExtent.m_ZMax, m_ZMax);
    }


//-----------------------------------------------------------------------------
// Overlaps
// Returns true if the two extents overlap
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::Overlaps(const HGF3DExtent<DataType>& i_rObject) const
    {
    HINVARIANTS;

    return((m_XMin < i_rObject.m_XMax) &&
           (m_XMax > i_rObject.m_XMin) &&
           (m_YMin < i_rObject.m_YMax) &&
           (m_YMax > i_rObject.m_YMin) &&
           (m_ZMin < i_rObject.m_ZMax) &&
           (m_ZMax > i_rObject.m_ZMin));

    }


//-----------------------------------------------------------------------------
// OuterOverlaps
// Returns true if the two extents overlap to within given tolerance.
// This implies that even if extent do not properly overlap, then
// are nevertheless contiguous within given tolerance and are thus
// considered to outer overlap.
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::OuterOverlaps(const HGF3DExtent<DataType>& i_rObject,
                                                                    DataType i_Tolerance) const
    {
    HINVARIANTS;

    return(HNumeric<DataType>::SMALLER_OR_EQUAL(m_XMin, i_rObject.m_XMax, i_Tolerance) &&
           HNumeric<DataType>::GREATER_OR_EQUAL(m_XMax, i_rObject.m_XMin, i_Tolerance) &&
           HNumeric<DataType>::SMALLER_OR_EQUAL(m_YMin, i_rObject.m_YMax, i_Tolerance) &&
           HNumeric<DataType>::GREATER_OR_EQUAL(m_YMax, i_rObject.m_YMin, i_Tolerance) &&
           HNumeric<DataType>::SMALLER_OR_EQUAL(m_ZMin, i_rObject.m_ZMax, i_Tolerance) &&
           HNumeric<DataType>::GREATER_OR_EQUAL(m_ZMax, i_rObject.m_ZMin, i_Tolerance));
    }


//-----------------------------------------------------------------------------
// InnerOverlaps
// Returns true if the two extents overlap by at least given tolerance.
// This implies that even if extent do overlap, they must
// also overlap by at least given tolerance in any one dimension.
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::InnerOverlaps(const HGF3DExtent& i_rObject,
                                                                    DataType i_Tolerance) const
    {
    HINVARIANTS;
    return(HNumeric<DataType>::SMALLER(m_XMin, i_rObject.m_XMax, i_Tolerance) &&
           HNumeric<DataType>::GREATER(m_XMax, i_rObject.m_XMin, i_Tolerance) &&
           HNumeric<DataType>::SMALLER(m_YMin, i_rObject.m_YMax, i_Tolerance) &&
           HNumeric<DataType>::GREATER(m_YMax, i_rObject.m_YMin, i_Tolerance) &&
           HNumeric<DataType>::SMALLER(m_ZMin, i_rObject.m_ZMax, i_Tolerance) &&
           HNumeric<DataType>::GREATER(m_ZMax, i_rObject.m_ZMin, i_Tolerance));
    }


//-----------------------------------------------------------------------------
// Contains
// Returns true if the self extent fully contains the given extent.
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::Contains(const HGF3DExtent<DataType>& i_rObject) const
    {
    HINVARIANTS;

    return((m_XMin < i_rObject.m_XMin) &&
           (m_XMax > i_rObject.m_XMax) &&
           (m_YMin < i_rObject.m_YMin) &&
           (m_YMax > i_rObject.m_YMax) &&
           (m_ZMin < i_rObject.m_ZMin) &&
           (m_ZMax > i_rObject.m_ZMax));
    }


//-----------------------------------------------------------------------------
// InnerContains
// Returns true if the self extent fully contains the given extent and
// the given extent is located at least a tolerance distance from self
// boundaries for all dimensions..
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::InnerContains(const HGF3DExtent<DataType>& i_rObject,
                                                                    DataType i_Tolerance) const
    {
    HINVARIANTS;
    return(HNumeric<DataType>::SMALLER(m_XMin, i_rObject.m_XMin, i_Tolerance) &&
           HNumeric<DataType>::GREATER(m_XMax, i_rObject.m_XMax, i_Tolerance) &&
           HNumeric<DataType>::SMALLER(m_YMin, i_rObject.m_YMin, i_Tolerance) &&
           HNumeric<DataType>::GREATER(m_YMax, i_rObject.m_YMax, i_Tolerance) &&
           HNumeric<DataType>::SMALLER(m_ZMin, i_rObject.m_ZMin, i_Tolerance) &&
           HNumeric<DataType>::GREATER(m_ZMax, i_rObject.m_ZMax, i_Tolerance));
    }


//-----------------------------------------------------------------------------
// OuterContains
// Returns true if the self extent fully contains the given extent or this
// given extent is located within a tolerance distance from self
// boundaries. This implies that extent boundaries may be one upon
// the other within given tolerance. It is thus possible for the boundary
// of given extent to be so slightly outside self.
//-----------------------------------------------------------------------------
template <class DataType> bool HGF3DExtent<DataType>::OuterContains(const HGF3DExtent<DataType>& i_rObject,
                                                                    DataType i_Tolerance) const
    {
    HINVARIANTS;
    return(HNumeric<DataType>::SMALLER_OR_EQUAL(m_XMin, i_rObject.m_XMin, i_Tolerance) &&
           HNumeric<DataType>::GREATER_OR_EQUAL(m_XMax, i_rObject.m_XMax, i_Tolerance) &&
           HNumeric<DataType>::SMALLER_OR_EQUAL(m_YMin, i_rObject.m_YMin, i_Tolerance) &&
           HNumeric<DataType>::GREATER_OR_EQUAL(m_YMax, i_rObject.m_YMax, i_Tolerance) &&
           HNumeric<DataType>::SMALLER_OR_EQUAL(m_ZMin, i_rObject.m_ZMin, i_Tolerance) &&
           HNumeric<DataType>::GREATER_OR_EQUAL(m_ZMax, i_rObject.m_ZMax, i_Tolerance));
    }



END_IMAGEPP_NAMESPACE