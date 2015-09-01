//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DUniverse.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    IMAGEPP_EXPORT virtual HVE2DComplexLinear
                                GetLinear() const;
    IMAGEPP_EXPORT virtual HVE2DComplexLinear
                                GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
    IMAGEPP_EXPORT virtual HVE2DComplexLinear*
                                AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;


    // Scanlines generation
    virtual void                Rasterize(HGFScanLines& pio_rScanlines) const;


    virtual bool                IsEmpty     () const;
    virtual HVE2DShapeTypeId    GetShapeType() const;
    IMAGEPP_EXPORT virtual HVE2DShape*
                                DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*         DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*         IntersectShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*         UnifyShapeSCS(const HVE2DShape& pi_rShape) const;
    // PLEASE NOTE THAT THE ABSENCE OF DifferentiateShape Overload IS VOLONTARY
    virtual HVE2DShape*         DifferentiateFromShape(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*         IntersectShape(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*         UnifyShape(const HVE2DShape& pi_rShape) const;
    virtual double              CalculateArea() const;
    virtual double              CalculatePerimeter() const;
    IMAGEPP_EXPORT virtual HVE2DShape::SpatialPosition
                                CalculateSpatialPositionOf(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual HVE2DShape::SpatialPosition
                                CalculateSpatialPositionOf(const HGF2DLocation& pi_rLocation) const;
    virtual bool                IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual void                MakeEmpty();

    virtual HGF2DShape*         GetLightShape() const;


    // From HVE2DVector
    virtual HGF2DLocation       CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    virtual size_t              Intersect(const HVE2DVector& pi_rVector,
                                          HGF2DLocationCollection* po_pCrossPoints) const;
    virtual size_t              ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                           HGF2DLocationCollection* po_pContiguousnessPoints) const;
    virtual void                ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                             const HGF2DLocation& pi_rPoint,
                                                             HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                             HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    virtual HVE2DVector*        AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
//        virtual bool      Touches(const HVE2DVector& pi_rVector) const;
//        virtual bool      Flirts(const HVE2DVector& pi_rVector) const;
    virtual bool                Crosses(const HVE2DVector& pi_rVector) const;
    virtual bool                AreContiguous(const HVE2DVector& pi_rVector) const;
    virtual bool                AreAdjacent(const HVE2DVector& pi_rVector) const;
    virtual bool                IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                          HVE2DVector::ExtremityProcessing
                                          pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                          double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool                IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                             HVE2DVector::ExtremityProcessing
                                             pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                             double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool                AreContiguousAt(const HVE2DVector& pi_rVector,
                                                const HGF2DLocation& pi_rPoint) const;
    virtual HGFBearing          CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                                 HVE2DVector::ArbitraryDirection
                                                 pi_Direction = HVE2DVector::BETA) const;
    virtual double              CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                             HVE2DVector::ArbitraryDirection
                                                             pi_Direction = HVE2DVector::BETA) const;

    // From HGFGraphicObject
    virtual HGF2DExtent         GetExtent() const;
    virtual void                Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void                Scale(double pi_ScaleFactor,
                                      const HGF2DLocation& pi_rScaleOrigin);
    virtual void                Scale(double pi_ScaleFactorX,
                                      double pi_ScaleFactorY,
                                      const HGF2DLocation& pi_rScaleOrigin);

    // From HPMPersistentObject
    virtual HPMPersistentObject*
                                Clone() const;

    // Debugging
    IMAGEPP_EXPORT virtual void  PrintState(ostream& po_rOutput) const;

protected:

private:

    };
END_IMAGEPP_NAMESPACE


#include "HVE2DUniverse.hpp"
