//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DUniverse.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
inline HGF2DUniverse& HGF2DUniverse::operator=(const HGF2DUniverse& pi_rObj)
    {
    // Call ancester operator
    HGF2DSimpleShape::operator=(pi_rObj);

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the shape is adjacent with given vector.
//-----------------------------------------------------------------------------
inline bool HGF2DUniverse::AreAdjacent(const HGF2DVector& pi_rVector) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the shape is contiguous with given vector.
//-----------------------------------------------------------------------------
inline bool HGF2DUniverse::AreContiguous(const HGF2DVector& pi_rVector) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// IsPointOn
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
inline bool HGF2DUniverse::IsPointOn(const HGF2DPosition& pi_rPoint,
                                      HGF2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                      double pi_Tolerance) const
    {
    return(false);
    }



//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HGF2DVector* HGF2DUniverse::Clone() const
    {
    return (new HGF2DUniverse(*this));
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the polygon.
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent HGF2DUniverse::GetExtent() const
    {
    return (HGF2DLiteExtent((-DBL_MAX),
                        (-DBL_MAX),
                        DBL_MAX,
                        DBL_MAX));
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the shape by the specified displacement
//-----------------------------------------------------------------------------
inline void HGF2DUniverse::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // nothing to do
    }


//-----------------------------------------------------------------------------
// Scale
// This method scales the shape by the specified scaling factor
//-----------------------------------------------------------------------------
inline void HGF2DUniverse::Scale(double pi_ScaleFactor, const HGF2DPosition& pi_rScaleOrigin)
    {
    // The given scale factor must not be zero
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // nothing to do
    }


//-----------------------------------------------------------------------------
// Scale
// This method scales the shape by the specified scaling factor
//-----------------------------------------------------------------------------
inline void HGF2DUniverse::Scale(double pi_ScaleFactorX,
                                 double pi_ScaleFactorY,
                                 const HGF2DPosition& pi_rScaleOrigin)
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
inline bool HGF2DUniverse::IsEmpty() const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the shape
//-----------------------------------------------------------------------------
inline double HGF2DUniverse::CalculateArea() const
    {
    return numeric_limits<double>::infinity();
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
inline double HGF2DUniverse::CalculatePerimeter() const
    {
    return numeric_limits<double>::infinity();
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the polygon area
//-----------------------------------------------------------------------------
inline bool HGF2DUniverse::IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const
    {
    return(true);
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the polygon
//-----------------------------------------------------------------------------
inline void HGF2DUniverse::MakeEmpty()
    {
    // Nothing to do
    }


//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HGF2DShapeTypeId HGF2DUniverse::GetShapeType() const
    {
    return(HGF2DUniverse::CLASS_ID);
    }


//-----------------------------------------------------------------------------
// UnifyShape
// This method create a new shape as the union between self and given.
//-----------------------------------------------------------------------------
inline HGF2DShape* HGF2DUniverse::UnifyShape(const HGF2DShape& pi_rShape) const
    {
    return(new HGF2DUniverse());
    }


//-----------------------------------------------------------------------------
// IntersectShape
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
inline HGF2DShape* HGF2DUniverse::IntersectShape(const HGF2DShape& pi_rShape) const
    {
    // The intrsection is always the other (given) shape
    return(static_cast<HGF2DShape*>(pi_rShape.Clone()));
    }


//-----------------------------------------------------------------------------
// DifferentiateFromShape
// This method creates a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
inline HGF2DShape* HGF2DUniverse::DifferentiateFromShape(const HGF2DShape& pi_rShape) const
    {
    return(new HGF2DVoidShape());
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOf
// This method returns the spatial position relative to shape of given point
//-----------------------------------------------------------------------------
inline HGF2DShape::SpatialPosition HGF2DUniverse::CalculateSpatialPositionOf(const HGF2DPosition& pi_rPoint) const
    {
    return(HGF2DShape::S_IN);
    }


//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
inline HGF2DPosition HGF2DUniverse::CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const
    {
    return HGF2DPosition(DBL_MAX, DBL_MAX);
    }



//-----------------------------------------------------------------------------
// Crosses
// This method checks if the polygon crosses with given vector.
//-----------------------------------------------------------------------------
inline bool HGF2DUniverse::Crosses(const HGF2DVector& pi_rVector) const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// Intersect
// This method checks if the intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
inline size_t HGF2DUniverse::Intersect(const HGF2DVector& pi_rVector,
                                       HGF2DPositionCollection* po_pCrossPoints) const
    {
    return (0);
    }

//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the rectangle is contiguous with given vector
// at specified point.
//-----------------------------------------------------------------------------
inline bool HGF2DUniverse::AreContiguousAt(const HGF2DVector& pi_rVector,
                                            const HGF2DPosition& pi_rPoint) const
    {
    return(false);
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
inline void HGF2DUniverse::ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                        const HGF2DPosition& pi_rPoint,
                                                        HGF2DPosition* po_pFirstContiguousnessPoint,
                                                        HGF2DPosition* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
inline size_t HGF2DUniverse::ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                        HGF2DPositionCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(AreContiguous(pi_rVector));

    return(0);
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of rectangle at specified point
//-----------------------------------------------------------------------------
inline double HGF2DUniverse::CalculateAngularAcceleration(const HGF2DPosition& pi_rPoint,
                                                                          HGF2DVector::ArbitraryDirection pi_Direction) const
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
inline HGFBearing HGF2DUniverse::CalculateBearing(const HGF2DPosition& pi_rPoint,
                                            HGF2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    return HGFBearing(0.0);
    }




//-----------------------------------------------------------------------------
// CreateScanLines
// This method creates the scan lines for the present shape
//-----------------------------------------------------------------------------
inline void HGF2DUniverse::Rasterize(HGFScanLines& pio_rScanlines) const
    {
    // The universe is too big to rasterize it :)
    HASSERT(0);
    }

END_IMAGEPP_NAMESPACE