//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DUniverse
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DSimpleShape.h"
#include "HVE2DVoidShape.h"
#include "HGF2DUniverse.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a simple shape which represents the whole world or
    universe. This shape conceptually represents and unbounded shape. All
    coordinates are located in it. This kind of shape is effective when
    representation of the whole universe is desired. Just like any other
    shape it can operate area operation with other shapes of any kind.
    All shapes are considered IN a universe except for another universe,
    which is considered ON.

    The HVE2DUniverse class is a descendant from HVE2DSimpleShape. This makes
    sense since a universe is composed of one unique component. It follows that
    such a shape can be a component of holed shape or a surface just like
    any other simple shape.

    Since it is a simple shape, a HVE2DUniverse object has the ability to
    produce a complex linear representing its boundary. Being unbounded
    however, the complex linear takes the form of a rectangle for
    which the coordinates are set to -(numeric_limits<double>::infinity()) and
    (numeric_limits<double>::infinity()) for its upper and lower coordinates. Just like any
    other vector, it has an extent. Again the extent is defined but its
    lower and upper coordinates are also set to -(numeric_limits<double>::infinity()) and
    (numeric_limits<double>::infinity()).

    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
class HVE2DUniverse : public HVE2DSimpleShape
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DUniverseId)

public:

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor. These methods permit to
        instantiate a new HVE2DUniverse object. The first one is the default
        constructor. The second one only sets the interpretation coordinate
        system. The third one is the copy constructor. No coordinates is
        needed to define a universe, since its nature fully qualifies
        the object graphic properties.

        @param pi_rpCoordSys Constant reference to a smart pointer to
                             coordinate system that will be used
                             in the interpretation of the shape.

        @param pi_rObject Constant reference to a HVE2DUniverse to duplicate.


        Example:
        @code
        @end

        @see HGF2DCoordSys
        @see HVE2DVoidShape
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT               HVE2DUniverse();
    IMAGEPP_EXPORT               HVE2DUniverse(const HGF2DUniverse& pi_rLightShape, 
                                              const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT               HVE2DUniverse(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT               HVE2DUniverse(const HVE2DUniverse&  pi_rObject);
    IMAGEPP_EXPORT virtual       ~HVE2DUniverse();

    HVE2DUniverse&              operator=(const HVE2DUniverse& pi_rObj);


    // From HVE2DSimpleShape
    IMAGEPP_EXPORT HVE2DComplexLinear
                                GetLinear() const override;
    IMAGEPP_EXPORT HVE2DComplexLinear
                                GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const override;
    IMAGEPP_EXPORT HVE2DComplexLinear*
                                AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const override;


    // Scanlines generation
    void                Rasterize(HGFScanLines& pio_rScanlines) const override;


    bool                IsEmpty     () const override;
    HVE2DShapeTypeId    GetShapeType() const override;
    IMAGEPP_EXPORT HVE2DShape*
                                DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const override;
    HVE2DShape*         DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const override;
    HVE2DShape*         IntersectShapeSCS(const HVE2DShape& pi_rShape) const override;
    HVE2DShape*         UnifyShapeSCS(const HVE2DShape& pi_rShape) const override;
    // PLEASE NOTE THAT THE ABSENCE OF DifferentiateShape Overload IS VOLONTARY
    virtual HVE2DShape*         DifferentiateFromShape(const HVE2DShape& pi_rShape) const;
    HVE2DShape*         IntersectShape(const HVE2DShape& pi_rShape) const override;
    HVE2DShape*         UnifyShape(const HVE2DShape& pi_rShape) const override;
    double              CalculateArea() const override;
    double              CalculatePerimeter() const override;
    IMAGEPP_EXPORT HVE2DShape::SpatialPosition
                                CalculateSpatialPositionOf(const HVE2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT HVE2DShape::SpatialPosition
                                CalculateSpatialPositionOf(const HGF2DLocation& pi_rLocation) const override;
    bool                IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    void                MakeEmpty() override;

    HGF2DShape*         GetLightShape() const override;


    // From HVE2DVector
    HGF2DLocation       CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const override;
    size_t              Intersect(const HVE2DVector& pi_rVector,
                                          HGF2DLocationCollection* po_pCrossPoints) const override;
    size_t              ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                           HGF2DLocationCollection* po_pContiguousnessPoints) const override;
    void                ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                             const HGF2DLocation& pi_rPoint,
                                                             HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                             HGF2DLocation* pi_pSecondContiguousnessPoint) const override;
    HVE2DVector*        AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const override;
//        virtual bool      Touches(const HVE2DVector& pi_rVector) const;
//        virtual bool      Flirts(const HVE2DVector& pi_rVector) const;
    bool                Crosses(const HVE2DVector& pi_rVector) const override;
    bool                AreContiguous(const HVE2DVector& pi_rVector) const override;
    bool                AreAdjacent(const HVE2DVector& pi_rVector) const override;
    bool                IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                          HVE2DVector::ExtremityProcessing
                                          pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                          double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    bool                IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                             HVE2DVector::ExtremityProcessing
                                             pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                             double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    bool                AreContiguousAt(const HVE2DVector& pi_rVector,
                                                const HGF2DLocation& pi_rPoint) const override;
    HGFBearing          CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                                 HVE2DVector::ArbitraryDirection
                                                 pi_Direction = HVE2DVector::BETA) const override;
    double              CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                             HVE2DVector::ArbitraryDirection
                                                             pi_Direction = HVE2DVector::BETA) const override;

    // From HGFGraphicObject
    HGF2DExtent         GetExtent() const override;
    void                Move(const HGF2DDisplacement& pi_rDisplacement) override;
    void                Scale(double pi_ScaleFactor,
                                      const HGF2DLocation& pi_rScaleOrigin) override;
    virtual void                Scale(double pi_ScaleFactorX,
                                      double pi_ScaleFactorY,
                                      const HGF2DLocation& pi_rScaleOrigin);

    // From HPMPersistentObject
    HPMPersistentObject*
                                Clone() const override;

    // Debugging
    IMAGEPP_EXPORT void  PrintState(ostream& po_rOutput) const override;

protected:

private:

    };
END_IMAGEPP_NAMESPACE


#include "HVE2DUniverse.hpp"
