//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DExtent.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <Imagepp/h/HNumeric.h>

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Constructor

    The extent coordinates are undefined after such construction.

    @param pi_rpCoordSys IN A pointer to the interpretation coordinate system.
    -----------------------------------------------------------------------------
*/
inline HGF2DExtent::HGF2DExtent(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : m_XDefined(false),
      m_YDefined(false),
      m_XMin(0.0),
      m_XMax(0.0),
      m_YMin(0.0),
      m_YMax(0.0),
      m_pCoordSys(pi_rpCoordSys)
    {
    }


/** -----------------------------------------------------------------------------
    Constructor with raw coordinates

    Creates an extent providing all values and interpretation coordinate system.

    At all times, the XMin coordinate of the extent must be smaller or equal to XMax,
    and at all times YMin must be smaller or equal to YMax.

    @param pi_XMin, pi_YMin IN Xmin, Ymin coordinates of extent, expressed in the
                               coordinate system specified by pi_rpCoordSys.

    @param pi_XMax, pi_YMax IN Xmax, Ymax coordinates of extent, expressed in the
                               coordinate system specified by pi_rpCoordSys.

    @param pi_rpCoordSys IN A constant reference to smart pointer to a coordinate
                            system object expressing the coordinate system that
                            will be used by this extent.

    -----------------------------------------------------------------------------
*/
inline HGF2DExtent::HGF2DExtent(double pi_XMin,
                                double pi_YMin,
                                double pi_XMax,
                                double pi_YMax,
                                const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : m_XDefined(true),
      m_YDefined(true),
      m_XMin(pi_XMin),
      m_XMax(pi_XMax),
      m_YMin(pi_YMin),
      m_YMax(pi_YMax),
      m_pCoordSys(pi_rpCoordSys)
    {

    HPRECONDITION (pi_XMin <=  pi_XMax);
    HPRECONDITION (pi_YMin <=  pi_YMax);
    }


//-----------------------------------------------------------------------------
// The copy constructor.
//-----------------------------------------------------------------------------
inline HGF2DExtent::HGF2DExtent(const HGF2DExtent& pi_rObj)
    : m_XMin (pi_rObj.m_XMin),
      m_XMax (pi_rObj.m_XMax),
      m_YMin (pi_rObj.m_YMin),
      m_YMax (pi_rObj.m_YMax),
      m_XDefined(pi_rObj.m_XDefined),
      m_YDefined(pi_rObj.m_YDefined),
      m_pCoordSys(pi_rObj.m_pCoordSys)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DExtent::~HGF2DExtent()
    {
    }


/** -----------------------------------------------------------------------------
    Inequality evaluation operator.  Two extent objects are considered "equal" if
    they map to the same physical position in either of the extent coordinate
    system.  If they do not use the same HGF2DCoordSys object, internal conversion
    occurs in such a way that compared values are expressed in the same HGF2DCoordSys.

    @param pi_rObj IN Constant reference to the extent object to compare to.

    @return Boolean value reflecting equality or inequality between objects.

    @see operator==()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DExtent::operator!=(const HGF2DExtent& pi_rObj) const
    {
    return (!operator==(pi_rObj));
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
inline bool HGF2DExtent::IsDefined () const
    {
    return (m_XDefined && m_YDefined);
    }


/** -----------------------------------------------------------------------------
    Checks this location is located inside the extent.  If the extent is undefined,
    then false is automatically returned, If the point falls exactly on the extent
    boundary, then it is considered inside the extent by default. If the pi_ExcludeBoundary
    is provided, then if the point is on the extent boundary, the point will be considered
    IN if set to false and outside if set to true.

    @param pi_rPoint IN Point to check if it is in the extent.

    @param pi_ExcludeBoundary IN OPTIONAL Defaults to false. If set to true, then a point
                              located on the extent boundary within the tolerance
                              will not be considered in the extent.

    @return true if point is in extent, false otherwise.


    -----------------------------------------------------------------------------
*/
inline bool HGF2DExtent::IsPointIn (const HGF2DLocation& pi_rPoint,
                                     bool                pi_ExcludeBoundary) const
    {
    bool   ReturnValue = false;

    if (IsDefined())
        {
        // Take a copy of the point, and take it in our coordinate system
        HGF2DLocation TempPoint(pi_rPoint, m_pCoordSys);

        // Check if point lies inside (Borders are exclusive here)
        if (pi_ExcludeBoundary)
            ReturnValue =  ((TempPoint.GetX() > m_XMin) &&
                            (TempPoint.GetX() < m_XMax) &&
                            (TempPoint.GetY() > m_YMin) &&
                            (TempPoint.GetY() < m_YMax));
        else
            // Check if point lies inside (Borders are inclusive here)
            ReturnValue =  ((TempPoint.GetX() >= m_XMin) &&
                            (TempPoint.GetX() <= m_XMax) &&
                            (TempPoint.GetY() >= m_YMin) &&
                            (TempPoint.GetY() <= m_YMax));
        }

    return(ReturnValue);
    }

//-----------------------------------------------------------------------------
// IsPointInnerIn
// Indicates if a point is inside the extent
//-----------------------------------------------------------------------------
inline bool HGF2DExtent::IsPointInnerIn (const HGF2DLocation& pi_rPoint) const
{
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
inline bool HGF2DExtent::IsPointOutterIn (const HGF2DLocation& pi_rPoint) const
{
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
inline bool HGF2DExtent::IsPointInnerIn(const HGF2DLocation& pi_rPoint, 
                                             double pi_Tolerance) const
{
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
inline bool HGF2DExtent::IsPointOutterIn(const HGF2DLocation& pi_rPoint, 
                                              double pi_Tolerance) const
{
    // Tolerance provided must be positive or null
    HPRECONDITION(pi_Tolerance >= 0.0);

    // Check if point lies inside (Borders are inclusive here)
    return (HNumeric<double>::GREATER_OR_EQUAL(pi_rPoint.GetX(), m_XMin, pi_Tolerance) &&
            HNumeric<double>::SMALLER_OR_EQUAL(pi_rPoint.GetX(), m_XMax, pi_Tolerance) &&
            HNumeric<double>::GREATER_OR_EQUAL(pi_rPoint.GetY(), m_YMin, pi_Tolerance) &&
            HNumeric<double>::SMALLER_OR_EQUAL(pi_rPoint.GetY(), m_YMax, pi_Tolerance));
}

/** -----------------------------------------------------------------------------
    Returns the Xmin numeric value of the coordinate of this extent.
    To use this method, the extent must be defined (IsDefined()).

    @return A distance of the requested value expressed in the units of the
            current coordinate system of the extent
    -----------------------------------------------------------------------------
*/
inline double HGF2DExtent::GetXMin() const
    {
    HPRECONDITION(IsDefined());

    return m_XMin;
    }


/** -----------------------------------------------------------------------------
    Returns the Ymin numeric value of the coordinate of this extent.
    To use this method, the extent must be defined (IsDefined()).

    @return A distance of the requested value expressed in the units of the
            current coordinate system of the extent
    -----------------------------------------------------------------------------
*/
inline double HGF2DExtent::GetYMin() const
    {
    HPRECONDITION(IsDefined());

    return m_YMin;
    }


/** -----------------------------------------------------------------------------
    Returns the Xmax numeric value of the coordinate of this extent.
    To use this method, the extent must be defined (IsDefined()).

    @return A distance of the requested value expressed in the units of the
            current coordinate system of the extent
    -----------------------------------------------------------------------------
*/
inline double HGF2DExtent::GetXMax() const
    {
    HPRECONDITION(IsDefined());

    return m_XMax;
    }


/** -----------------------------------------------------------------------------
    Returns the YMax numeric value of the coordinate of this extent.
    To use this method, the extent must be defined (IsDefined()).

    @return A distance of the requested value expressed in the units of the
            current coordinate system of the extent
    -----------------------------------------------------------------------------
*/
inline double HGF2DExtent::GetYMax() const
    {
    HPRECONDITION(IsDefined());

    return m_YMax;
    }

/** -----------------------------------------------------------------------------
    Returns the location of the origin which represents the point at
    coordinate (Xmin, Ymin). To use this method, the extent must be
    defined (IsDefined()).

    @return A location object that describes the physical position of the origin
            expressed in the same coordinate system as the Extent
    -----------------------------------------------------------------------------
*/
inline HGF2DLocation HGF2DExtent::GetOrigin() const
    {
    HPRECONDITION(IsDefined());

    return (HGF2DLocation (m_XMin, m_YMin, m_pCoordSys));
    }


/** -----------------------------------------------------------------------------
    Returns the location of the corner which represents the point at
    coordinate (Xmax, Ymax). To use this method, the extent must
    be defined (IsDefined()).

    @return A location object that describes the physical position of the corner
            expressed in the same coordinate system as the Extent.
    -----------------------------------------------------------------------------
*/
inline HGF2DLocation HGF2DExtent::GetCorner() const
    {
    HPRECONDITION(IsDefined());

    return (HGF2DLocation(m_XMax, m_YMax, m_pCoordSys));
    }

/** -----------------------------------------------------------------------------
Returns the location of the origin which represents the point at
coordinate (Xmin, Ymin). To use this method, the extent must be
defined (IsDefined()).

@return A location object that describes the physical position of the origin
expressed in the same coordinate system as the Extent
-----------------------------------------------------------------------------
*/
inline HGF2DLocation HGF2DExtent::GetLowerLeft() const
    {
    HPRECONDITION(IsDefined());

    return (HGF2DLocation(m_XMin, m_YMin, m_pCoordSys));
    }

/** -----------------------------------------------------------------------------
    Returns the location of the lower right corner which represents the point at
    coordinate (Xmax, Ymin). To use this method, the extent must be
    defined (IsDefined()).

    @return A location object that describes the physical position of the lower right corner
            expressed in the same coordinate system as the Extent
    -----------------------------------------------------------------------------
*/
inline HGF2DLocation HGF2DExtent::GetLowerRight() const
    {
    HPRECONDITION(IsDefined());

    return (HGF2DLocation (m_XMax, m_YMin, m_pCoordSys));
    }

/** -----------------------------------------------------------------------------
    Returns the location of the upper left corner which represents the point at
    coordinate (Xmin, Ymax). To use this method, the extent must be
    defined (IsDefined()).

    @return A location object that describes the physical position of the upper left corner
            expressed in the same coordinate system as the Extent
    -----------------------------------------------------------------------------
*/
inline HGF2DLocation HGF2DExtent::GetUpperLeft() const
    {
    HPRECONDITION(IsDefined());

    return (HGF2DLocation (m_XMin, m_YMax, m_pCoordSys));
    }

/** -----------------------------------------------------------------------------
Returns the location of the corner which represents the point at
coordinate (Xmax, Ymax). To use this method, the extent must
be defined (IsDefined()).

@return A location object that describes the physical position of the corner
expressed in the same coordinate system as the Extent.
-----------------------------------------------------------------------------
*/
inline HGF2DLocation HGF2DExtent::GetUpperRight() const
    {
    HPRECONDITION(IsDefined());

    return (HGF2DLocation(m_XMax, m_YMax, m_pCoordSys));
    }

/** -----------------------------------------------------------------------------
    Returns the width of the extent, which correspond to the absolute value of
    the difference between Xmax and Xmin. The units in which is expressed the
    distance is the same as the unit used by the X dimension of the coordinate
    system. To use these methods, the extent must be defined (IsDefined()).

    @return Width of extent.
    -----------------------------------------------------------------------------
*/
inline double HGF2DExtent::GetWidth () const
    {
    // If the extent is undefined we return 0.0
    return (IsDefined() ? fabs(m_XMax - m_XMin) : 0.0);
    }


/** -----------------------------------------------------------------------------
    Returns the height of the extent , which correspond to the absolute value
    of the difference between Ymax and Ymin. The unit in which is expressed
    the distance is the same in which is expressed the Y dimension of the
    coordinate system. To use this method, the extent must be defined (IsDefined()).

    @return Height of extent.
    -----------------------------------------------------------------------------
*/
inline double HGF2DExtent::GetHeight () const
    {
    // If the extent is undefined we return 0.0

    return(IsDefined() ? fabs(m_YMax - m_YMin) : 0.0);

    }


/** -----------------------------------------------------------------------------
    Returns the area of the extent. The unit in which is expressed the area
    is the same in which is expressed the square of the X distance units of
    the coordinate system. ). To use this method, the extent must be
    defined (IsDefined()).

    @return Area of the extent.

    -----------------------------------------------------------------------------
*/
inline double HGF2DExtent::CalculateArea () const
    {
    return (GetWidth() * GetHeight());
    }

/** -----------------------------------------------------------------------------
    Returns a pointer to the coordinate system object used by this extent to
    express all its values

    @return A constant reference to the smart pointer to the HGF2DCoordSys object
            used by this extent.
    -----------------------------------------------------------------------------
*/
inline const HFCPtr<HGF2DCoordSys>& HGF2DExtent::GetCoordSys () const
    {
    return (m_pCoordSys);
    }


/** -----------------------------------------------------------------------------
    Displacement operation. This operation of addition with an HGF2DDisplacement
    permits to apply a translation in its own coordinate system to the extent.
    The extent is modified.

    @param pi_rOffset IN Constant reference to an HGF2DDisplacement object to
                         expressing the displacement.

    @return Reference to self to be used as an l-value.

    @see operator-=()
    @see operator+()
    @see Move()
    -----------------------------------------------------------------------------
*/
inline HGF2DExtent& HGF2DExtent::operator+=(const HGF2DDisplacement& pi_rOffset)
    {
    HPRECONDITION(IsDefined());

    m_XMin += pi_rOffset.GetDeltaX();
    m_XMax += pi_rOffset.GetDeltaX();
    m_YMin += pi_rOffset.GetDeltaY();
    m_YMax += pi_rOffset.GetDeltaY();

    return (*this);
    }

/** -----------------------------------------------------------------------------
    Displacement operation. This operation of subtraction with an HGF2DDisplacement
    permits to apply a translation in its own coordinate system to the extent.
    The extent is modified.

    @param pi_rOffset IN Constant reference to an HGF2DDisplacement object to
                         expressing the displacement.

    @return Reference to self to be used as an l-value.

    @see operator+=()
    @see operator-()
    @see Move()
    -----------------------------------------------------------------------------
*/
inline HGF2DExtent& HGF2DExtent::operator-=(const HGF2DDisplacement& pi_rOffset)
    {
    HPRECONDITION(IsDefined());

    m_XMin -= pi_rOffset.GetDeltaX();
    m_XMax -= pi_rOffset.GetDeltaX();
    m_YMin -= pi_rOffset.GetDeltaY();
    m_YMax -= pi_rOffset.GetDeltaY();

    return (*this);
    }



/** -----------------------------------------------------------------------------
    Displacement operation. This operation of addition with an HGF2DDisplacement
    permits to apply a translation in its own coordinate system to the extent.
    The extent is modified.

    @param pi_rOffset IN Constant reference to an HGF2DDisplacement object to
                         expressing the displacement.

    @return Reference to self to be used as an l-value.

    @see operator+=()
    @see operator+()
    -----------------------------------------------------------------------------
*/
inline void HGF2DExtent::Move(const HGF2DDisplacement& pi_rOffset)
    {
    operator+=(pi_rOffset);
    }

/** -----------------------------------------------------------------------------
    This method sets the HGF2DCoordSys object used to describe the coordinate
    system used by this extent. Actual Xmin-Ymin and Xmax-Ymax values of this
    extent remain the same, but they will be expressed in the new HGF2DCoordSys.
    Reference to the HGF2DCoordSys object is kept as a smart pointer to it,
    allowing utilization of a single HGF2DCoordSys object among many extent,
    points and other objects using coordinates, without having to track
    which coordinate system is used or not.

    @param pi_rpCoordSys A constant reference to smart pointer to a new
                         HGF2DCoordSys object expressing the coordinate system
                         that will be used by this extent

    @see GetCoordSys()
    @see ChangeCoordSys()
    -----------------------------------------------------------------------------
*/
inline void HGF2DExtent::SetCoordSys (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    m_pCoordSys = pi_rpCoordSys;
    }


/** -----------------------------------------------------------------------------
    Union method.  The result is equivalent to the Add() method when the right
    operand is an extent. When performing union of two extents, the extent is
    modified to include only the smallest extent that encloses both extents,
    having different HGF2DCoordSys objects or not.

    If the HGF2DCoordSys object used by each operand is not the same, internal
    conversion occurs.
    If either extent is undefined and the other is not, then the result extent
    is equal to the defined one. If both extents are undefined, then the result
    extent will also be undefined.

    @param pi_rExtent Constant reference to extent object to perform union with.

    @see Intersect()
    @see Differentiate()
    -----------------------------------------------------------------------------
*/
inline void HGF2DExtent::Union(const HGF2DExtent& pi_rExtent)
    {
    Add(pi_rExtent);
    }

END_IMAGEPP_NAMESPACE