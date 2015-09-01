//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DRectangle.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DRectangle
//-----------------------------------------------------------------------------
#pragma once

#include "HVE2DSimpleShape.h"
#include "HGF2DRectangle.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements the simple shape of type rectangle. A
    rectangle is a simple shape which cannot contain any holes,
    made of 4 vertices parallel by pair, and oriented according the
    X and Y dimension axis of the coordinate system it is expressed
    into. A rectangle is therefore defined either by 2
    locations, or by four values (XMin, XMax, YMin, and YMax). Since
    a rectangle has the same geometric properties as the extent
    object (HGF2DExtent), an extent object can directly be transformed
    into a rectangle shape.

    Rectangles are by far the most performant shape for
    operating shape intersection. For unions and differentiation,
    rectangles must often be converted to polygons of segments before processing.

    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
class HVE2DRectangle : public HVE2DSimpleShape
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DRectangleId)

public:

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor. These methods permit to instantiate
        a new HVE2DRectangle object. The first one is the default constructor.
        The second one only sets the interpretation coordinate system. The
        third one enables specification of rectangle by setting extrema. The
        value of extrema is interpreted in the given coordinate system units.
        The fourth constructor permits definition by specification of origin
        and corner. The coordinate system set for the rectangle is the coordinate
        system of the origin. The fifth allows creation of a rectangle
        from a fence. The sixth one create a rectangle from the given extent.

        @param pi_rpCoordSys Constant reference to a smart pointer to
                             coordinate system that will be used
                             in the interpretation of the shape.

        @param pi_XMin Minimum X of rectangle. This value must be smaller than pi_XMax.

        @param pi_YMin Minimum Y of rectangle. This value must be smaller than pi_YMax.

        @param pi_XMax Maximum X of rectangle. This value must be greater than pi_XMin.

        @param pi_YMax Maximum Y of rectangle. This value must be greater than pi_YMin.

        @param pi_rOrigin Location specifying the origin of rectangle.

        @param pi_rCorner Location specifying the corner of rectangle.

        @param pi_rExtent An extent object from which is constructed a rectangle.

        @param pi_rRectFence Constant reference to a rectangle fence to transform into
                             a rectangle.

        @param pi_rObject Constant reference to a HVE2DRectangle to duplicate.


        Example:
        @code
        @end

        @see HGF2DCoordSys
        @see HGF2DLocation
        @see HGF2DExtent
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT                    HVE2DRectangle();
    IMAGEPP_EXPORT                    HVE2DRectangle(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT                    HVE2DRectangle(double     pi_XMin,
                                             double     pi_YMin,
                                             double     pi_XMax,
                                             double     pi_YMax,
                                             const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT                    HVE2DRectangle(const HGF2DLocation& pi_rFirstPoint,
                                             const HGF2DLocation& pi_rSecondPoint);
    IMAGEPP_EXPORT                    HVE2DRectangle(const HGF2DRectangle& pi_rRectFence,
                                             const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT                    HVE2DRectangle(const HGF2DExtent& pi_rExtent);
    IMAGEPP_EXPORT                    HVE2DRectangle(const HVE2DRectangle&    pi_rObject);
    IMAGEPP_EXPORT virtual            ~HVE2DRectangle();

    HVE2DRectangle&    operator=(const HVE2DRectangle& pi_rObj);

    // Getting and setting
    void               SetRectangle(const HGF2DLocation& pi_rFirstPoint,
                                    const HGF2DLocation& pi_rSecondPoint);
    void               SetRectangle(double pi_XMin,
                                    double pi_YMin,
                                    double pi_XMax,
                                    double pi_YMax);
    void               GetRectangle(HGF2DLocation* pi_pMinPoint,
                                    HGF2DLocation* pi_pMaxPoint) const;
    void               GetRectangle(double* po_pXMin,
                                    double* po_pYMin,
                                    double* po_pXMax,
                                    double* po_pYMax) const;
    bool               Overlaps(const HVE2DShape& pi_rShape) const;

    // Misc
    IMAGEPP_EXPORT virtual void       Scale(double              pi_ScaleFactorX,
                                    double              pi_ScaleFactorY,
                                    const HGF2DLocation& pi_rScaleOrigin);


    // From HVE2DSimpleShape
    IMAGEPP_EXPORT virtual HVE2DComplexLinear     GetLinear() const;
    IMAGEPP_EXPORT virtual HVE2DComplexLinear     GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
    IMAGEPP_EXPORT virtual HVE2DComplexLinear*    AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;

    // From HVE2DShape

    // Scanlines generation
    IMAGEPP_EXPORT virtual void       Rasterize(HGFScanLines& pio_rScanlines) const;

    IMAGEPP_EXPORT virtual bool      IsEmpty     () const;
    IMAGEPP_EXPORT virtual HVE2DShapeTypeId    GetShapeType() const;
    IMAGEPP_EXPORT virtual HVE2DShape*         DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HVE2DShape*         DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HVE2DShape*         IntersectShapeSCS(const HVE2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HVE2DShape*         UnifyShapeSCS(const HVE2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual double              CalculateArea() const;
    IMAGEPP_EXPORT virtual double              CalculatePerimeter() const;
    IMAGEPP_EXPORT virtual bool                IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual void                MakeEmpty();
    IMAGEPP_EXPORT virtual HVE2DShape::SpatialPosition
                                       CalculateSpatialPositionOfSingleComponentVector(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual HVE2DShape::SpatialPosition
                                       CalculateSpatialPositionOfNonCrossingLinear(const HVE2DLinear& pi_rLinear) const;

    IMAGEPP_EXPORT virtual void                Drop(HGF2DLocationCollection* po_pPoint, double       pi_Tolerance) const;

    virtual HGF2DShape*                GetLightShape() const;


    // From HVE2DVector
    IMAGEPP_EXPORT virtual HGF2DLocation       CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    IMAGEPP_EXPORT virtual size_t              Intersect(const HVE2DVector& pi_rVector,
                                                 HGF2DLocationCollection* po_pCrossPoints) const;
    IMAGEPP_EXPORT virtual size_t              ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                                  HGF2DLocationCollection* po_pContiguousnessPoints) const;
    IMAGEPP_EXPORT virtual void                ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                                    const HGF2DLocation& pi_rPoint,
                                                                    HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                                    HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    IMAGEPP_EXPORT virtual HVE2DVector*        AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    IMAGEPP_EXPORT virtual bool                Crosses(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool                AreContiguous(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool                AreAdjacent(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool                IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                                 HVE2DVector::ExtremityProcessing
                                                 pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                                 double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual bool                IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                                    HVE2DVector::ExtremityProcessing
                                                    pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                                    double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual bool                AreContiguousAt(const HVE2DVector& pi_rVector,
                                                       const HGF2DLocation& pi_rPoint) const;
    IMAGEPP_EXPORT virtual HGFBearing          CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                                        HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
    IMAGEPP_EXPORT virtual double              CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                                    HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;

    // From HGFGraphicObject
    IMAGEPP_EXPORT virtual HGF2DExtent         GetExtent() const;
    IMAGEPP_EXPORT virtual void                Move(const HGF2DDisplacement& pi_rDisplacement);
    IMAGEPP_EXPORT virtual void                Scale(double              pi_ScaleFactor,
                                             const HGF2DLocation& pi_rScaleOrigin);

    // From HPMPersistentObject
    IMAGEPP_EXPORT virtual HPMPersistentObject*   Clone() const;


    // Debugging
    IMAGEPP_EXPORT virtual void                PrintState(ostream& po_rOutput) const;

protected:

private:

    // Area oriented operations on rectangle
    virtual HVE2DShape*                DifferentiateRectangleSCS(const HVE2DRectangle& pi_rRectangle) const;
    virtual HVE2DShape*                IntersectRectangleSCS(const HVE2DRectangle& pi_rRectangle) const;
    virtual HVE2DShape*                UnifyRectangleSCS(const HVE2DRectangle& pi_rRectangle) const;

    void ResetTolerance();

    // Member attribute ... the extremes of the rectangle
    double     m_XMin;
    double     m_XMax;
    double     m_YMin;
    double     m_YMax;
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DRectangle.hpp"
