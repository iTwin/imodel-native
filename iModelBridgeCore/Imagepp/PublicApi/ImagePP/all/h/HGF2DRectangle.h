//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DRectangle.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DRectangle
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DSimpleShape.h"

BEGIN_IMAGEPP_NAMESPACE
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
class HGF2DRectangle : public HGF2DSimpleShape
    {

    HDECLARE_CLASS_ID(HGF2DRectangleId, HGF2DSimpleShape)

public:

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor. These methods permit to instantiate
        a new HGF2DRectangle object. The first one is the default constructor.
        The second one only sets the interpretation coordinate system. The
        third one enables specification of rectangle by setting extrema. The
        value of extrema is interpreted in the given coordinate system units.
        The fourth constructor permits definition by specification of origin
        and corner. The coordinate system set for the rectangle is the coordinate
        system of the origin. The fifth allows creation of a rectangle
        from a fence. The sixth one create a rectangle from the given extent.

        @param pi_XMin Minimum X of rectangle. This value must be smaller than pi_XMax.

        @param pi_YMin Minimum Y of rectangle. This value must be smaller than pi_YMax.

        @param pi_XMax Maximum X of rectangle. This value must be greater than pi_XMin.

        @param pi_YMax Maximum Y of rectangle. This value must be greater than pi_YMin.

        @param pi_rOrigin Location specifying the origin of rectangle.

        @param pi_rCorner Location specifying the corner of rectangle.

        @param pi_rExtent An extent object from which is constructed a rectangle.

        @param pi_rRectFence Constant reference to a rectangle fence to transform into
                             a rectangle.

        @param pi_rObject Constant reference to a HGF2DRectangle to duplicate.


        Example:
        @code
        @end

        @see HGF2DLocation
        @see HGF2DLiteExtent
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT                       HGF2DRectangle();
    IMAGEPP_EXPORT                       HGF2DRectangle(double     pi_XMin,
                                                       double     pi_YMin,
                                                       double     pi_XMax,
                                                       double     pi_YMax);
    IMAGEPP_EXPORT                       HGF2DRectangle(const HGF2DPosition& pi_rFirstPoint,
                                                       const HGF2DPosition& pi_rSecondPoint);
    IMAGEPP_EXPORT                       HGF2DRectangle(const HGF2DLiteExtent& pi_rExtent);
    IMAGEPP_EXPORT                       HGF2DRectangle(const HGF2DRectangle&    pi_rObject);
    IMAGEPP_EXPORT virtual               ~HGF2DRectangle();

    HGF2DRectangle&                     operator=(const HGF2DRectangle& pi_rObj);

    // Getting and setting
    double                              GetXMin() const;
    double                              GetYMin() const;
    double                              GetXMax() const;
    double                              GetYMax() const;

    void                                SetRectangle(const HGF2DPosition& pi_rFirstPoint,
                                                     const HGF2DPosition& pi_rSecondPoint);
    void                                SetRectangle(double pi_XMin,
                                                     double pi_YMin,
                                                     double pi_XMax,
                                                     double pi_YMax);
    void                                GetRectangle(HGF2DPosition* pi_pMinPoint,
                                                     HGF2DPosition* pi_pMaxPoint) const;
    void                                GetRectangle(double* po_pXMin,
                                                     double* po_pYMin,
                                                     double* po_pXMax,
                                                     double* po_pYMax) const;
    bool                                Overlaps(const HGF2DShape& pi_rShape) const;

    // Misc
    IMAGEPP_EXPORT virtual void          Scale(double              pi_ScaleFactorX,
                                              double              pi_ScaleFactorY,
                                              const HGF2DPosition& pi_rScaleOrigin);


    // From HGF2DSimpleShape
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DLinear>    GetLinear() const;
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DLinear>    GetLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const;

    // From HGF2DShape

    // Scanlines generation
    IMAGEPP_EXPORT virtual void          Rasterize(HGFScanLines& pio_rScanlines) const;

    IMAGEPP_EXPORT virtual bool          IsEmpty() const;
    IMAGEPP_EXPORT virtual HGF2DShapeTypeId 
                                        GetShapeType() const;
    IMAGEPP_EXPORT virtual HGF2DShape*   DifferentiateShape(const HGF2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HGF2DShape*   DifferentiateFromShape(const HGF2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HGF2DShape*   IntersectShape(const HGF2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HGF2DShape*   UnifyShape(const HGF2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual double        CalculateArea() const;
    IMAGEPP_EXPORT virtual double        CalculatePerimeter() const;
    IMAGEPP_EXPORT virtual bool          IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual void          MakeEmpty();
    IMAGEPP_EXPORT virtual HGF2DShape::SpatialPosition    
                                        CalculateSpatialPositionOfSingleComponentVector(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual HGF2DShape::SpatialPosition    
                                        CalculateSpatialPositionOfNonCrossingLinear(const HGF2DLinear& pi_rLinear) const;

    IMAGEPP_EXPORT virtual void          Drop(HGF2DPositionCollection* po_pPoint,
                                             double      pi_rTolerance) const;

    // From HGF2DVector
    IMAGEPP_EXPORT virtual HGF2DPosition CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
    IMAGEPP_EXPORT virtual size_t        Intersect(const HGF2DVector& pi_rVector,
                                                   HGF2DPositionCollection* po_pCrossPoints) const;
    IMAGEPP_EXPORT virtual size_t        ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                                   HGF2DPositionCollection* po_pContiguousnessPoints) const;
    IMAGEPP_EXPORT virtual void          ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                                     const HGF2DPosition& pi_rPoint,
                                                                     HGF2DPosition* pi_pFirstContiguousnessPoint,
                                                                     HGF2DPosition* pi_pSecondContiguousnessPoint) const;
    IMAGEPP_EXPORT virtual bool          Crosses(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool          AreContiguous(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool          AreAdjacent(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool          IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                                  HGF2DVector::ExtremityProcessing
                                                  pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                                  double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual bool          AreContiguousAt(const HGF2DVector& pi_rVector,
                                                        const HGF2DPosition& pi_rPoint) const;
    IMAGEPP_EXPORT virtual HGFBearing    CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                                                         HGF2DVector::ArbitraryDirection pi_Direction = HGF2DVector::BETA) const;
    IMAGEPP_EXPORT virtual double        CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                                                     HGF2DVector::ArbitraryDirection pi_Direction = HGF2DVector::BETA) const;

    // From HGFGraphicObject
    IMAGEPP_EXPORT virtual HGF2DLiteExtent
                                        GetExtent() const;
    IMAGEPP_EXPORT virtual void          Move(const HGF2DDisplacement& pi_rDisplacement);
    IMAGEPP_EXPORT virtual void          Scale(double              pi_ScaleFactor,
                                              const HGF2DPosition& pi_rScaleOrigin);

    // From HPMPersistentObject
    IMAGEPP_EXPORT virtual HGF2DVector*  Clone() const;


    // Debugging
    IMAGEPP_EXPORT virtual void          PrintState(ostream& po_rOutput) const;


    virtual HFCPtr<HGF2DShape>          AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const override;

protected:

private:

    // Area oriented operations on rectangle
    virtual HGF2DShape*                 DifferentiateRectangle(const HGF2DRectangle& pi_rRectangle) const;
    virtual HGF2DShape*                 IntersectRectangle(const HGF2DRectangle& pi_rRectangle) const;
    virtual HGF2DShape*                 UnifyRectangle(const HGF2DRectangle& pi_rRectangle) const;

    void ResetTolerance();

    // Member attribute ... the extremes of the rectangle
    double     m_XMin;
    double     m_XMax;
    double     m_YMin;
    double     m_YMax;
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DRectangle.hpp"
