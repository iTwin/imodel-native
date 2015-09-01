//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DSimpleShape.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DSimpleShape
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DShape.h"
#include "HGF2DLinear.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This abstract class encapsulates all attributes common to simple shapes.
    Simple shapes are defined as shapes that enclose a single area and do
    not contain holes. A rectangle, a circle are examples of simple shapes.

    The present class does not introduce many new methods, but however implements
    the behavior common to all simple shapes. Notably it implements the
    default behavior required for unification, intersection and differentiation
    of shapes which are applicable and need not be overridden by descending
    classes other than for performance reason, and simplification of the result
    shape.

    All the new methods are the methods required for implementing this behavior.
    These methods permit to obtain a linear defining the path of the simple
    shape into space.

    -----------------------------------------------------------------------------
*/
class HNOVTABLEINIT HGF2DSimpleShape : public HGF2DShape
    {

    HDECLARE_CLASS_ID(HGF2DShapeId_Simple, HGF2DShape)

public:

    // Return value for the rotation direction of simple shape
    enum RotationDirection
        {
        CW,
        CCW
        };

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor. These methods permit to instantiate
        a new HGF2DSimpleShape object. The first one is the default
        constructor. The second one only sets the interpretation coordinate system.

        @param pi_rObject Constant reference to a shape to duplicate.


        Example:
        @code
        @end

        -----------------------------------------------------------------------------
    */
                                    HGF2DSimpleShape ();
                                    HGF2DSimpleShape (const HGF2DSimpleShape&   pi_rObject);
    virtual                         ~HGF2DSimpleShape();

    HGF2DSimpleShape&               operator=(const HGF2DSimpleShape& pi_rObj);

    // Component linear extraction
    /** -----------------------------------------------------------------------------
        This method returns a complex linear defining the path followed
        by the simple shape. The optional parameter indicates in which
        direction (Clockwise or counterclockwise) the linear must define the path.

        @param pi_DirectionDesired An bool that indicates the direction of
                                   rotation the linear must follow.
                                    Valid values are HGF2DSimpleShape::CW or
                                    HGF2DSimpleShape::CCW.


        @return A complex linear defining the path of the shape boundary.

        Example:
        @code
        @end

        @see AllocateLinear()
        -----------------------------------------------------------------------------
    */
    virtual HFCPtr<HGF2DLinear>     GetLinear() const = 0;
    virtual HFCPtr<HGF2DLinear>     GetLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const = 0;

    /** -----------------------------------------------------------------------------
        This method returns a dynamically allocated complex linear defining the
        path followed by the simple shape. The parameter indicates in
        which direction (Clockwise or counterclockwise) the linear must define the
        path. When the linear is not needed anymore it must be deleted by the caller.
        This method was only implemented for performance reason to prevent the
        duplication of the complex linear during return of the linear by
        GetLinear() methods.

        @param pi_DirectionDesired An bool that indicates the direction of
                                   rotation the linear allocated must follow.
                                   Valid values are HGF2DSimpleShape::CW
                                   or HGF2DSimpleShape::CCW.

        @return A dynamically allocated copy of the linear turning in the
                desired direction defining the path of the shape boundary.
                The linear must be freed when needed no more.

        Example:
        @code
        @end

        @see GetLinear()
        -----------------------------------------------------------------------------
    */
//    virtual HVE2DComplexLinear*    AllocateLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const = 0;


    // From HGF2DShape
    virtual bool                    IsSimple() const;
    virtual bool                    IsComplex() const;

    virtual const HGF2DShape::ShapeList&
                                    GetShapeList() const;

    // Iteration on component holes
    virtual bool                    HasHoles() const;
    virtual const HGF2DShape::HoleList&
                                    GetHoleList() const;

    virtual HGF2DShape*             DifferentiateFromShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*             DifferentiateShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*             IntersectShape(const HGF2DShape& pi_rShape) const;
    virtual HGF2DShape*             UnifyShape(const HGF2DShape& pi_rShape) const;

    virtual void                    Drop(HGF2DPositionCollection* po_pPoint,
                                         double                   pi_rTolerance) const;

    // Debugging
    virtual void                    PrintState(ostream& po_rOutput) const;

protected:

private:

    enum DecomposeOperation
        {
        DIFF,
        DIFFFROM,
        UNION,
        INTERSECT
        };

#if (0)
    HGF2DShape*             UnifyCrossingSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape, const HGF2DPositionCollection& pi_rPoints) const;
    HGF2DShape*             UnifySimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const;
    HGF2DShape*             IntersectCrossingSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape, const HGF2DPositionCollection& pi_rPoints) const;
    HGF2DShape*             IntersectSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const;
    HGF2DShape*             DifferentiateCrossingSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape, const HGF2DPositionCollection& pi_rPoints) const;
    HGF2DShape*             DifferentiateSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const;
    HGF2DShape*             DifferentiateFromCrossingSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape, const HGF2DPositionCollection& pi_rPoints) const;
    HGF2DShape*             DifferentiateFromSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const;

#endif
#if (0)

    void                    Decompose(const HGF2DSimpleShape& pi_rSimpleShape,
                                      HGF2DSimpleShape::DecomposeOperation pi_Operation,
                                      HoleList& pi_rListOfShapes,
                                      const HGF2DPositionCollection& pi_rPoints) const;

    void                    SuperScan(const HGF2DSimpleShape& pi_rSimpleShape,
                                      bool                   pi_WantInPtsOfPoly1,
                                      bool                   pi_ScanPoly1CW,
                                      bool                   pi_WantInPtsOfPoly2,
                                      bool                   pi_ScanPoly2CW,
                                      HoleList&               pi_rListOfShapes,
                                      const HGF2DPositionCollection& pi_rPoints) const;

#endif
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DSimpleShape.hpp"
