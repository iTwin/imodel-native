//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DSimpleShape.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DSimpleShape
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DShape.h"
#include "HVE2DComplexLinear.h"

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
BEGIN_IMAGEPP_NAMESPACE
class HNOVTABLEINIT HVE2DSimpleShape : public HVE2DShape
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DShapeId_Simple)

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
        a new HVE2DSimpleShape object. The first one is the default
        constructor. The second one only sets the interpretation coordinate system.

        @param pi_rpCoordSys Constant reference to a smart pointer to
                             coordinate system that will be used
                             in the interpretation of the shape.

        @param pi_rObject Constant reference to a shape to duplicate.


        Example:
        @code
        @end

        @see HGF2DCoordSys
        -----------------------------------------------------------------------------
    */
    HVE2DSimpleShape ();
    HVE2DSimpleShape (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DSimpleShape (const HVE2DSimpleShape&   pi_rObject);
    virtual            ~HVE2DSimpleShape();

    HVE2DSimpleShape&  operator=(const HVE2DSimpleShape& pi_rObj);

    // Component linear extraction
    /** -----------------------------------------------------------------------------
        This method returns a complex linear defining the path followed
        by the simple shape. The optional parameter indicates in which
        direction (Clockwise or counterclockwise) the linear must define the path.

        @param pi_DirectionDesired An bool that indicates the direction of
                                   rotation the linear must follow.
                                    Valid values are HVE2DSimpleShape::CW or
                                    HVE2DSimpleShape::CCW.


        @return A complex linear defining the path of the shape boundary.

        Example:
        @code
        @end

        @see AllocateLinear()
        -----------------------------------------------------------------------------
    */
    virtual HVE2DComplexLinear    GetLinear() const = 0;
    virtual HVE2DComplexLinear    GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const = 0;

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
                                   Valid values are HVE2DSimpleShape::CW
                                   or HVE2DSimpleShape::CCW.

        @return A dynamically allocated copy of the linear turning in the
                desired direction defining the path of the shape boundary.
                The linear must be freed when needed no more.

        Example:
        @code
        @end

        @see GetLinear()
        -----------------------------------------------------------------------------
    */
    virtual HVE2DComplexLinear*    AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const = 0;


    // From HVE2DShape
    virtual bool                   IsSimple() const;
    virtual bool                   IsComplex() const;

    virtual const HVE2DShape::ShapeList&
                                   GetShapeList() const;

    // Iteration on component holes
    virtual bool                  HasHoles() const;
    virtual const HVE2DShape::HoleList&
                                  GetHoleList() const;

    IMAGEPP_EXPORT virtual HVE2DShape*    DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HVE2DShape*    DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HVE2DShape*    IntersectShapeSCS(const HVE2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HVE2DShape*    UnifyShapeSCS(const HVE2DShape& pi_rShape) const;

    virtual void                  Drop(HGF2DLocationCollection* po_pPoint,
                                       double                   pi_Tolerance) const;

    // Debugging
    IMAGEPP_EXPORT virtual void           PrintState(ostream& po_rOutput) const;

protected:

private:

    enum DecomposeOperation
        {
        DIFF,
        DIFFFROM,
        UNION,
        INTERSECT
        };


    HVE2DShape*             UnifyCrossingSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape, const HGF2DLocationCollection& pi_rPoints) const;
    HVE2DShape*             UnifySimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const;
    HVE2DShape*             IntersectCrossingSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape, const HGF2DLocationCollection& pi_rPoints) const;
    HVE2DShape*             IntersectSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const;
    HVE2DShape*             DifferentiateCrossingSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape, const HGF2DLocationCollection& pi_rPoints) const;
    HVE2DShape*             DifferentiateSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const;
    HVE2DShape*             DifferentiateFromCrossingSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape, const HGF2DLocationCollection& pi_rPoints) const;
    HVE2DShape*             DifferentiateFromSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const;

    void                    DecomposeSCS(const HVE2DSimpleShape& pi_rSimpleShape,
                                         HVE2DSimpleShape::DecomposeOperation pi_Operation,
                                         HoleList& pi_rListOfShapes,
                                         const HGF2DLocationCollection& pi_rPoints) const;

    void                    SuperScanSCS(const HVE2DSimpleShape& pi_rSimpleShape,
                                         bool                   pi_WantInPtsOfPoly1,
                                         bool                   pi_ScanPoly1CW,
                                         bool                   pi_WantInPtsOfPoly2,
                                         bool                   pi_ScanPoly2CW,
                                         HoleList&               pi_rListOfShapes,
                                         const HGF2DLocationCollection& pi_rPoints) const;


    };
END_IMAGEPP_NAMESPACE

#include "HVE2DSimpleShape.hpp"
