//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    IMAGEPPTEST_EXPORT HVE2DHoledShape();
    IMAGEPPTEST_EXPORT HVE2DHoledShape(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPPTEST_EXPORT HVE2DHoledShape(const HGF2DHoledShape& pi_rLighShape, const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPPTEST_EXPORT HVE2DHoledShape(const HVE2DSimpleShape& pi_rSimpleShape);
    IMAGEPPTEST_EXPORT HVE2DHoledShape(const HVE2DHoledShape&   pi_rObject);
    virtual            ~HVE2DHoledShape();

    HVE2DHoledShape&           operator=(const HVE2DHoledShape& pi_rObj);

    // Setting and management
    IMAGEPPTEST_EXPORT void                       AddHole(const HVE2DSimpleShape& pi_rSimpleShape);
    IMAGEPPTEST_EXPORT virtual void               SetBaseShape(const HVE2DSimpleShape& pi_rComplex);
    IMAGEPPTEST_EXPORT virtual const HVE2DSimpleShape&
                               GetBaseShape() const;

    // From HVE2DShape

    IMAGEPPTEST_EXPORT void               Rasterize(HGFScanLines& pio_rScanlines) const override;

    bool               IsSimple() const override;
    bool               IsComplex() const override;
    bool               IsEmpty() const override;
    HVE2DShapeTypeId   GetShapeType() const override;

    const HVE2DShape::ShapeList&
                               GetShapeList() const override;
    bool               HasHoles() const override;
    const HVE2DShape::HoleList&    
                               GetHoleList() const override;
    IMAGEPPTEST_EXPORT HVE2DShape*        DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT HVE2DShape*        DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT HVE2DShape*        IntersectShapeSCS(const HVE2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT HVE2DShape*        UnifyShapeSCS(const HVE2DShape& pi_rShape) const override;

    IMAGEPPTEST_EXPORT double             CalculateArea() const override;
    IMAGEPPTEST_EXPORT double             CalculatePerimeter() const override;
    IMAGEPPTEST_EXPORT bool               IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPPTEST_EXPORT void                       MakeEmpty() override;
    IMAGEPPTEST_EXPORT HVE2DShape::SpatialPosition
                               CalculateSpatialPositionOfSingleComponentVector(
                                     const HVE2DVector& pi_rVector) const override;

    IMAGEPPTEST_EXPORT void               Drop(HGF2DLocationCollection* po_pPoint,
                                    double       pi_Tolerance) const override;

    IMAGEPPTEST_EXPORT HGF2DShape*        GetLightShape() const override;


    // From HVE2DVector
    IMAGEPPTEST_EXPORT HGF2DLocation      CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const override;
    IMAGEPPTEST_EXPORT size_t             Intersect(const HVE2DVector& pi_rVector,
                                         HGF2DLocationCollection* po_pCrossPoints) const override;
    IMAGEPPTEST_EXPORT size_t             ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                          HGF2DLocationCollection* po_pContiguousnessPoints) const override;
    IMAGEPPTEST_EXPORT void               ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                            const HGF2DLocation& pi_rPoint,
                                                            HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                            HGF2DLocation* pi_pSecondContiguousnessPoint) const override;
    IMAGEPPTEST_EXPORT HPMPersistentObject*
                               Clone() const override;
    IMAGEPPTEST_EXPORT HVE2DVector*       AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const override;
    IMAGEPPTEST_EXPORT bool               Crosses(const HVE2DVector& pi_rVector) const override;
    IMAGEPPTEST_EXPORT bool               AreContiguous(const HVE2DVector& pi_rVector) const override;
    IMAGEPPTEST_EXPORT bool               AreAdjacent(const HVE2DVector& pi_rVector) const override;
    IMAGEPPTEST_EXPORT bool               IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                         HVE2DVector::ExtremityProcessing
                                         pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                         double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPPTEST_EXPORT bool               IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                            HVE2DVector::ExtremityProcessing
                                            pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                            double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPPTEST_EXPORT bool               AreContiguousAt(const HVE2DVector& pi_rVector,
                                               const HGF2DLocation& pi_rPoint) const override;
    IMAGEPPTEST_EXPORT HGFBearing         CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                                HVE2DVector::ArbitraryDirection
                                                pi_Direction = HVE2DVector::BETA) const override;
    IMAGEPPTEST_EXPORT double             CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                            HVE2DVector::ArbitraryDirection
                                                            pi_Direction = HVE2DVector::BETA) const override;


    // From HGFGraphicObject
    IMAGEPPTEST_EXPORT HGF2DExtent        GetExtent() const override;
    IMAGEPPTEST_EXPORT void               Move(const HGF2DDisplacement& pi_rDisplacement) override;
    IMAGEPPTEST_EXPORT void               Scale(double pi_ScaleFactor,
                                     const HGF2DLocation& pi_rScaleOrigin) override;

    IMAGEPPTEST_EXPORT void               SetAutoToleranceActive(bool pi_ActiveAutoTolerance) override;
    IMAGEPPTEST_EXPORT void               SetTolerance(double pi_Tolerance) override;
    IMAGEPPTEST_EXPORT void               SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance) override;

    IMAGEPPTEST_EXPORT void               PrintState(ostream& po_rOutput) const override;

protected:

    IMAGEPPTEST_EXPORT void               SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) override;

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
