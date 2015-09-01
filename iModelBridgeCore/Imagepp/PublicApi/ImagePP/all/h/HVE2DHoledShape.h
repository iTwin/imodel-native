//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DHoledShape.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DShape.h"
#include "HVE2DSimpleShape.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements the generic holed shape. An holed shape is a
    simple shape to which is removed parts of the internal area by use of
    other disjoint or flirting simple shapes positioned inside the area of
    the outter simple shape.
    Any type of simple shape (HVE2DSimpleShape) can define the outter shape
    or the removed parts (holes).

    As mentionned, it is possible for holes to flirt either another
    hole (as represented) or the outter shape. They must not intersect
    their boundaries, nr be contiguous however. This rule permits representation
    of any holed shape imaginable in the 2D space.

    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
class HVE2DHoledShape : public HVE2DShape
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DShapeId_Holed)

public:

    // Primary methods
    HVE2DHoledShape();
    HVE2DHoledShape(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DHoledShape(const HGF2DHoledShape& pi_rLighShape, const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DHoledShape(const HVE2DSimpleShape& pi_rSimpleShape);
    HVE2DHoledShape(const HVE2DHoledShape&   pi_rObject);
    virtual            ~HVE2DHoledShape();

    HVE2DHoledShape&           operator=(const HVE2DHoledShape& pi_rObj);

    // Setting and management
    void                       AddHole(const HVE2DSimpleShape& pi_rSimpleShape);
    virtual void               SetBaseShape(const HVE2DSimpleShape& pi_rComplex);
    virtual const HVE2DSimpleShape&
                               GetBaseShape() const;

    // From HVE2DShape

    virtual void               Rasterize(HGFScanLines& pio_rScanlines) const;

    virtual bool               IsSimple() const;
    virtual bool               IsComplex() const;
    virtual bool               IsEmpty() const;
    virtual HVE2DShapeTypeId   GetShapeType() const;

    virtual const HVE2DShape::ShapeList&
                               GetShapeList() const;
    virtual bool               HasHoles() const;
    virtual const HVE2DShape::HoleList&    
                               GetHoleList() const;
    virtual HVE2DShape*        DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*        DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*        IntersectShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*        UnifyShapeSCS(const HVE2DShape& pi_rShape) const;

    virtual double             CalculateArea() const;
    virtual double             CalculatePerimeter() const;
    virtual bool               IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    void                       MakeEmpty();
    virtual HVE2DShape::SpatialPosition
                               CalculateSpatialPositionOfSingleComponentVector(
                                     const HVE2DVector& pi_rVector) const;

    virtual void               Drop(HGF2DLocationCollection* po_pPoint,
                                    double       pi_Tolerance) const;

    virtual HGF2DShape*        GetLightShape() const;


    // From HVE2DVector
    virtual HGF2DLocation      CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    virtual size_t             Intersect(const HVE2DVector& pi_rVector,
                                         HGF2DLocationCollection* po_pCrossPoints) const;
    virtual size_t             ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                          HGF2DLocationCollection* po_pContiguousnessPoints) const;
    virtual void               ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                            const HGF2DLocation& pi_rPoint,
                                                            HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                            HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    virtual HPMPersistentObject*
                               Clone() const;
    virtual HVE2DVector*       AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    virtual bool               Crosses(const HVE2DVector& pi_rVector) const;
    virtual bool               AreContiguous(const HVE2DVector& pi_rVector) const;
    virtual bool               AreAdjacent(const HVE2DVector& pi_rVector) const;
    virtual bool               IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                         HVE2DVector::ExtremityProcessing
                                         pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                         double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool               IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                            HVE2DVector::ExtremityProcessing
                                            pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                            double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool               AreContiguousAt(const HVE2DVector& pi_rVector,
                                               const HGF2DLocation& pi_rPoint) const;
    virtual HGFBearing         CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                                HVE2DVector::ArbitraryDirection
                                                pi_Direction = HVE2DVector::BETA) const;
    virtual double             CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                            HVE2DVector::ArbitraryDirection
                                                            pi_Direction = HVE2DVector::BETA) const;


    // From HGFGraphicObject
    virtual HGF2DExtent        GetExtent() const;
    virtual void               Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void               Scale(double pi_ScaleFactor,
                                     const HGF2DLocation& pi_rScaleOrigin);

    virtual void               SetAutoToleranceActive(bool pi_ActiveAutoTolerance);
    virtual void               SetTolerance(double pi_Tolerance);
    virtual void               SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance);

    virtual void               PrintState(ostream& po_rOutput) const;

protected:

    virtual void               SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

private:

    HVE2DShape*                DifferentiateFromSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const;
    HVE2DShape*                DifferentiateFromHoledShapeSCS(const HVE2DHoledShape& pi_rHoledShape) const;
    HVE2DShape*                DifferentiateSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const;
    HVE2DShape*                DifferentiateHoledShapeSCS(const HVE2DHoledShape& pi_rHoledShape) const;
    HVE2DShape*                IntersectSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const;
    HVE2DShape*                IntersectHoledShapeSCS(const HVE2DHoledShape& pi_rHoledShape) const;
    HVE2DShape*                UnifySimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const;
    HVE2DShape*                UnifyHoledShapeSCS(const HVE2DHoledShape& pi_rHoledShape) const;

    // Base shape
    HFCPtr<HVE2DSimpleShape> m_pBaseShape;

    // List of holes
    HoleList           m_HoleList;
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DHoledShape.hpp"
