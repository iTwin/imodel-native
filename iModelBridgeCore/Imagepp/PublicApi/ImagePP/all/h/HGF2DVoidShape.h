//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DVoidShape.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DVoidShape
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DSimpleShape.h"

BEGIN_IMAGEPP_NAMESPACE

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

    The HGF2DVoidShape class is a descendant from HGF2DSimpleShape. This makes
    sense since a void shape is composed of one unique component. It follows
    that such a shape can be a component of holed shape or a surface just like
    any other simple shape.

    Since it is a simple shape, a HGF2DVoidShape object has the ability to
    produce a complex linear representing its boundary. Being void, the
    complex linear returned is empty.

    -----------------------------------------------------------------------------
*/
class HGF2DVoidShape : public HGF2DSimpleShape
    {

    HDECLARE_CLASS_ID(HGF2DShapeId_Void, HGF2DSimpleShape)

public:

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor. These methods permit to instantiate
        a new HGF2DVoidShape object. The first one is the default constructor.
        The second one only sets the interpretation coordinate system. The
        third one is the copy constructor. No coordinates is needed to define
        a void shape, since its nature fully qualifies the object
        graphic properties.

        @param pi_rpCoordSys Constant reference to a smart pointer to
                             coordinate system that will be used
                             in the interpretation of the shape.


        @param pi_rObject Constant reference to a HGF2DVoidShape to duplicate.


        Example:
        @code
        @end

        @see HGF2DPosition
        -----------------------------------------------------------------------------
    */
    HGF2DVoidShape ();
    HGF2DVoidShape (const HGF2DVoidShape&   pi_rObject);
    virtual            ~HGF2DVoidShape();

    HGF2DVoidShape&                 operator=(const HGF2DVoidShape& pi_rObj);


    // From HGF2DSimpleShape
    virtual HFCPtr<HGF2DLinear>     GetLinear() const;
    virtual HFCPtr<HGF2DLinear>     GetLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const;

    // Scanlines generation
    IMAGEPP_EXPORT virtual void      Rasterize(HGFScanLines& pio_rScanlines) const;


    virtual bool                    IsEmpty     () const;
    virtual HGF2DShapeTypeId        GetShapeType() const;
    virtual HGF2DShape*             DifferentiateShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*             DifferentiateFromShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*             IntersectShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*             UnifyShape(const HGF2DShape& pi_rShape) const;

    virtual double                  CalculateArea() const;
    virtual double                  CalculatePerimeter() const;
    IMAGEPP_EXPORT virtual HGF2DShape::SpatialPosition     
                                    CalculateSpatialPositionOf(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual HGF2DShape::SpatialPosition     
                                    CalculateSpatialPositionOf(const HGF2DPosition& pi_rLocation) const;
    virtual bool                    IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    virtual void                    MakeEmpty();

    // From HGF2DVector
    virtual HGF2DPosition           CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
    virtual size_t                  Intersect(const HGF2DVector& pi_rVector,
                                              HGF2DPositionCollection* po_pCrossPoints) const;
    virtual size_t                  ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                               HGF2DPositionCollection* po_pContiguousnessPoints) const;
    virtual void                    ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                                 const HGF2DPosition& pi_rPoint,
                                                                 HGF2DPosition* pi_pFirstContiguousnessPoint,
                                                                 HGF2DPosition* pi_pSecondContiguousnessPoint) const;
    virtual bool                    Crosses(const HGF2DVector& pi_rVector) const;
    virtual bool                    AreContiguous(const HGF2DVector& pi_rVector) const;
    virtual bool                    AreAdjacent(const HGF2DVector& pi_rVector) const;
    virtual bool                    IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                              HGF2DVector::ExtremityProcessing
                                              pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                              double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    virtual bool                    AreContiguousAt(const HGF2DVector& pi_rVector,
                                                    const HGF2DPosition& pi_rPoint) const;
    virtual HGFBearing              CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                                                     HGF2DVector::ArbitraryDirection
                                                     pi_Direction = HGF2DVector::BETA) const;
    virtual double                  CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                                                 HGF2DVector::ArbitraryDirection
                                                                 pi_Direction = HGF2DVector::BETA) const;

    virtual void                    Drop(HGF2DPositionCollection* po_pPoint,
                                         double                   pi_rTolerance) const;
    // From HGF2DVector
    virtual HGF2DLiteExtent         GetExtent() const;
    virtual void                    Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void                    Scale(double pi_ScaleFactor,
                                          const HGF2DPosition& pi_rScaleOrigin);
    virtual void                    Scale(double pi_ScaleFactorX,
                                          double pi_ScaleFactorY,
                                          const HGF2DPosition& pi_rScaleOrigin);


    virtual HGF2DVector*            Clone() const;

    // Debugging
    IMAGEPP_EXPORT virtual void      PrintState(ostream& po_rOutput) const;


    virtual HFCPtr<HGF2DShape>      AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const override;
    virtual HFCPtr<HGF2DShape>      AllocTransformInverse(const HGF2DTransfoModel& pi_rModel) const override;

protected:

private:

    };

END_IMAGEPP_NAMESPACE
#include "HGF2DVoidShape.hpp"

