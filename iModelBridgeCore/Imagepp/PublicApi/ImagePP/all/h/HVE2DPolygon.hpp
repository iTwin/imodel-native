//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DPolygon.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DPolygon::HVE2DPolygon()
    : HVE2DSimpleShape()
    {
    }

//-----------------------------------------------------------------------------
// Constructor with setting of complex linear
//-----------------------------------------------------------------------------
inline HVE2DPolygon::HVE2DPolygon(const HVE2DComplexLinear& pi_rComplex)
    : HVE2DSimpleShape(pi_rComplex.GetCoordSys()),
      m_ComplexLinear(pi_rComplex)
    {
    // The shape forming a polygon may not cross its own path
//    HPRECONDITION(!pi_rComplex.AutoCrosses());

    // A linear forming a polygon must close on itself
    HPRECONDITION(pi_rComplex.GetStartPoint().IsEqualTo(pi_rComplex.GetEndPoint(), GetTolerance()));

    // Extract tolerance settings
    SetAutoToleranceActive(pi_rComplex.IsAutoToleranceActive());
    SetTolerance(pi_rComplex.GetTolerance());
    }

//-----------------------------------------------------------------------------
// Constructor with a coordinate system only
//-----------------------------------------------------------------------------
inline HVE2DPolygon::HVE2DPolygon(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys),
      m_ComplexLinear(pi_rpCoordSys)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DPolygon object.
//-----------------------------------------------------------------------------
inline HVE2DPolygon::HVE2DPolygon(const HVE2DPolygon& pi_rObj)
    : HVE2DSimpleShape(pi_rObj),
      m_ComplexLinear(pi_rObj.m_ComplexLinear)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DPolygon::~HVE2DPolygon()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
inline HVE2DPolygon& HVE2DPolygon::operator=(const HVE2DPolygon& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Empty the polygon
        MakeEmpty();

        // Invoque the ancester operator
        HVE2DSimpleShape::operator=(pi_rObj);

        // Set linear to given shape linear
        SetLinear(pi_rObj.m_ComplexLinear);
        }

    // Return reference to self
    return (*this);
    }

//-----------------------------------------------------------------------------
// GetLinear
// This method returns the complex linear.of the shape
//-----------------------------------------------------------------------------
inline HVE2DComplexLinear HVE2DPolygon::GetLinear() const
    {
    return (m_ComplexLinear);
    }

/** -----------------------------------------------------------------------------
    These methods sets the spatial definition of the polygon.
    The given linear must not auto-intersect, must auto-close.

    @param pi_rLinear    Constant reference to the HVE2DLinear to set
                        polygon to.

    Example
    @code
    @end

    @see GetLinear()
    -----------------------------------------------------------------------------
*/
inline void HVE2DPolygon::SetLinear(const HVE2DLinear& pi_rLinear)
    {
    // The shape forming a polygon may not cross its own path
//    HPRECONDITION(!pi_rLinear.AutoCrosses());

    // A linear forming a polygon must close on itself
    HPRECONDITION(pi_rLinear.GetStartPoint() == pi_rLinear.GetEndPoint());

    // We empty the complex linear
    m_ComplexLinear.MakeEmpty();

    // We check if the given linear is complex
    if (pi_rLinear.IsComplex())
        {
        // Since it is a complex ...
        m_ComplexLinear.AppendComplexLinear((*((HVE2DComplexLinear*)(&pi_rLinear))));
        }
    else
        {
        // Since it is not a complex we add it
        m_ComplexLinear.AppendLinear(pi_rLinear);
        }

    // Depending if auto tolerance is active ...
    if (IsAutoToleranceActive())
        {
        // The component complex must also be auto tolerance active
        HASSERT(m_ComplexLinear.IsAutoToleranceActive());

        // Copy tolertance from component complex ... which calculated the value already
        SetTolerance(m_ComplexLinear.GetTolerance());
        }

    }


