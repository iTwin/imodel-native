//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DOrientedRectangle.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DOrientedRectangle::HVE2DOrientedRectangle()
    : HVE2DSimpleShape()
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVE2DOrientedRectangle::HVE2DOrientedRectangle (const HVE2DRectangle& pi_rRectangle)
    : HVE2DSimpleShape(pi_rRectangle.GetCoordSys())
    {
    double  XMin;
    double  XMax;
    double  YMin;
    double  YMax;

    // Obtain rectangle description
    pi_rRectangle.GetRectangle(&XMin, &YMin, &XMax, &YMax);

    // Set points
    m_FirstPoint = HGF2DLocation(XMin, YMin, pi_rRectangle.GetCoordSys());
    m_SecondPoint = HGF2DLocation(XMin, YMax, pi_rRectangle.GetCoordSys());
    m_ThirdPoint = HGF2DLocation(XMax, YMax, pi_rRectangle.GetCoordSys());
    m_FourthPoint = HGF2DLocation(XMax, YMin, pi_rRectangle.GetCoordSys());
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVE2DOrientedRectangle::HVE2DOrientedRectangle(const HGF2DLocation& pi_rFirstPoint,
                                                      const HGF2DLocation& pi_rSecondPoint,
                                                      const HGF2DLocation& pi_rThirdPoint,
                                                      const HGF2DLocation& pi_rFourthPoint)
    : HVE2DSimpleShape(pi_rFirstPoint.GetCoordSys()),
      m_FirstPoint(pi_rFirstPoint),
      m_SecondPoint(pi_rSecondPoint, pi_rFirstPoint.GetCoordSys()),
      m_ThirdPoint(pi_rThirdPoint, pi_rFirstPoint.GetCoordSys()),
      m_FourthPoint(pi_rFourthPoint, pi_rFirstPoint.GetCoordSys())
    {
    // Create copies of second, third and fourth point in the first point coordinate system
    HDEBUGCODE(HGF2DLocation SecondPoint(pi_rSecondPoint, pi_rFirstPoint.GetCoordSys()));
    HDEBUGCODE(HGF2DLocation ThirdPoint(pi_rThirdPoint, pi_rFirstPoint.GetCoordSys()));
    HDEBUGCODE(HGF2DLocation FourthPoint(pi_rFourthPoint, pi_rFirstPoint.GetCoordSys()));

    // Verify that the given points are valid
    // The direction from first to second must be equal (+PI) to the bearing from third to fourth
    HPRECONDITION((pi_rFirstPoint - SecondPoint).CalculateBearing().IsEqualTo(
                      (ThirdPoint - FourthPoint).CalculateBearing() + PI)));
    // The direction from second to third must be equal (+PI) to the bearing from fourth to first
    HPRECONDITION((SecondPoint - ThirdPoint).CalculateBearing().IsEqualTo(
                      (FourthPoint - pi_rFirstPoint).CalculateBearing() + PI));
    // The angle variation between any consecutive segment must be PI/2 (or 3 * PI/2)
    HPRECONDITION(
         HDOUBLE_EQUAL_EPSILON((pi_rFirstPoint - SecondPoint).CalculateBearing() - (SecondPoint - ThirdPoint).CalculateBearing()), PI / 2) ||
         HDOUBLE_EQUAL_EPSILON((pi_rFirstPoint - SecondPoint).CalculateBearing() - (SecondPoint - ThirdPoint).CalculateBearing()), 3 * PI / 2)
         );
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVE2DOrientedRectangle::HVE2DOrientedRectangle(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys),
      m_FirstPoint(pi_rpCoordSys),
      m_SecondPoint(pi_rpCoordSys),
      m_ThirdPoint(pi_rpCoordSys),
      m_FourthPoint(pi_rpCoordSys)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DOrientedRectangle object.
//-----------------------------------------------------------------------------
inline HVE2DOrientedRectangle::HVE2DOrientedRectangle(const HVE2DOrientedRectangle& pi_rObj)
    : HVE2DSimpleShape(pi_rObj),
      m_FirstPoint(pi_rObj.m_FirstPoint),
      m_SecondPoint(pi_rObj.m_SecondPoint),
      m_ThirdPoint(pi_rObj.m_ThirdPoint),
      m_FourthPoint(pi_rObj.m_FourthPoint)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DOrientedRectangle::~HVE2DOrientedRectangle()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
