//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DUniverse.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
inline HVE2DUniverse& HVE2DUniverse::operator=(const HVE2DUniverse& pi_rObj)
    {
    // Call ancester operator
    HVE2DSimpleShape::operator=(pi_rObj);

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the shape is adjacent with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DUniverse::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the shape is contiguous with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DUniverse::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// IsPointOn
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
inline bool HVE2DUniverse::IsPointOn(const HGF2DLocation& pi_rPoint,
                                      HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                      double pi_Tolerance) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
inline bool HVE2DUniverse::IsPointOnSCS(const HGF2DLocation& pi_rPoint,
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
inline HPMPersistentObject* HVE2DUniverse::Clone() const
    {
    return (new HVE2DUniverse(*this));
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the polygon.
//-----------------------------------------------------------------------------
inline HGF2DExtent HVE2DUniverse::GetExtent() const
    {
    return (HGF2DExtent((-DBL_MAX),
                        (-DBL_MAX),
                        DBL_MAX,
                        DBL_MAX,
                        GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the shape by the specified displacement
//-----------------------------------------------------------------------------
inline void HVE2DUniverse::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // nothing to do
    }


//-----------------------------------------------------------------------------
// Scale
// This method scales the shape by the specified scaling factor
//-----------------------------------------------------------------------------
inline void HVE2DUniverse::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factor must not be zero
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // nothing to do
    }


//-----------------------------------------------------------------------------
// Scale
// This method scales the shape by the specified scaling factor
//-----------------------------------------------------------------------------
inline void HVE2DUniverse::Scale(double pi_ScaleFactorX,
                                 double pi_ScaleFactorY,
                                 const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factor must not be zero
    HPRECONDITION(pi_ScaleFactorX != 0.0);
    HPRECONDITION(pi_ScaleFactorY != 0.0);

    // nothing to do
    }


//-----------------------------------------------------------------------------
// IsEmpty
// This method checks if the polygon is defined or not (represents an empty shape)
//-----------------------------------------------------------------------------
inline bool HVE2DUniverse::IsEmpty() const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the shape
//-----------------------------------------------------------------------------
inline double HVE2DUniverse::CalculateArea() const
    {
    return (numeric_limits<double>::infinity());
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
inline double HVE2DUniverse::CalculatePerimeter() const
    {
    return numeric_limits<double>::infinity();
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the polygon area
//-----------------------------------------------------------------------------
inline bool HVE2DUniverse::IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const
    {
    return(true);
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the polygon
//-----------------------------------------------------------------------------
inline void HVE2DUniverse::MakeEmpty()
    {
    // Nothing to do
    }


//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HVE2DShapeTypeId HVE2DUniverse::GetShapeType() const
    {
    return(HVE2DUniverse::CLASS_ID);
    }

//-----------------------------------------------------------------------------
// UnifyShapeSCS
// This method create a new shape as the union between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DUniverse::UnifyShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    return(new HVE2DUniverse(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// UnifyShape
// This method create a new shape as the union between self and given.
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DUniverse::UnifyShape(const HVE2DShape& pi_rShape) const
    {
    return(new HVE2DUniverse(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// IntersectShapeSCS
// This method create a new shape as the intersection between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DUniverse::IntersectShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shape must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    // The intrsection is always the other (given) shape
    return((HVE2DShape*)pi_rShape.AllocateCopyInCoordSys(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// IntersectShape
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DUniverse::IntersectShape(const HVE2DShape& pi_rShape) const
    {
    // The intrsection is always the other (given) shape
    return((HVE2DShape*)pi_rShape.AllocateCopyInCoordSys(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// DifferentiateFromShapeSCS
// This method creates a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DUniverse::DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    return(new HVE2DVoidShape(GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// DifferentiateFromShape
// This method creates a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
inline HVE2DShape* HVE2DUniverse::DifferentiateFromShape(const HVE2DShape& pi_rShape) const
    {
    return(new HVE2DVoidShape(GetCoordSys()));
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOf
// This method returns the spatial position relative to shape of given point
//-----------------------------------------------------------------------------
inline HVE2DShape::SpatialPosition HVE2DUniverse::CalculateSpatialPositionOf(const HGF2DLocation& pi_rPoint) const
    {
    return(HVE2DShape::S_IN);
    }


//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
inline HGF2DLocation HVE2DUniverse::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    return(HGF2DLocation(DBL_MAX, DBL_MAX, GetCoordSys()));
    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HVE2DVector* HVE2DUniverse::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    return(new HVE2DUniverse(pi_rpCoordSys));
    }


//-----------------------------------------------------------------------------
// Crosses
// This method checks if the polygon crosses with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DUniverse::Crosses(const HVE2DVector& pi_rVector) const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// Intersect
// This method checks if the intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
inline size_t HVE2DUniverse::Intersect(const HVE2DVector& pi_rVector,
                                       HGF2DLocationCollection* po_pCrossPoints) const
    {
    return (0);
    }

//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the rectangle is contiguous with given vector
// at specified point.
//-----------------------------------------------------------------------------
inline bool HVE2DUniverse::AreContiguousAt(const HVE2DVector& pi_rVector,
                                            const HGF2DLocation& pi_rPoint) const
    {
    return(false);
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
inline void HVE2DUniverse::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
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
inline size_t HVE2DUniverse::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                        HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(AreContiguous(pi_rVector));

    return(0);
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of rectangle at specified point
//-----------------------------------------------------------------------------
inline double HVE2DUniverse::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                                                          HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The acceleration on a rectangle is always 0.0
    return 0.0;
    }


//-----------------------------------------------------------------------------
// CalculateBearing
// Calculates and returns the bearing of rectangle at specified point
//-----------------------------------------------------------------------------
inline HGFBearing HVE2DUniverse::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                                  HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    return HGFBearing(0.0);
    }




//-----------------------------------------------------------------------------
// CreateScanLines
// This method creates the scan lines for the present shape
//-----------------------------------------------------------------------------
inline void HVE2DUniverse::Rasterize(HGFScanLines& pio_rScanlines) const
    {
    // The universe is too big to rasterize it :)
    HASSERT(0);
    }

END_IMAGEPP_NAMESPACE