//-----------------------------------------------------------------------------
// AllocateLinear
// This method returns the complex linear of the shape in the specified
// rotation direction
//-----------------------------------------------------------------------------
inline HVE2DComplexLinear* HVE2DPolygon::AllocateLinear(HVE2DSimpleShape::RotationDirection pi_RotationDirection) const
    {
    HVE2DComplexLinear*  pReturnedLinear = new HVE2DComplexLinear(m_ComplexLinear);

    if (CalculateRotationDirection() != pi_RotationDirection)
        // Reverse the copy
        pReturnedLinear->Reverse();

    return(pReturnedLinear);
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns the complex linear of the shape in the specified
// rotation direction
//-----------------------------------------------------------------------------
inline HVE2DComplexLinear HVE2DPolygon::GetLinear(HVE2DSimpleShape::RotationDirection pi_RotationDirection) const
    {
    // THERE ARE TWO RETURNS IN THIS METHOD FOR PERFORMANCE REASON
    if (CalculateRotationDirection() != pi_RotationDirection)
        {
        // The linear does not rotate in the proper direction
        // Make a copy of it
        HVE2DComplexLinear  NewComplex(m_ComplexLinear);

        // Reverse the copy
        NewComplex.Reverse();

        return (NewComplex);
        }
    else
        {
        return (m_ComplexLinear);
        }
    }


//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
inline HGF2DLocation HVE2DPolygon::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    return (m_ComplexLinear.CalculateClosestPoint(pi_rPoint));
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the polygon is adjacent with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DPolygon::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    return (m_ComplexLinear.AreAdjacent(pi_rVector));
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the polygon is contiguous with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DPolygon::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    return (m_ComplexLinear.AreContiguous(pi_rVector));
    }


//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the polygon is contiguous with given vector
// at specified point
//-----------------------------------------------------------------------------
inline bool HVE2DPolygon::AreContiguousAt(const HVE2DVector& pi_rVector,
                                           const HGF2DLocation& pi_rPoint) const
    {
    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    return (m_ComplexLinear.AreContiguousAt(pi_rVector, pi_rPoint));
    }


#if (0)
//-----------------------------------------------------------------------------
// Flirts
// This method checks if the polygon flirts with given vector.
// INCOMPLETE ???
//-----------------------------------------------------------------------------
inline bool HVE2DPolygon::Flirts(const HVE2DVector& pi_rVector) const
    {
    // ???? What happens if crosses at start point ?
    return (m_ComplexLinear.Flirts(pi_rVector) || (m_ComplexLinear.ConnectsTo(pi_rVector)));
    }
#endif


//-----------------------------------------------------------------------------
// IsPointOn
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
inline bool HVE2DPolygon::IsPointOn(const HGF2DLocation& pi_rPoint,
                                     HVE2DVector::ExtremityProcessing pi_ExtremitityProcessing,
                                     double pi_Tolerance) const
    {
    // Note that the pi_Extremity processing value is ignored since
    // a polygon has no extremities
    return (m_ComplexLinear.IsPointOn(pi_rPoint, HVE2DVector::INCLUDE_EXTREMITIES, pi_Tolerance));
    }


//-----------------------------------------------------------------------------
// IsPointOnSCS
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
inline bool HVE2DPolygon::IsPointOnSCS(const HGF2DLocation& pi_rPoint,
                                        HVE2DVector::ExtremityProcessing pi_ExtremitityProcessing,
                                        double pi_Tolerance) const
    {
    HPRECONDITION(GetCoordSys() == pi_rPoint.GetCoordSys());

    // Note that the pi_Extremity processing value is ignored since
    // a polygon has no extremities
    return (m_ComplexLinear.IsPointOnSCS(pi_rPoint, HVE2DVector::INCLUDE_EXTREMITIES, pi_Tolerance));
    }


//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DPolygon::Clone() const
    {
    return (new HVE2DPolygon(*this));
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the polygon.
//-----------------------------------------------------------------------------
inline HGF2DExtent HVE2DPolygon::GetExtent() const
    {
    return (m_ComplexLinear.GetExtent());
    }


//-----------------------------------------------------------------------------
// Move
// This method moves the polygon by the specified displacement
//-----------------------------------------------------------------------------
inline void HVE2DPolygon::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    m_ComplexLinear.Move(pi_rDisplacement);
    }

//-----------------------------------------------------------------------------
// Scale
// This method scales the polygon by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
inline void HVE2DPolygon::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factor must be different from 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    m_ComplexLinear.Scale(pi_ScaleFactor, pi_rScaleOrigin);
    }

