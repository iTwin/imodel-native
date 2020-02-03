//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DComplexShape
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DShape.h"
#include "HGF2DDisplacement.h"

BEGIN_IMAGEPP_NAMESPACE
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
class HGF2DComplexShape : public HGF2DShape
    {

    HDECLARE_CLASS_ID(HGF2DShapeId_Complex, HGF2DShape)

public:

    // Primary methods
    HGF2DComplexShape();
    HGF2DComplexShape(const HGF2DShape::SimpleShapeList& pi_rList);
    HGF2DComplexShape(const HGF2DShape& pi_rShape);
    HGF2DComplexShape(const HGF2DComplexShape&    pi_rObject);
    virtual            ~HGF2DComplexShape();

    IMAGEPPTEST_EXPORT HGF2DComplexShape&      operator=(const HGF2DComplexShape& pi_rObj);

    // Setting
    void                    AddShape(const HGF2DShape& pi_rShape);

    // From HGF2DShape

    IMAGEPPTEST_EXPORT void            Rasterize(HGFScanLines& pio_rScanlines) const override;

    bool            IsSimple() const override;
    bool            IsComplex() const override;
    HGF2DShapeTypeId
                            GetShapeType() const override;
    bool            IsEmpty () const override;
    const HGF2DShape::ShapeList&
                            GetShapeList() const override;

    bool            HasHoles() const override;
    const HGF2DShape::HoleList&
                            GetHoleList() const override;

    IMAGEPPTEST_EXPORT HGF2DShape*     DifferentiateFromShape(const HGF2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT HGF2DShape*     DifferentiateShape(const HGF2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT HGF2DShape*     IntersectShape(const HGF2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT HGF2DShape*     UnifyShape(const HGF2DShape& pi_rShape) const override;
    IMAGEPPTEST_EXPORT double          CalculateArea() const override;
    IMAGEPPTEST_EXPORT double          CalculatePerimeter() const override;
    IMAGEPPTEST_EXPORT bool            IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const override;
    IMAGEPPTEST_EXPORT void                    MakeEmpty() override;
    IMAGEPPTEST_EXPORT HGF2DShape::SpatialPosition  
                            CalculateSpatialPositionOfSingleComponentVector(const HGF2DVector& pi_rVector) const override;

    IMAGEPPTEST_EXPORT void            Drop(HGF2DPositionCollection* po_pPoint,
                                 double                   pi_rTolerance) const override;


    // From HGF2DVector
    IMAGEPPTEST_EXPORT HGF2DPosition   CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const override;
    IMAGEPPTEST_EXPORT size_t          Intersect(const HGF2DVector& pi_rVector,
                                      HGF2DPositionCollection* po_pCrossPoints) const override;
    IMAGEPPTEST_EXPORT size_t          ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                       HGF2DPositionCollection* po_pContiguousnessPoints) const override;
    IMAGEPPTEST_EXPORT void            ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                         const HGF2DPosition& pi_rPoint,
                                                         HGF2DPosition* pi_pFirstContiguousnessPoint,
                                                         HGF2DPosition* pi_pSecondContiguousnessPoint) const override;
    IMAGEPPTEST_EXPORT bool            Crosses(const HGF2DVector& pi_rVector) const override;
    IMAGEPPTEST_EXPORT bool            AreContiguous(const HGF2DVector& pi_rVector) const override;
    IMAGEPPTEST_EXPORT bool            AreAdjacent(const HGF2DVector& pi_rVector) const override;
    IMAGEPPTEST_EXPORT bool            IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                      HGF2DVector::ExtremityProcessing
                                      pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                      double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const override;
    IMAGEPPTEST_EXPORT bool            AreContiguousAt(const HGF2DVector& pi_rVector,
                                            const HGF2DPosition& pi_rPoint) const override;
    IMAGEPPTEST_EXPORT HGFBearing      CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                                             HGF2DVector::ArbitraryDirection
                                             pi_Direction = HGF2DVector::BETA) const override;
    IMAGEPPTEST_EXPORT double          CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                                         HGF2DVector::ArbitraryDirection
                                                         pi_Direction = HGF2DVector::BETA) const override;
    IMAGEPPTEST_EXPORT HGF2DShape::SpatialPosition
                            CalculateSpatialPositionOf(const HGF2DVector& pi_rVector) const override;
    IMAGEPPTEST_EXPORT HGF2DShape::SpatialPosition
                            CalculateSpatialPositionOf(const HGF2DPosition& pi_rPosition) const override;
    
    // From HGFGraphicObject
    IMAGEPPTEST_EXPORT HGF2DLiteExtent GetExtent() const override;
    IMAGEPPTEST_EXPORT void            Move(const HGF2DDisplacement& pi_rDisplacement) override;
    IMAGEPPTEST_EXPORT void            Scale(double pi_ScaleFactor,
                                  const HGF2DPosition& pi_rScaleOrigin) override;
    IMAGEPPTEST_EXPORT void            SetAutoToleranceActive(bool pi_ActiveAutoTolerance) override;
    IMAGEPPTEST_EXPORT void            SetTolerance(double pi_Tolerance) override;
    IMAGEPPTEST_EXPORT void            SetStrokeTolerance(const HFCPtr<HGFLiteTolerance> & pi_Tolerance) override;


    // From HPMPersistentObject
    IMAGEPPTEST_EXPORT HGF2DVector*    Clone() const override;

    // Debugging
    IMAGEPPTEST_EXPORT void            PrintState(ostream& po_rOutput) const override;


    IMAGEPPTEST_EXPORT virtual HFCPtr<HGF2DShape>              AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const override;

    IMAGEPPTEST_EXPORT virtual void                            GetBestOrientedExtent(HGF2DPositionCollection* po_pMinimalBoxCorners,
                                                                  HGF2DPositionCollection* po_pConvexHull) const override;

protected:

private:

    // List of shapes
    ShapeList      m_ShapeList;
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DComplexShape.hpp"
