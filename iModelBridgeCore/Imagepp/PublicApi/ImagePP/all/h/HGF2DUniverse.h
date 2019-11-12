//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DUniverse
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DSimpleShape.h"
#include "HGF2DVoidShape.h"
BEGIN_IMAGEPP_NAMESPACE

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

    The HGF2DUniverse class is a descendant from HGF2DSimpleShape. This makes
    sense since a universe is composed of one unique component. It follows that
    such a shape can be a component of holed shape or a surface just like
    any other simple shape.

    Since it is a simple shape, a HGF2DUniverse object has the ability to
    produce a complex linear representing its boundary. Being unbounded
    however, the complex linear takes the form of a rectangle for
    which the coordinates are set to -(numeric_limits<double>::infinity()) and
    (numeric_limits<double>::infinity()) for its upper and lower coordinates. Just like any
    other vector, it has an extent. Again the extent is defined but its
    lower and upper coordinates are also set to -(numeric_limits<double>::infinity()) and
    (numeric_limits<double>::infinity()).

    -----------------------------------------------------------------------------
*/
class HGF2DUniverse : public HGF2DSimpleShape
    {

    HDECLARE_CLASS_ID(HGF2DUniverseId, HGF2DSimpleShape)

public:

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor. These methods permit to
        instantiate a new HGF2DUniverse object. The first one is the default
        constructor. The second one only sets the interpretation coordinate
        system. The third one is the copy constructor. No coordinates is
        needed to define a universe, since its nature fully qualifies
        the object graphic properties.

        @param pi_rpCoordSys Constant reference to a smart pointer to
                             coordinate system that will be used
                             in the interpretation of the shape.

        @param pi_rObject Constant reference to a HGF2DUniverse to duplicate.


        Example:
        @code
        @end

        @see HGF2DVoidShape
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT             HGF2DUniverse();
    IMAGEPP_EXPORT             HGF2DUniverse(const HGF2DUniverse&  pi_rObject);
    IMAGEPP_EXPORT virtual    ~HGF2DUniverse();

    HGF2DUniverse&     operator=(const HGF2DUniverse& pi_rObj);


    // From HGF2DSimpleShape
    IMAGEPP_EXPORT HFCPtr<HGF2DLinear>    GetLinear() const override;
    IMAGEPP_EXPORT HFCPtr<HGF2DLinear>    GetLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const override;

    // Scanlines generation
    void       Rasterize(HGFScanLines& pio_rScanlines) const override;


    bool      IsEmpty     () const override;
    HGF2DShapeTypeId     GetShapeType() const override;
    IMAGEPP_EXPORT HGF2DShape*   DifferentiateShape(const HGF2DShape& pi_rShape) const override;
    HGF2DShape*          DifferentiateFromShape(const HGF2DShape& pi_rShape) const override;
    HGF2DShape*          IntersectShape(const HGF2DShape& pi_rShape) const override;
    HGF2DShape*          UnifyShape(const HGF2DShape& pi_rShape) const override;
    // PLEASE NOTE THAT THE ABSENCE OF DifferentiateShape Overload IS VOLONTARY
    double             CalculateArea() const override;
    double             CalculatePerimeter() const override;
    IMAGEPP_EXPORT HGF2DShape::SpatialPosition     CalculateSpatialPositionOf(const HGF2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT HGF2DShape::SpatialPosition     CalculateSpatialPositionOf(const HGF2DPosition& pi_rLocation) const override;
    bool      IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const override;
    void       MakeEmpty() override;

    // From HGF2DVector
    HGF2DPosition       CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const override;
    size_t     Intersect(const HGF2DVector& pi_rVector,
                                 HGF2DPositionCollection* po_pCrossPoints) const override;
    size_t     ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                  HGF2DPositionCollection* po_pContiguousnessPoints) const override;
    void       ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                    const HGF2DPosition& pi_rPoint,
                                                    HGF2DPosition* pi_pFirstContiguousnessPoint,
                                                    HGF2DPosition* pi_pSecondContiguousnessPoint) const override;
    bool      Crosses(const HGF2DVector& pi_rVector) const override;
    bool      AreContiguous(const HGF2DVector& pi_rVector) const override;
    bool      AreAdjacent(const HGF2DVector& pi_rVector) const override;
    bool      IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                 HGF2DVector::ExtremityProcessing
                                 pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                 double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const override;
    bool      AreContiguousAt(const HGF2DVector& pi_rVector,
                                       const HGF2DPosition& pi_rPoint) const override;
    HGFBearing    CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                     HGF2DVector::ArbitraryDirection
                     pi_Direction = HGF2DVector::BETA) const override;
    double     CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                 HGF2DVector::ArbitraryDirection
                                 pi_Direction = HGF2DVector::BETA) const override;

    HGF2DLiteExtent    GetExtent() const override;
    void       Move(const HGF2DDisplacement& pi_rDisplacement) override;
    void       Scale(double pi_ScaleFactor,
                             const HGF2DPosition& pi_rScaleOrigin) override;
    virtual void       Scale(double pi_ScaleFactorX,
                             double pi_ScaleFactorY,
                             const HGF2DPosition& pi_rScaleOrigin);

    HGF2DVector*    Clone() const override;

    // Debugging
    IMAGEPP_EXPORT void       PrintState(ostream& po_rOutput) const override;


    virtual HFCPtr<HGF2DShape>              AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const override;
    virtual HFCPtr<HGF2DShape>              AllocTransformInverse(const HGF2DTransfoModel& pi_rModel) const override;

protected:

private:

    };

END_IMAGEPP_NAMESPACE
#include "HGF2DUniverse.hpp"