//-----------------------------------------------------------------------------
// IsEmpty
// This method checks if the polygon is defined or not (represents an empty shape)
//-----------------------------------------------------------------------------
inline bool HVE2DPolygon::IsEmpty() const
    {
    return(m_ComplexLinear.IsEmpty());
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the shape
//-----------------------------------------------------------------------------
inline double HVE2DPolygon::CalculateArea() const
    {
    HGF2DLocation   AreaOrigin = m_ComplexLinear.GetLinear(m_ComplexLinear.GetNumberOfLinears() - 1).GetStartPoint();

    return(fabs(m_ComplexLinear.CalculateRayArea(AreaOrigin)));
    }

//-----------------------------------------------------------------------------
// CalculateRawArea
// This method calculates the raw area (signed) of the shape
//-----------------------------------------------------------------------------
inline double HVE2DPolygon::CalculateRawArea() const
    {
    HGF2DLocation   AreaOrigin = m_ComplexLinear.GetLinear(m_ComplexLinear.GetNumberOfLinears() - 1).GetStartPoint();

    return(m_ComplexLinear.CalculateRayArea(AreaOrigin));
    }

//-----------------------------------------------------------------------------
// CalculateRotationDirection
// This method calculates the rotation direction
//-----------------------------------------------------------------------------
inline HVE2DSimpleShape::RotationDirection HVE2DPolygon::CalculateRotationDirection() const
    {
    HGF2DLocation   AreaOrigin = m_ComplexLinear.GetLinear(m_ComplexLinear.GetNumberOfLinears() - 1).GetStartPoint();
    return (m_ComplexLinear.CalculateRayArea(AreaOrigin) < 0.0 ?
            HVE2DSimpleShape::CW : HVE2DSimpleShape::CCW);
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
inline double HVE2DPolygon::CalculatePerimeter() const
    {
    return(m_ComplexLinear.CalculateLength());
    }

//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the polygon
//-----------------------------------------------------------------------------
inline void HVE2DPolygon::MakeEmpty()
    {
    m_ComplexLinear.MakeEmpty();
    }

//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HVE2DShapeTypeId HVE2DPolygon::GetShapeType() const
    {
    return(HVE2DPolygon::CLASS_ID);
    }

//-----------------------------------------------------------------------------
// Drop
// Returns the description of shape in the form of raw location
// segments
//-----------------------------------------------------------------------------
inline void HVE2DPolygon::Drop(HGF2DLocationCollection* po_pPoints,
                               double                   pi_Tolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    m_ComplexLinear.Drop(po_pPoints, pi_Tolerance, HVE2DLinear::INCLUDE_END_POINT);
    }

// SetAutoToleranceActive
// Sets the auto tolerance active to the components
//-----------------------------------------------------------------------------
inline void HVE2DPolygon::SetAutoToleranceActive(bool pi_AutoToleranceActive)
    {
    // Set auto tolerance setting to component
    m_ComplexLinear.SetAutoToleranceActive(pi_AutoToleranceActive);

    // Call ancester
    HVE2DVector::SetAutoToleranceActive(pi_AutoToleranceActive);
    }


//-----------------------------------------------------------------------------
// SetTolerance
// Sets the tolerance to the component
//-----------------------------------------------------------------------------
inline void HVE2DPolygon::SetTolerance(double pi_Tolerance)
    {
    // The tolerance may not be null of negative
    HPRECONDITION(pi_Tolerance > 0.0);

    // Set auto tolerance setting to component
    m_ComplexLinear.SetTolerance(pi_Tolerance);

    // Call ancester
    HVE2DVector::SetTolerance(pi_Tolerance);
    }

//-----------------------------------------------------------------------------
// SetStrokeTolerance
// Sets the stroke tolerance to the component
//-----------------------------------------------------------------------------
inline void HVE2DPolygon::SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance)
    {
    // Set auto tolerance setting to component
    m_ComplexLinear.SetStrokeTolerance(pi_Tolerance);

    // Call ancester
    HVE2DVector::SetStrokeTolerance(pi_Tolerance);
    }

END_IMAGEPP_NAMESPACE
