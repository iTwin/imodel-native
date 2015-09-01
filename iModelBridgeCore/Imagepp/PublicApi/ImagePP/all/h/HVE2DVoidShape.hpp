//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DVoidShape.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------



BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
inline HVE2DVoidShape& HVE2DVoidShape::operator=(const HVE2DVoidShape& pi_rObj)
    {
    // Call ancester operator
    HVE2DSimpleShape::operator=(pi_rObj);

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the shape
//-----------------------------------------------------------------------------
inline HVE2DComplexLinear HVE2DVoidShape::GetLinear() const
    {
    // return an empty linear
    return(HVE2DComplexLinear(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the shape
//-----------------------------------------------------------------------------
inline HVE2DComplexLinear HVE2DVoidShape::GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // return an empty linear
    return(HVE2DComplexLinear(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// AllocateLinear
// This method returns a linear descibing the path of the shape
//-----------------------------------------------------------------------------
inline HVE2DComplexLinear* HVE2DVoidShape::AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // return an empty linear
    return(new HVE2DComplexLinear(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the shape is adjacent with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DVoidShape::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the shape is contiguous with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DVoidShape::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// IsPointOn
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
inline bool HVE2DVoidShape::IsPointOn(const HGF2DLocation& pi_rPoint,
                                       HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                       double pi_Tolerance) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
inline bool HVE2DVoidShape::IsPointOnSCS(const HGF2DLocation& pi_rPoint,
                                          HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                          double pi_Tolerance) const
    {
    HPRECONDITION(GetCoordSys() == pi_rPoint.GetCoordSys());

    return(false);
    }


//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DVoidShape::Clone() const
    {
    return (new HVE2DVoidShape(*this));
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the polygon.
//-----------------------------------------------------------------------------
inline HGF2DExtent HVE2DVoidShape::GetExtent() const
    {
    return (HGF2DExtent(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the shape by the specified displacement
//-----------------------------------------------------------------------------
inline void HVE2DVoidShape::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // nothing to do
    }

//-----------------------------------------------------------------------------
// IsEmpty
// This method checks if the polygon is defined or not (represents an empty shape)
//-----------------------------------------------------------------------------
inline bool HVE2DVoidShape::IsEmpty() const
    {
    return(true);
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the shape
//-----------------------------------------------------------------------------
inline double HVE2DVoidShape::CalculateArea() const
    {
    return 0.0;
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
inline double HVE2DVoidShape::CalculatePerimeter() const
    {
    return 0.0;
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the polygon area
//-----------------------------------------------------------------------------
inline bool HVE2DVoidShape::IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const
    {
    return(false);
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the polygon
//-----------------------------------------------------------------------------
inline void HVE2DVoidShape::MakeEmpty()
    {
    // Nothing to do
    }


//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HVE2DShapeTypeId HVE2DVoidShape::GetShapeType() const
    {
    return(HVE2DVoidShape::CLASS_ID);
    }


//-----------------------------------------------------------------------------
// UnifyShapeSCS
// This method create a new shape as the union between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DVoidShape::UnifyShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    // The union is always the other (given) shape
    return((HVE2DShape*)pi_rShape.Clone());
    }

//-----------------------------------------------------------------------------
// UnifyShape
// This method create a new shape as the union between self and given.
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DVoidShape::UnifyShape(const HVE2DShape& pi_rShape) const
    {
    HVE2DShape* pResultShape;

    // The union is always the other (given) shape

    // Check if the given shape has the same coord sys a self
    if (pi_rShape.GetCoordSys() == GetCoordSys())
        pResultShape = (HVE2DShape*)pi_rShape.Clone();
    else
        pResultShape = (HVE2DShape*)pi_rShape.AllocateCopyInCoordSys(GetCoordSys());

    return(pResultShape);
    }

//-----------------------------------------------------------------------------
// IntersectShapeSCS
// This method create a new shape as the intersection between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DVoidShape::IntersectShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shape must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    // The intersection is always void
    return(new HVE2DVoidShape(*this));
    }

//-----------------------------------------------------------------------------
// IntersectShape
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DVoidShape::IntersectShape(const HVE2DShape& pi_rShape) const
    {
    // The intersection is always void
    return(new HVE2DVoidShape(*this));
    }

//-----------------------------------------------------------------------------
// DifferentiateFromShapeSCS
// This method creates a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DVoidShape::DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    return((HVE2DShape*)pi_rShape.Clone());
    }

//-----------------------------------------------------------------------------
// DifferentiateFromShape
// This method creates a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DVoidShape::DifferentiateFromShape(const HVE2DShape& pi_rShape) const
    {
    HVE2DShape* pResultShape;

    // The diff from is always the other (given) shape

    // Check if the given shape has the same coord sys a self
    if (pi_rShape.GetCoordSys() == GetCoordSys())
        pResultShape = (HVE2DShape*)pi_rShape.Clone();
    else
        pResultShape = (HVE2DShape*)pi_rShape.AllocateCopyInCoordSys(GetCoordSys());

    return(pResultShape);
    }

//-----------------------------------------------------------------------------
// DifferentiateShapeSCS
// This method create a new shape as the difference between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DVoidShape::DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    // Nothing in shape ... return void
    return(new HVE2DVoidShape(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// DifferentiateShape
// This method create a new shape as the difference between self and given.
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DVoidShape::DifferentiateShape(const HVE2DShape& pi_rShape) const
    {
    // Nothing in shape ... return void
    return(new HVE2DVoidShape(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// CalculateSpatialPositionOf
// This method returns the spatial position relative to shape of given point
//-----------------------------------------------------------------------------
inline HVE2DShape::SpatialPosition HVE2DVoidShape::CalculateSpatialPositionOf(const HGF2DLocation& pi_rPoint) const
    {
    return(HVE2DShape::S_OUT);
    }

//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
inline HGF2DLocation HVE2DVoidShape::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    return(HGF2DLocation(DBL_MAX, DBL_MAX, GetCoordSys()));
    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HVE2DVector* HVE2DVoidShape::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    return(new HVE2DVoidShape(pi_rpCoordSys));
    }


//-----------------------------------------------------------------------------
// Scale
// This method scales the rectangle by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
inline void HVE2DVoidShape::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factor must not be zero
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // nothing to do
    }

//-----------------------------------------------------------------------------
// Scale
// This method scales the rectangle by the specified scaling factors
// around the given location
//-----------------------------------------------------------------------------
inline void HVE2DVoidShape::Scale(double pi_ScaleFactorX,
                                  double pi_ScaleFactorY,
                                  const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factors must not be zero
    HPRECONDITION(pi_ScaleFactorX != 0.0);
    HPRECONDITION(pi_ScaleFactorY != 0.0);

    // nothing to do
    }



//-----------------------------------------------------------------------------
// Crosses
// This method checks if the polygon crosses with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DVoidShape::Crosses(const HVE2DVector& pi_rVector) const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// Intersect
// This method checks if the intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
inline size_t HVE2DVoidShape::Intersect(const HVE2DVector& pi_rVector,
                                        HGF2DLocationCollection* po_pCrossPoints) const
    {
    return (0);
    }

//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the rectangle is contiguous with given vector
// at specified point.
//-----------------------------------------------------------------------------
inline bool HVE2DVoidShape::AreContiguousAt(const HVE2DVector& pi_rVector,
                                            const HGF2DLocation& pi_rPoint) const
    {
    return(false);
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
inline void HVE2DVoidShape::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                         const HGF2DLocation& pi_rPoint,
                                                         HGF2DLocation* po_pFirstContiguousnessPoint,
                                                         HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
inline size_t HVE2DVoidShape::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                         HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(AreContiguous(pi_rVector));

    return(0);
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of rectangle at specified point
//-----------------------------------------------------------------------------
inline double HVE2DVoidShape::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                                                           HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The acceleration on a void is always 0.0
    return 0.0;
    }


//-----------------------------------------------------------------------------
// CalculateBearing
// Calculates and returns the bearing of rectangle at specified point
//-----------------------------------------------------------------------------
inline HGFBearing HVE2DVoidShape::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                                   HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    return HGFBearing(0.0);
    }



//-----------------------------------------------------------------------------
// Drop
// This method drops the shape
//-----------------------------------------------------------------------------
inline void HVE2DVoidShape::Drop(HGF2DLocationCollection* po_pPoints,
                                 double                   pi_Tolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    // Nothing to do
    }
END_IMAGEPP_NAMESPACE
