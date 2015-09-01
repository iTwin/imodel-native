//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DComplexShape.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    HGF2DComplexShape&      operator=(const HGF2DComplexShape& pi_rObj);

    // Setting
    void                    AddShape(const HGF2DShape& pi_rShape);

    // From HGF2DShape

    virtual void            Rasterize(HGFScanLines& pio_rScanlines) const;

    virtual bool            IsSimple() const;
    virtual bool            IsComplex() const;
    virtual HGF2DShapeTypeId
                            GetShapeType() const;
    virtual bool            IsEmpty () const;
    virtual const HGF2DShape::ShapeList&
                            GetShapeList() const;

    virtual bool            HasHoles() const;
    virtual const HGF2DShape::HoleList&
                            GetHoleList() const;

    virtual HGF2DShape*     DifferentiateFromShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*     DifferentiateShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*     IntersectShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*     UnifyShape(const HGF2DShape& pi_rShape) const;
    virtual double          CalculateArea() const;
    virtual double          CalculatePerimeter() const;
    virtual bool            IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    void                    MakeEmpty();
    virtual HGF2DShape::SpatialPosition  
                            CalculateSpatialPositionOfSingleComponentVector(const HGF2DVector& pi_rVector) const;

    virtual void            Drop(HGF2DPositionCollection* po_pPoint,
                                 double                   pi_rTolerance) const;


    // From HGF2DVector
    virtual HGF2DPosition   CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
    virtual size_t          Intersect(const HGF2DVector& pi_rVector,
                                      HGF2DPositionCollection* po_pCrossPoints) const;
    virtual size_t          ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                       HGF2DPositionCollection* po_pContiguousnessPoints) const;
    virtual void            ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                         const HGF2DPosition& pi_rPoint,
                                                         HGF2DPosition* pi_pFirstContiguousnessPoint,
                                                         HGF2DPosition* pi_pSecondContiguousnessPoint) const;
    virtual bool            Crosses(const HGF2DVector& pi_rVector) const;
    virtual bool            AreContiguous(const HGF2DVector& pi_rVector) const;
    virtual bool            AreAdjacent(const HGF2DVector& pi_rVector) const;
    virtual bool            IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                      HGF2DVector::ExtremityProcessing
                                      pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                      double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    virtual bool            AreContiguousAt(const HGF2DVector& pi_rVector,
                                            const HGF2DPosition& pi_rPoint) const;
    virtual HGFBearing      CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                                             HGF2DVector::ArbitraryDirection
                                             pi_Direction = HGF2DVector::BETA) const;
    virtual double          CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                                         HGF2DVector::ArbitraryDirection
                                                         pi_Direction = HGF2DVector::BETA) const;
    virtual HGF2DShape::SpatialPosition
                            CalculateSpatialPositionOf(const HGF2DVector& pi_rVector) const;
    virtual HGF2DShape::SpatialPosition
                            CalculateSpatialPositionOf(const HGF2DPosition& pi_rPosition) const;
    
    // From HGFGraphicObject
    virtual HGF2DLiteExtent GetExtent() const;
    virtual void            Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void            Scale(double pi_ScaleFactor,
                                  const HGF2DPosition& pi_rScaleOrigin);
    virtual void            SetAutoToleranceActive(bool pi_ActiveAutoTolerance);
    virtual void            SetTolerance(double pi_Tolerance);
    virtual void            SetStrokeTolerance(const HFCPtr<HGFLiteTolerance> & pi_Tolerance);


    // From HPMPersistentObject
    virtual HGF2DVector*    Clone() const;

    // Debugging
    virtual void            PrintState(ostream& po_rOutput) const;


    virtual HFCPtr<HGF2DShape>              AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const override;

protected:

private:

    // List of shapes
    ShapeList      m_ShapeList;
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DComplexShape.hpp"
