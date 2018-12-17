//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DVoidShape.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "HGF2DSegment.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF2DVoidShape::HGF2DVoidShape()
    : HGF2DSimpleShape()
    {
    }



//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DVoidShape object.
//-----------------------------------------------------------------------------
inline HGF2DVoidShape::HGF2DVoidShape(const HGF2DVoidShape& pi_rObj)
    : HGF2DSimpleShape(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DVoidShape::~HGF2DVoidShape()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
inline HGF2DVoidShape& HGF2DVoidShape::operator=(const HGF2DVoidShape& pi_rObj)
    {
    // Call ancester operator
    HGF2DSimpleShape::operator=(pi_rObj);

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the shape
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DLinear> HGF2DVoidShape::GetLinear() const
    {
    // return an empty linear
    return(new HGF2DSegment());
    }

//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the shape
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DLinear> HGF2DVoidShape::GetLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // return an empty linear
    return (new HGF2DSegment());
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the shape is adjacent with given vector.
//-----------------------------------------------------------------------------
inline bool HGF2DVoidShape::AreAdjacent(const HGF2DVector& pi_rVector) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the shape is contiguous with given vector.
//-----------------------------------------------------------------------------
inline bool HGF2DVoidShape::AreContiguous(const HGF2DVector& pi_rVector) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// IsPointOn
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
inline bool HGF2DVoidShape::IsPointOn(const HGF2DPosition& pi_rPoint,
                                       HGF2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                       double pi_Tolerance) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HGF2DVector* HGF2DVoidShape::Clone() const
    {
    return (new HGF2DVoidShape(*this));
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the polygon.
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent HGF2DVoidShape::GetExtent() const
    {
    return (HGF2DLiteExtent());
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the shape by the specified displacement
//-----------------------------------------------------------------------------
inline void HGF2DVoidShape::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // nothing to do
    }

//-----------------------------------------------------------------------------
// IsEmpty
// This method checks if the polygon is defined or not (represents an empty shape)
//-----------------------------------------------------------------------------
inline bool HGF2DVoidShape::IsEmpty() const
    {
    return(true);
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the shape
//-----------------------------------------------------------------------------
inline double HGF2DVoidShape::CalculateArea() const
    {
    return 0.0;
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
inline double HGF2DVoidShape::CalculatePerimeter() const
    {
    return 0.0;
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the polygon area
//-----------------------------------------------------------------------------
inline bool HGF2DVoidShape::IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const
    {
    return(false);
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the polygon
//-----------------------------------------------------------------------------
inline void HGF2DVoidShape::MakeEmpty()
    {
    // Nothing to do
    }


//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HGF2DShapeTypeId HGF2DVoidShape::GetShapeType() const
    {
    return(HGF2DVoidShape::CLASS_ID);
    }


//-----------------------------------------------------------------------------
// UnifyShape
// This method create a new shape as the union between self and given.
//-----------------------------------------------------------------------------
inline HGF2DShape* HGF2DVoidShape::UnifyShape(const HGF2DShape& pi_rShape) const
    {
    // The union is always the other (given) shape
    return static_cast<HGF2DShape*>(pi_rShape.Clone());
    }



//-----------------------------------------------------------------------------
// IntersectShape
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
inline HGF2DShape* HGF2DVoidShape::IntersectShape(const HGF2DShape& pi_rShape) const
    {
    // The intersection is always void
    return(new HGF2DVoidShape(*this));
    }


//-----------------------------------------------------------------------------
// DifferentiateFromShape
// This method creates a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
inline HGF2DShape* HGF2DVoidShape::DifferentiateFromShape(const HGF2DShape& pi_rShape) const
    {
    HGF2DShape* pResultShape;

    // The diff from is always the other (given) shape

    pResultShape = (HGF2DShape*)pi_rShape.Clone();

    return(pResultShape);
    }


//-----------------------------------------------------------------------------
// DifferentiateShape
// This method create a new shape as the difference between self and given.
//-----------------------------------------------------------------------------
inline HGF2DShape* HGF2DVoidShape::DifferentiateShape(const HGF2DShape& pi_rShape) const
    {
    // Nothing in shape ... return void
    return new HGF2DVoidShape();
    }

//-----------------------------------------------------------------------------
// CalculateSpatialPositionOf
// This method returns the spatial position relative to shape of given point
//-----------------------------------------------------------------------------
inline HGF2DShape::SpatialPosition HGF2DVoidShape::CalculateSpatialPositionOf(const HGF2DPosition& pi_rPoint) const
    {
    return(HGF2DShape::S_OUT);
    }

//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
inline HGF2DPosition HGF2DVoidShape::CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const
    {
    return(HGF2DPosition(DBL_MAX, DBL_MAX));
    }




//-----------------------------------------------------------------------------
// Scale
// This method scales the rectangle by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
inline void HGF2DVoidShape::Scale(double pi_ScaleFactor, const HGF2DPosition& pi_rScaleOrigin)
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
inline void HGF2DVoidShape::Scale(double pi_ScaleFactorX,
                                  double pi_ScaleFactorY,
                                  const HGF2DPosition& pi_rScaleOrigin)
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
inline bool HGF2DVoidShape::Crosses(const HGF2DVector& pi_rVector) const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// Intersect
// This method checks if the intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
inline size_t HGF2DVoidShape::Intersect(const HGF2DVector& pi_rVector,
                                        HGF2DPositionCollection* po_pCrossPoints) const
    {
    return (0);
    }

//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the rectangle is contiguous with given vector
// at specified point.
//-----------------------------------------------------------------------------
inline bool HGF2DVoidShape::AreContiguousAt(const HGF2DVector& pi_rVector,
                                             const HGF2DPosition& pi_rPoint) const
    {
    return(false);
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
inline void HGF2DVoidShape::ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
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
inline size_t HGF2DVoidShape::ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                         HGF2DPositionCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(AreContiguous(pi_rVector));

    return(0);
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of rectangle at specified point
//-----------------------------------------------------------------------------
inline double HGF2DVoidShape::CalculateAngularAcceleration(const HGF2DPosition& pi_rPoint,
                                                                           HGF2DVector::ArbitraryDirection pi_Direction) const
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
inline HGFBearing HGF2DVoidShape::CalculateBearing(const HGF2DPosition& pi_rPoint,
                                                   HGF2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    return HGFBearing(0.0);
    }



//-----------------------------------------------------------------------------
// Drop
// This method drops the shape
//-----------------------------------------------------------------------------
inline void HGF2DVoidShape::Drop(HGF2DPositionCollection* po_pPoints,
                               double pi_rTolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    // Nothing to do
    }

END_IMAGEPP_NAMESPACE

