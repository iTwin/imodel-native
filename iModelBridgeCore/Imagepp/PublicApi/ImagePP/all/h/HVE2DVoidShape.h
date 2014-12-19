//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DVoidShape.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HVE2DVoidShape
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DSimpleShape.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a simple shape which represents no area. It has
    no position, its extent is undefined. This kind of shape is very useful
    to represent results of area oriented operation, when there is no area
    defined.

    Just like any other shape it can operate area operation with other shapes
    of any kind. All shapes are considered out a void shape except for another
    void shape, which is considered ON.

    The HVE2DVoidShape class is a descendant from HVE2DSimpleShape. This makes
    sense since a void shape is composed of one unique component. It follows
    that such a shape can be a component of holed shape or a surface just like
    any other simple shape.

    Since it is a simple shape, a HVE2DVoidShape object has the ability to
    produce a complex linear representing its boundary. Being void, the
    complex linear returned is empty.

    -----------------------------------------------------------------------------
*/
class HVE2DVoidShape : public HVE2DSimpleShape
    {

    HPM_DECLARE_CLASS_DLL(_HDLLg,  1118)

public:

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor. These methods permit to instantiate
        a new HVE2DVoidShape object. The first one is the default constructor.
        The second one only sets the interpretation coordinate system. The
        third one is the copy constructor. No coordinates is needed to define
        a void shape, since its nature fully qualifies the object
        graphic properties.

        @param pi_rpCoordSys Constant reference to a smart pointer to
                             coordinate system that will be used
                             in the interpretation of the shape.


        @param pi_rObject Constant reference to a HVE2DVoidShape to duplicate.


        Example:
        @code
        @end

        @see HGF2DCoordSys
        @see HGF2DLocation
        -----------------------------------------------------------------------------
    */
    _HDLLg HVE2DVoidShape ();
    _HDLLg HVE2DVoidShape (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    _HDLLg HVE2DVoidShape (const HVE2DVoidShape&   pi_rObject);
    _HDLLg virtual            ~HVE2DVoidShape();

    HVE2DVoidShape&    operator=(const HVE2DVoidShape& pi_rObj);


    // From HGF2DSimpleShape
    virtual HVE2DComplexLinear
    GetLinear() const;
    virtual HVE2DComplexLinear
    GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
    virtual HVE2DComplexLinear*
    AllocateLinear(
        HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;

    // Scanlines generation
    _HDLLg virtual void       Rasterize(HGFScanLines& pio_rScanlines) const;


    virtual bool      IsEmpty     () const;
    virtual HVE2DShapeTypeId
    GetShapeType() const;
    virtual HVE2DShape*
    DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*
    DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*
    IntersectShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*
    UnifyShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*
    DifferentiateShape(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*
    DifferentiateFromShape(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*
    IntersectShape(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*
    UnifyShape(const HVE2DShape& pi_rShape) const;

    virtual double    CalculateArea() const;
    virtual double    CalculatePerimeter() const;
    _HDLLg virtual HVE2DShape::SpatialPosition
    CalculateSpatialPositionOf(const HVE2DVector& pi_rVector) const;
    _HDLLg virtual HVE2DShape::SpatialPosition
    CalculateSpatialPositionOf(const HGF2DLocation& pi_rLocation) const;
    virtual bool      IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual void       MakeEmpty();

    virtual HGF2DShape*                GetLightShape() const;


    // From HVE2DVector
    virtual HGF2DLocation
    CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    virtual size_t     Intersect(const HVE2DVector& pi_rVector,
                                 HGF2DLocationCollection* po_pCrossPoints) const;
    virtual size_t     ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                  HGF2DLocationCollection* po_pContiguousnessPoints) const;
    virtual void       ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                    const HGF2DLocation& pi_rPoint,
                                                    HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                    HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    virtual HVE2DVector*
    AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    virtual bool      Crosses(const HVE2DVector& pi_rVector) const;
    virtual bool      AreContiguous(const HVE2DVector& pi_rVector) const;
    virtual bool      AreAdjacent(const HVE2DVector& pi_rVector) const;
    virtual bool      IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                 HVE2DVector::ExtremityProcessing
                                 pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                 double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool      IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                    HVE2DVector::ExtremityProcessing
                                    pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                    double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool      AreContiguousAt(const HVE2DVector& pi_rVector,
                                       const HGF2DLocation& pi_rPoint) const;
    virtual HGFBearing
    CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                     HVE2DVector::ArbitraryDirection
                     pi_Direction = HVE2DVector::BETA) const;
    virtual double     CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                 HVE2DVector::ArbitraryDirection
                                 pi_Direction = HVE2DVector::BETA) const;

    virtual void       Drop(HGF2DLocationCollection* po_pPoint,
                            double                   pi_Tolerance) const;

    // From HGFGraphicObject
    virtual HGF2DExtent
    GetExtent() const;
    virtual void       Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void       Scale(double pi_ScaleFactor,
                             const HGF2DLocation& pi_rScaleOrigin);
    virtual void       Scale(double pi_ScaleFactorX,
                             double pi_ScaleFactorY,
                             const HGF2DLocation& pi_rScaleOrigin);

    // From HPMPersistentObject
    virtual HPMPersistentObject*
    Clone() const;

    // Debugging
    _HDLLg virtual void       PrintState(ostream& po_rOutput) const;

protected:

private:

    };


#include "HVE2DVoidShape.hpp"