inline HVE2DOrientedRectangle& HVE2DOrientedRectangle::operator=(const HVE2DOrientedRectangle& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Call ancester operator
        HVE2DSimpleShape::operator=(pi_rObj);

        // Copy attributes
        m_FirstPoint = pi_rObj.m_FirstPoint;
        m_SecondPoint = pi_rObj.m_SecondPoint;
        m_ThirdPoint = pi_rObj.m_ThirdPoint;
        m_FourthPoint = pi_rObj.m_FourthPoint;
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// IsPointOn
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
inline bool HVE2DOrientedRectangle::IsPointOn(const HGF2DLocation& pi_rPoint,
                                               HVE2DVector::ExtremityProcessing pi_ExtremityProcessing) const
    {
    return(IsPointOnSCS(HGF2DLocation(pi_rPoint,GetCoordSys())));
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
inline bool HVE2DOrientedRectangle::IsPointOnSCS(const HGF2DLocation& pi_rPoint,
                                                  HVE2DVector::ExtremityProcessing pi_ExtremityProcessing) const
    {
    // The point must be expressed in the same coordinate system as oriented rectangle
    HPRECONDITION(GetCoordSys() == pi_rPoint.GetCoordSys());

    return(HGF2DSegment(m_FirstPoint, m_SecondPoint).IsPointOnSCS(pi_rPoint) ||
           HGF2DSegment(m_SecondPoint, m_ThirdPoint).IsPointOnSCS(pi_rPoint) ||
           HGF2DSegment(m_ThirdPoint, m_FourthPoint).IsPointOnSCS(pi_rPoint) ||
           HGF2DSegment(m_FourthPoint, m_FirstPoint).IsPointOnSCS(pi_rPoint));
    }


//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DOrientedRectangle::Clone() const
    {
    return (new HVE2DOrientedRectangle(*this));
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the polygon.
//-----------------------------------------------------------------------------
inline HGF2DExtent HVE2DOrientedRectangle::GetExtent() const
    {
    // Create recipient rectangle
    HGF2DExtent TheExtent(GetCoordSys());

    // Check if rectangle is not empty
    if (!IsEmpty())
        {
        TheExtent.Add(m_FirstPoint);
        TheExtent.Add(m_SecondPoint);
        TheExtent.Add(m_ThirdPoint);
        TheExtent.Add(m_FourthPoint);
        }

    return(TheExtent);
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the rectangle by the specified displacement
//-----------------------------------------------------------------------------
inline void HVE2DOrientedRectangle::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    m_FirstPoint += pi_rDisplacement;
    m_SecondPoint += pi_rDisplacement;
    m_ThirdPoint += pi_rDisplacement;
    m_FourthPoint += pi_rDisplacement;
    }

#if (0)
//-----------------------------------------------------------------------------
// Scale
// This method scales the rectangle by the specified scaling factor
//-----------------------------------------------------------------------------
inline void HVE2DOrientedRectangle::Scale(double pi_ScaleFactor)
    {
    // The given scale factor must not be zero
    HPRECONDITION(pi_ScaleFactor > 0.0);

    // The origin (which is the minimum) is not changed

    // Change the maximum values
    m_XMax = m_XMin + ((m_XMax - m_XMin) * pi_ScaleFactor);
    m_YMax = m_YMin + ((m_YMax - m_YMin) * pi_ScaleFactor);

    HASSERT(m_XMax >= m_XMin);
    HASSERT(m_YMax >= m_YMin);
    }
#endif

//-----------------------------------------------------------------------------
// IsEmpty
// This method checks if the polygon is defined or not (represents an empty shape)
//-----------------------------------------------------------------------------
inline bool HVE2DOrientedRectangle::IsEmpty() const
    {
    return((m_FirstPoint.IsEqualTo(m_SecondPoint) || m_FirstPoint.IsEqualTo(m_FourthPoint));
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the shape
//-----------------------------------------------------------------------------
inline double HVE2DOrientedRectangle::CalculateArea() const
    {
    return ((m_FirstPoint - m_FourthPoint).CalculateLength() *
            (m_FirstPoint - m_SecondPoint).CalculateLength())
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
inline double HVE2DOrientedRectangle::CalculatePerimeter() const
    {
    return(2 * (m_FirstPoint - m_FourthPoint).CalculateLength() +
           2 * (m_FirstPoint - m_SecondPoint).CalculateLength());
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the polygon area
//-----------------------------------------------------------------------------
inline bool HVE2DOrientedRectangle::IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const
    {
    // Obtain a copy of point in the oriented rectangle coordinate system
    HGF2DLocation   MyTestPoint(pi_rPoint, GetCoordSys());

    // Obtain tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    HASSERT(0);
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the polygon
//-----------------------------------------------------------------------------
inline void HVE2DOrientedRectangle::MakeEmpty()
    {
    m_SecojndPoint  = m_FirstPoint;
    m_ThirdPoint = m_FirstPoint;
    m_FourthPoint = m_FirstPoint;
    }


//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HVE2DShapeTypeId HVE2DOrientedRectangle::GetShapeType() const
    {
    return(HVE2DOrientedRectangle::TYPE_ID);
    }


//-----------------------------------------------------------------------------
// UnifyShapeSCS
// This method create a new shape as the union between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DOrientedRectangle::UnifyShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    return(pi_rShape.GetShapeType() == HVE2DOrientedRectangle::TYPE_ID  ?
           UnifyRectangleSCS(*((HVE2DOrientedRectangle*)(&pi_rShape)))  :
           pi_rShape.UnifyShapeSCS(*this));
    }

//-----------------------------------------------------------------------------
// DifferentiateShapeSCS
// This method create a new shape as the difference between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DOrientedRectangle::DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    return(pi_rShape.GetShapeType() == HVE2DOrientedRectangle::TYPE_ID  ?
           DifferentiateRectangleSCS(*((HVE2DOrientedRectangle*)(&pi_rShape)))  :
           pi_rShape.DifferentiateFromShapeSCS(*this));

    }


//-----------------------------------------------------------------------------
// DifferentiateFromShapeSCS
// This method creates a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DOrientedRectangle::DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    return(pi_rShape.GetShapeType() == HVE2DOrientedRectangle::TYPE_ID  ?
           ((HVE2DOrientedRectangle*)(&pi_rShape))->DifferentiateRectangleSCS(*this)  :
           pi_rShape.DifferentiateShapeSCS(*this));

    }


//-----------------------------------------------------------------------------
// IntersectShapeSCS
// This method create a new shape as the intersection between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DOrientedRectangle::IntersectShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shape must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    return(pi_rShape.GetShapeType() == HVE2DOrientedRectangle::TYPE_ID  ?
           IntersectRectangleSCS(*((HVE2DOrientedRectangle*)(&pi_rShape)))  :
           pi_rShape.IntersectShapeSCS(*this));
    }


END_IMAGEPP_NAMESPACE
