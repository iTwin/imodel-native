//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DCircle.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "HVE2DPolygon.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DCircle::HVE2DCircle()
    : m_Center(0.0, 0.0, GetCoordSys()),
      m_Radius(0.0)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVE2DCircle::HVE2DCircle(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys),
      m_Center(pi_rpCoordSys)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor (center and radius)
//-----------------------------------------------------------------------------
inline HVE2DCircle::HVE2DCircle(const HGF2DLocation& pi_rCenter,
                                double               pi_rRadius)
    : HVE2DSimpleShape(pi_rCenter.GetCoordSys()),
      m_Center(pi_rCenter),
      m_Radius(pi_rRadius)
    {
    HPRECONDITION(pi_rRadius > 0.0);

    ResetTolerance();

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor (by three points)
//-----------------------------------------------------------------------------
inline HVE2DCircle::HVE2DCircle(const HGF2DLocation& pi_rFirstPoint,
                                const HGF2DLocation& pi_rSecondPoint,
                                const HGF2DLocation& pi_rThirdPoint)
    : HVE2DSimpleShape(pi_rFirstPoint.GetCoordSys()),
      m_Center(pi_rFirstPoint.GetCoordSys())
    {
    // The three points must be different
    HPRECONDITION(!pi_rFirstPoint.IsEqualTo(pi_rSecondPoint));
    HPRECONDITION(!pi_rFirstPoint.IsEqualTo(pi_rThirdPoint));
    HPRECONDITION(!pi_rSecondPoint.IsEqualTo(pi_rThirdPoint));

    // The three points must not be co-linear
    HPRECONDITION((pi_rFirstPoint - pi_rSecondPoint).CalculateBearing() !=
                  (pi_rFirstPoint - pi_rThirdPoint).CalculateBearing());

    SetByPerimeterPoints(pi_rFirstPoint, pi_rSecondPoint, pi_rThirdPoint);

    HINVARIANTS;

    }




//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DCircle object.
//-----------------------------------------------------------------------------
inline HVE2DCircle::HVE2DCircle(const HVE2DCircle& pi_rObj)
    : HVE2DSimpleShape(pi_rObj),
      m_Radius(pi_rObj.m_Radius),
      m_Center(pi_rObj.m_Center)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DCircle::~HVE2DCircle()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
inline HVE2DCircle& HVE2DCircle::operator=(const HVE2DCircle& pi_rObj)
    {

    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Invoque ancester copy
        HVE2DSimpleShape::operator=(pi_rObj);

        // Copy attributes
        m_Center = pi_rObj.m_Center;
        m_Radius = pi_rObj.m_Radius;
        }

    HINVARIANTS;

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// operator!=
// Equality compare operator.
//-----------------------------------------------------------------------------
inline bool HVE2DCircle::operator!=(const HVE2DCircle& pi_rObj) const
    {
    HINVARIANTS;
    HPRECONDITION(!IsEmpty());

    // They must either have the same coordinate system or be related
    // through a shape preserving relation.
    HPRECONDITION((GetCoordSys() == pi_rObj.GetCoordSys()) ||
                  (GetCoordSys()->HasShapePreservingRelationTo(pi_rObj.GetCoordSys())));

    return(!operator==(pi_rObj));
    }



//-----------------------------------------------------------------------------
// GetRadius
//-----------------------------------------------------------------------------
inline double HVE2DCircle::GetRadius() const
    {
    HINVARIANTS;
    return(m_Radius);
    }


//-----------------------------------------------------------------------------
// SetRadius
//-----------------------------------------------------------------------------
inline void HVE2DCircle::SetRadius(double pi_rRadius)
    {
    HINVARIANTS;
    HPRECONDITION(pi_rRadius > 0.0);

    m_Radius = pi_rRadius;

    ResetTolerance();
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// GetCenter
//-----------------------------------------------------------------------------
inline const HGF2DLocation& HVE2DCircle::GetCenter() const
    {
    HINVARIANTS;
    return(m_Center);
    }

//-----------------------------------------------------------------------------
// SetCenter
//-----------------------------------------------------------------------------
inline void HVE2DCircle::SetCenter(const HGF2DLocation& pi_rCenter)
    {
    HINVARIANTS;
    m_Center = pi_rCenter.ExpressedIn(GetCoordSys());

    ResetTolerance();
    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Set
//-----------------------------------------------------------------------------
inline void HVE2DCircle::Set(const HGF2DLocation& pi_rCenter,
                             double               pi_rRadius)
    {
    HINVARIANTS;
    // The center must be greater than 0.0
    HPRECONDITION(pi_rRadius > 0.0);

    // Set center
    m_Center = pi_rCenter.ExpressedIn(GetCoordSys());

    // Set radius
    m_Radius = pi_rRadius;

    ResetTolerance();
    HINVARIANTS;
    }




//-----------------------------------------------------------------------------
// CalculateShortestDistance
// This method returns the shortest distance from given point to circle
//-----------------------------------------------------------------------------
inline double HVE2DCircle::CalculateShortestDistance(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;
    HPRECONDITION(!IsEmpty());

    return(fabs((m_Center - pi_rPoint).CalculateLength() - m_Radius));
    }

//-----------------------------------------------------------------------------
// IsEmpty
// This method indicates if circle is empty
//-----------------------------------------------------------------------------
inline bool HVE2DCircle::IsEmpty() const
    {
    HINVARIANTS;
    return(m_Radius == 0.0);
    }

//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HVE2DShapeTypeId HVE2DCircle::GetShapeType() const
    {
    HINVARIANTS;
    return(HVE2DCircle::CLASS_ID);
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method returns the area value of circle
//-----------------------------------------------------------------------------
inline double HVE2DCircle::CalculateArea() const
    {
    HINVARIANTS;
    return(PI * m_Radius * m_Radius);
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method returns the perimeter of circle
//-----------------------------------------------------------------------------
inline double HVE2DCircle::CalculatePerimeter() const
    {
    HINVARIANTS;
    return(2 * PI * m_Radius);
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method indicates if given point is in circle
//-----------------------------------------------------------------------------
inline bool HVE2DCircle::IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const
    {
    HINVARIANTS;
    // Set tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    double TheDistance = (m_Center - pi_rPoint).CalculateLength();

    return((TheDistance < m_Radius) && (!HDOUBLE_EQUAL(TheDistance, m_Radius, Tolerance)));
    }

//-----------------------------------------------------------------------------
// IsPointOn
// This method indicates if given point is on circle
//-----------------------------------------------------------------------------
inline bool HVE2DCircle::IsPointOn(const HGF2DLocation& pi_rPoint,
                                    HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                    double                          pi_Tolerance) const

    {
    HINVARIANTS;
    return(
              (pi_Tolerance == HVE_USE_INTERNAL_EPSILON) ?
              (m_Center - pi_rPoint).CalculateLength().IsEqualTo(m_Radius, GetTolerance()) :
              (m_Center - pi_rPoint).CalculateLength().IsEqualTo(m_Radius, pi_Tolerance));
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// This method indicates if given point is on circle
//-----------------------------------------------------------------------------
inline bool HVE2DCircle::IsPointOnSCS(const HGF2DLocation& pi_rPoint,
                                       HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                       double                          pi_Tolerance) const
    {
    HINVARIANTS;
    // The point must be in the same coordinate system as circle
    HPRECONDITION(GetCoordSys() == pi_rPoint.GetCoordSys());

    return(
              (pi_Tolerance == HVE_USE_INTERNAL_EPSILON) ?
              (m_Center - pi_rPoint).CalculateLength().IsEqualTo(m_Radius, GetTolerance()) :
              (m_Center - pi_rPoint).CalculateLength().IsEqualTo(m_Radius, pi_Tolerance));
    }



//-----------------------------------------------------------------------------
// MakeEmpty
// This method makes the circle empty
//-----------------------------------------------------------------------------
inline void HVE2DCircle::MakeEmpty()
    {
    HINVARIANTS;
    m_Radius = 0.0;
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of circle
//-----------------------------------------------------------------------------
inline HGF2DExtent HVE2DCircle::GetExtent() const
    {
    HINVARIANTS;
    return(HGF2DExtent(m_Center.GetX() - m_Radius,
                       m_Center.GetY() - m_Radius,
                       m_Center.GetX() + m_Radius,
                       m_Center.GetY() + m_Radius,
                       GetCoordSys()));
    }


//-----------------------------------------------------------------------------
// Move
// This method moves the circle by specified displacement
//-----------------------------------------------------------------------------
inline void HVE2DCircle::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HINVARIANTS;
    m_Center += pi_rDisplacement;

    ResetTolerance();
    }

#if (0)
//-----------------------------------------------------------------------------
// GetOrigin
// This method returns the origin of circle.
//-----------------------------------------------------------------------------
inline HGF2DLocation HVE2DCircle::GetOrigin() const
    {
    HINVARIANTS;
    return(m_Center);
    }
#endif

//-----------------------------------------------------------------------------
// ChangeCoordSys
// This method changes the coordinate system of circle
//-----------------------------------------------------------------------------
inline void HVE2DCircle::ChangeCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    HINVARIANTS;
    // They must either have the same coordinate system or be related
    // through a shape preserving relation.
    HPRECONDITION((GetCoordSys() == pi_rpCoordSys) ||
                  (GetCoordSys()->HasShapePreservingRelationTo(pi_rpCoordSys)));

    // Calculate transformed permiter point (arbitrary point)
    HGF2DLocation   OtherPoint = m_Center +
                                 HGF2DDisplacement(HGFBearing(0.0),
                                                   m_Radius);

    // Change coordinate system of center
    m_Center.ChangeCoordSys(pi_rpCoordSys);

    // Change radius
    m_Radius = (m_Center - OtherPoint).CalculateLength();

    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the circle is adjacent with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DCircle::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    HINVARIANTS;
    return(GetLinear().AreAdjacent(pi_rVector));
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the circle is contiguous with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DCircle::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    HINVARIANTS;
    return(GetLinear().AreContiguous(pi_rVector));
    }

//-----------------------------------------------------------------------------
// Clone
// This method returns a clone of circle
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DCircle::Clone() const
    {
    HINVARIANTS;
    return(new HVE2DCircle(*this));
    }

//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
inline HGF2DLocation HVE2DCircle::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // The closest point is the point located in the direction of point from center
    // but at a simple radius of distance
    return(m_Center + HGF2DDisplacement((pi_rPoint - m_Center).CalculateBearing(), m_Radius));
    }

//-----------------------------------------------------------------------------
// Scale
// This method scales the circle by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
inline void HVE2DCircle::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    HINVARIANTS;
    // The given scale factor must not be zero
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // Change values
    m_Center += ((m_Center - pi_rScaleOrigin) * pi_ScaleFactor);

    m_Radius *= pi_ScaleFactor;

    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// Crosses
// This method checks if the polygon crosses with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DCircle::Crosses(const HVE2DVector& pi_rVector) const
    {
    HINVARIANTS;

    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // Since crossing computations are so complicated, we tel HVE2DPolygonOfSegments to perform
    // the operation
    return (HVE2DPolygon(GetLinear()).Crosses(pi_rVector));
    }

//-----------------------------------------------------------------------------
// Intersect
// This method checks if the intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
inline size_t HVE2DCircle::Intersect(const HVE2DVector& pi_rVector,
                                     HGF2DLocationCollection* po_pCrossPoints) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // Since intersect computations are so complicated, we tel HVE2DPolygonOfSegments to perform
    // the operation
    return (HVE2DPolygon(GetLinear()).Intersect(pi_rVector, po_pCrossPoints));
    }

//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the rectangle is contiguous with given vector
// at specified point.
//-----------------------------------------------------------------------------
inline bool HVE2DCircle::AreContiguousAt(const HVE2DVector& pi_rVector,
                                          const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    return(GetLinear().AreContiguousAt(pi_rVector, pi_rPoint));
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
inline void HVE2DCircle::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                      const HGF2DLocation& pi_rPoint,
                                                      HGF2DLocation* po_pFirstContiguousnessPoint,
                                                      HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    // Create a polygon and process by it
    HVE2DPolygon(GetLinear()).ObtainContiguousnessPointsAt(pi_rVector,
                                                           pi_rPoint,
                                                           po_pFirstContiguousnessPoint,
                                                           po_pSecondContiguousnessPoint);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
inline size_t HVE2DCircle::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                      HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    HPRECONDITION(AreContiguous(pi_rVector));

    return (HVE2DPolygon(GetLinear()).ObtainContiguousnessPoints(pi_rVector, po_pContiguousnessPoints));
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of rectangle at specified point
//-----------------------------------------------------------------------------
inline double HVE2DCircle::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                                                        HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The acceleration on a circle is a constant
    return (pi_Direction == HVE2DVector::ALPHA ? 1.0 / m_Radius : -1.0 / m_Radius);
    }


//-----------------------------------------------------------------------------
// CalculateBearing
// Calculates and returns the bearing of rectangle at specified point
//-----------------------------------------------------------------------------
inline HGFBearing HVE2DCircle::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                                HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    return((m_Center - pi_rPoint).CalculateBearing() +
           (pi_Direction == HVE2DVector::ALPHA ? PI/2 : -PI/2));
    }


END_IMAGEPP_NAMESPACE
