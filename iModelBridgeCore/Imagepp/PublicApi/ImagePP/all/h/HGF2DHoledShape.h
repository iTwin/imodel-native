//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DHoledShape.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DShape.h"
#include "HGF2DSimpleShape.h"
#include "HGF2DDisplacement.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class implements the generic holed shape. An holed shape is a
    simple shape to which is removed parts of the internal area by use of
    other disjoint or flirting simple shapes positioned inside the area of
    the outter simple shape.
    Any type of simple shape (HGF2DSimpleShape) can define the outter shape
    or the removed parts (holes).

    As mentionned, it is possible for holes to flirt either another
    hole (as represented) or the outter shape. They must not intersect
    their boundaries, nr be contiguous however. This rule permits representation
    of any holed shape imaginable in the 2D space.

    -----------------------------------------------------------------------------
*/
class HGF2DHoledShape : public HGF2DShape
    {

    HDECLARE_CLASS_ID(HGF2DShapeId_Holed, HGF2DShape)

public:

    // Primary methods
                                HGF2DHoledShape();
                                HGF2DHoledShape(const HGF2DSimpleShape& pi_rSimpleShape);
                                HGF2DHoledShape(const HGF2DHoledShape&   pi_rObject);
    virtual                     ~HGF2DHoledShape();

    HGF2DHoledShape&            operator=(const HGF2DHoledShape& pi_rObj);

    // Setting and management
    void                        AddHole(const HGF2DSimpleShape& pi_rSimpleShape);
    virtual void                SetBaseShape(const HGF2DSimpleShape& pi_rComplex);
    virtual const HGF2DSimpleShape&
                                GetBaseShape() const;

    // From HGF2DShape

    virtual void                Rasterize(HGFScanLines& pio_rScanlines) const;

    virtual bool                IsSimple() const;
    virtual bool                IsComplex() const;
    virtual bool                IsEmpty() const;
    virtual HGF2DShapeTypeId    GetShapeType() const;

    virtual const HGF2DShape::ShapeList&
                                GetShapeList() const;
    virtual bool                HasHoles() const;
    virtual const HGF2DShape::HoleList&
                                GetHoleList() const;
    virtual HGF2DShape*         DifferentiateShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*         DifferentiateFromShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*         IntersectShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*         UnifyShape(const HGF2DShape& pi_rShape) const;

    virtual double              CalculateArea() const;
    virtual double              CalculatePerimeter() const;
    virtual bool                IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    void                        MakeEmpty();
    virtual HGF2DShape::SpatialPosition
                                CalculateSpatialPositionOfSingleComponentVector(const HGF2DVector& pi_rVector) const;

    virtual void                Drop(HGF2DPositionCollection* po_pPoint,
                                     double                   pi_rTolerance) const;


    // From HGF2DVector
    virtual HGF2DPosition       CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
    virtual size_t              Intersect(const HGF2DVector& pi_rVector,
                                          HGF2DPositionCollection* po_pCrossPoints) const;
    virtual size_t              ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                           HGF2DPositionCollection* po_pContiguousnessPoints) const;
    virtual void                ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                             const HGF2DPosition& pi_rPoint,
                                                             HGF2DPosition* pi_pFirstContiguousnessPoint,
                                                             HGF2DPosition* pi_pSecondContiguousnessPoint) const;
    virtual HGF2DVector*        Clone() const;
    virtual bool                Crosses(const HGF2DVector& pi_rVector) const;
    virtual bool                AreContiguous(const HGF2DVector& pi_rVector) const;
    virtual bool                AreAdjacent(const HGF2DVector& pi_rVector) const;
    virtual bool                IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                          HGF2DVector::ExtremityProcessing
                                          pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                          double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    virtual bool                AreContiguousAt(const HGF2DVector& pi_rVector,
                                                const HGF2DPosition& pi_rPoint) const;
    virtual HGFBearing          CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                                                 HGF2DVector::ArbitraryDirection
                                                 pi_Direction = HGF2DVector::BETA) const;
    virtual double              CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                                             HGF2DVector::ArbitraryDirection
                                                             pi_Direction = HGF2DVector::BETA) const;


    // From HGF2DVector
    virtual HGF2DLiteExtent     GetExtent() const;
    virtual void                Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void                Scale(double pi_ScaleFactor,
                                      const HGF2DPosition& pi_rScaleOrigin);

    virtual void                SetAutoToleranceActive(bool pi_ActiveAutoTolerance);
    virtual void                SetTolerance(double pi_Tolerance);
    virtual void                SetStrokeTolerance(const HFCPtr<HGFLiteTolerance> & pi_Tolerance);

    virtual void                PrintState(ostream& po_rOutput) const;


    virtual HFCPtr<HGF2DShape>  AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const override;

protected:

private:

    HGF2DShape*        DifferentiateFromSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const;
    HGF2DShape*        DifferentiateFromHoledShape(const HGF2DHoledShape& pi_rHoledShape) const;
    HGF2DShape*        DifferentiateSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const;
    HGF2DShape*        DifferentiateHoledShape(const HGF2DHoledShape& pi_rHoledShape) const;
    HGF2DShape*        IntersectSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const;
    HGF2DShape*        IntersectHoledShape(const HGF2DHoledShape& pi_rHoledShape) const;
    HGF2DShape*        UnifySimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const;
    HGF2DShape*        UnifyHoledShape(const HGF2DHoledShape& pi_rHoledShape) const;

    // Base shape
    HFCPtr<HGF2DSimpleShape> m_pBaseShape;

    // List of holes
    HoleList           m_HoleList;
    };

END_IMAGEPP_NAMESPACE

#include "HGF2DHoledShape.hpp"
