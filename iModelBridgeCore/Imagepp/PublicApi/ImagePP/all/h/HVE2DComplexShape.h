//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DComplexShape.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DComplexShape
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DShape.h"


BEGIN_IMAGEPP_NAMESPACE
class HGF2DComplexShape;
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements the generic complex shape (complex shape). A
    complex shape is constituted of a list of shapes (including simple
    shapes, holed shapes and complex shape themselves). Which as a whole
    define part of the 2D space. The different constituting shapes may not
    intersect(either by their boundary or defined area) nor be
    contiguous one to the other. A complex shape is capable of
    representing any desired area or set of desired area in space.

    It is possible for shapes to flirt either another shape (as
    represented) or holes of a shape. They must not intersect
    their boundaries, nor be contiguous however. This rule
    permits representation of any complex shape imaginable
    in the 2D space.
    -----------------------------------------------------------------------------
*/
class HVE2DComplexShape : public HVE2DShape
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DShapeId_Complex)

public:

    // Primary methods
    HVE2DComplexShape();
    HVE2DComplexShape(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DComplexShape(const HVE2DShape::SimpleShapeList& pi_rList);
    HVE2DComplexShape(const HGF2DComplexShape& pi_rShape, const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DComplexShape(const HVE2DShape& pi_rShape);
    HVE2DComplexShape(const HVE2DComplexShape&    pi_rObject);
    virtual            ~HVE2DComplexShape();

    IMAGEPPTEST_EXPORT HVE2DComplexShape&      operator=(const HVE2DComplexShape& pi_rObj);

    // Setting
    void               AddShape(const HVE2DShape& pi_rShape);

    // From HVE2DShape

    IMAGEPPTEST_EXPORT void       Rasterize(HGFScanLines& pio_rScanlines) const override;

    bool      IsSimple() const override;
    bool      IsComplex() const override;
    HVE2DShapeTypeId
    GetShapeType() const override;
    bool      IsEmpty () const override;
    const HVE2DShape::ShapeList&
                      GetShapeList() const override;

    bool      HasHoles() const override;
    const HVE2DShape::HoleList&
                      GetHoleList() const override;

    IMAGEPPTEST_EXPORT HVE2DShape*    DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT HVE2DShape*    DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT HVE2DShape*    IntersectShapeSCS(const HVE2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT HVE2DShape*    UnifyShapeSCS(const HVE2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT double         CalculateArea() const override;
    IMAGEPPTEST_EXPORT double         CalculatePerimeter() const override;
    IMAGEPPTEST_EXPORT bool           IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPPTEST_EXPORT void                   MakeEmpty() override;
    IMAGEPPTEST_EXPORT HVE2DShape::SpatialPosition     
                           CalculateSpatialPositionOfSingleComponentVector(const HVE2DVector& pi_rVector) const override;

    IMAGEPPTEST_EXPORT void           Drop(HGF2DLocationCollection* po_pPoint,
                                double                   pi_Tolerance) const override;

    IMAGEPPTEST_EXPORT HGF2DShape*    GetLightShape() const override;


    // From HVE2DVector
    IMAGEPPTEST_EXPORT HGF2DLocation  CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const override;
    IMAGEPPTEST_EXPORT size_t         Intersect(const HVE2DVector& pi_rVector,
                                     HGF2DLocationCollection* po_pCrossPoints) const override;
    IMAGEPPTEST_EXPORT size_t         ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                      HGF2DLocationCollection* po_pContiguousnessPoints) const override;
    IMAGEPPTEST_EXPORT void           ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                        const HGF2DLocation& pi_rPoint,
                                                        HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                        HGF2DLocation* pi_pSecondContiguousnessPoint) const override;
    IMAGEPPTEST_EXPORT HVE2DVector*   AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const override;
    IMAGEPPTEST_EXPORT bool           Crosses(const HVE2DVector& pi_rVector) const override;
    IMAGEPPTEST_EXPORT bool           AreContiguous(const HVE2DVector& pi_rVector) const override;
    IMAGEPPTEST_EXPORT bool           AreAdjacent(const HVE2DVector& pi_rVector) const override;
    IMAGEPPTEST_EXPORT bool           IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                     HVE2DVector::ExtremityProcessing
                                     pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                     double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPPTEST_EXPORT bool           IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                        HVE2DVector::ExtremityProcessing
                                        pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                        double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPPTEST_EXPORT bool           AreContiguousAt(const HVE2DVector& pi_rVector,
                                           const HGF2DLocation& pi_rPoint) const override;
    IMAGEPPTEST_EXPORT HGFBearing     CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                            HVE2DVector::ArbitraryDirection
                                            pi_Direction = HVE2DVector::BETA) const override;
    IMAGEPPTEST_EXPORT double         CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                        HVE2DVector::ArbitraryDirection
                                                        pi_Direction = HVE2DVector::BETA) const override;

    // From HGFGraphicObject
    IMAGEPPTEST_EXPORT HGF2DExtent    GetExtent() const override;
    IMAGEPPTEST_EXPORT void           Move(const HGF2DDisplacement& pi_rDisplacement) override;
    IMAGEPPTEST_EXPORT void           Scale(double pi_ScaleFactor,
                                 const HGF2DLocation& pi_rScaleOrigin) override;

    IMAGEPPTEST_EXPORT void           SetAutoToleranceActive(bool pi_ActiveAutoTolerance) override;
    IMAGEPPTEST_EXPORT void           SetTolerance(double pi_Tolerance) override;
    IMAGEPPTEST_EXPORT void           SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance) override;


    // From HPMPersistentObject
    HPMPersistentObject*
                           Clone() const override;

    // Debugging
    IMAGEPPTEST_EXPORT void           PrintState(ostream& po_rOutput) const override;

protected:

    IMAGEPPTEST_EXPORT void           SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) override;

private:

    // List of shapes
    ShapeList      m_ShapeList;
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DComplexShape.hpp"
