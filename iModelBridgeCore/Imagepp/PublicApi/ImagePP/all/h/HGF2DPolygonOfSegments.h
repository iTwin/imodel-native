//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DPolygonOfSegments.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DPolygonOfSegments
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DSimpleShape.h"
#include "HGF2DExtent.h"
#include "HGF2DPolySegment.h"
#include "HGF2DLiteSegment.h"

BEGIN_IMAGEPP_NAMESPACE

class HGF2DRectangle;
class HGF2DComplexShape;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements the simple shape of type polygon of segments.
    A polygon of segment is constituted of linked segments which form
    a path in 2D space which does not auto intersect and closes
    on itself. This type of simple shape is not unlike the generic
    polygon (HGFPolygon), but is faster due to the fact that
    segments are known to be the sole constituent of a polygon of segments.

    Polygons of segment are far more faster to operation surface
    operations (Union, Intersect and Differentiate) than the generic
    polygon (HGFPolygon). If it is known in advance that only segments
    will be used for the construction of a shape, it is preferable
    to use an HGF2DPolygonOfSegments object.

    -----------------------------------------------------------------------------
*/
class HGF2DPolygonOfSegments : public HGF2DSimpleShape
    {

    HDECLARE_CLASS_ID(HGF2DPolygonOfSegmentsId, HGF2DSimpleShape)

public:


    // Primary methods
    /** -----------------------------------------------------------------------------

        Constructors and copy constructor. These methods permit to instantiate
        a new HGF2DPolygonOfSegments object. The first one is the default
        constructor. The second one only sets the interpretation coordinate system.
        The third constructs a polygon using a fence. The fourth one enables specification
        of polygon by giving a complex linear. This complex linear must close on itself,
        and only be constituted of HGF2DSegment objects. The fifth enables specification
        using a polysegment (HGF2DPolySegment) that must auto close and not autocross. The
        next creates a polygon of segments from a rectangle.
        An equivalent polygon of segments is constructed with four segments.
        The next constructor enables construction of polygon of segments from a
        raw set of points. The coordinates are placed in a buffer (or array) ou
        doubles, the first and all even position represent X coordinates, and all
        odd position represent Y coordinates. The buffer length must indicate the
        number of entries in array, not the number of coordinate pairs. This
        number must therefore be an even number. The last point of this buffer
        may or may not be identical to the first point. If this last coordinate
        pair is different, then an additionnal closing point is automatically added.
        In all constructor where no coordinate system is specificaly specified,
        the interpretation coordinate system is the one of the defining object

        @param pi_rpCoordSys Constant reference to a smart pointer to coordinate
                             system that will be used in the interpretation of
                             the shape.

        @param pi_rShape Constant refence to HGF2DPolygonOfSegments from which will
                         be extracted the segments. The given polygon fence
                         must be valid.

        @param pi_rComplex Constant reference to a complex linear which must
                           not auto intersect, auto-close and only be constituted
                           of HGF2DSegment objects.

        @param pi_BufferLength The number of entries in pi_aBuffer.

        @param pi_aBuffer An array of double containing for all even position X
                          coordinate, and for all odd position Y coordinates.
                          The last coordinate pair may or may not auto-close the path.

        @param pi_rRectangle An HGF2DRectangle object from which is constructed
                             a polygon of segments.

        @param pi_rObject Constant reference to a HGF2DPolygonOfSegments to duplicate.

        Example:
        @code
        @end

        @see IsComplex()
        @see HGF2DBasicLinear
        @see HGFComplexLinear
        -----------------------------------------------------------------------------
    */
                                    HGF2DPolygonOfSegments();
                                    HGF2DPolygonOfSegments(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT                   HGF2DPolygonOfSegments(const HGF2DPolySegment& pi_rPolySegment);
                                    HGF2DPolygonOfSegments(const HGF2DRectangle& pi_rRectangle);
    IMAGEPP_EXPORT                   HGF2DPolygonOfSegments(size_t  pi_BufferLength,
                                                           double pi_aBuffer[]);
    IMAGEPP_EXPORT                   HGF2DPolygonOfSegments(const HGF2DPolygonOfSegments&   pi_rObject);
    IMAGEPP_EXPORT virtual           ~HGF2DPolygonOfSegments();

    HGF2DPolygonOfSegments&         operator=(const HGF2DPolygonOfSegments& pi_rObj);

    virtual HGF2DSimpleShape::RotationDirection    
                                    CalculateRotationDirection() const;

    // Parallel Copy
    IMAGEPP_EXPORT HGF2DPolygonOfSegments*      
                                    AllocateParallelCopy(double pi_rOffset,
                                                         HGF2DVector::ArbitraryDirection
                                                         pi_DirectionToRight = HGF2DVector::BETA) const;

    // Point extraction
    const HGF2DPositionCollection&
                                    GetListOfPoints() const;

    // Setting
//    IMAGEPP_EXPORT virtual void       SetLinear(const HGF2DLinear& pi_rLinear);

    // Simplification
    IMAGEPP_EXPORT HGF2DRectangle*   GenerateCorrespondingRectangle() const;
    IMAGEPP_EXPORT bool              RepresentsARectangle() const;

    bool                            IsConvex() const;

    // Misc
    IMAGEPP_EXPORT virtual void      Rotate(double pi_Angle,
                                           const HGF2DPosition& pi_rRotationOrigin);
    IMAGEPP_EXPORT virtual void      Scale(double pi_ScaleFactorX,
                                          double pi_ScaleFactorY,
                                          const HGF2DPosition& pi_rScaleOrigin);


    // From HGF2DSimpleShape
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DLinear>   
                                    GetLinear() const;
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DLinear>   
                                    GetLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const;

    // From HGF2DShape

    IMAGEPP_EXPORT virtual void      Rasterize(HGFScanLines& pio_rScanlines) const;

    IMAGEPP_EXPORT virtual bool      IsEmpty     () const;
    IMAGEPP_EXPORT virtual HGF2DShapeTypeId
                                    GetShapeType() const;
    IMAGEPP_EXPORT virtual double    CalculateArea() const;
    IMAGEPP_EXPORT virtual double    CalculatePerimeter() const;
    IMAGEPP_EXPORT virtual bool      IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual void      MakeEmpty();

    IMAGEPP_EXPORT virtual HGF2DShape* 
                                    DifferentiateFromShape(const HGF2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HGF2DShape*  
                                    DifferentiateShape(const HGF2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HGF2DShape*  
                                    IntersectShape(const HGF2DShape& pi_rShape) const;
    IMAGEPP_EXPORT virtual HGF2DShape*  
                                    UnifyShape(const HGF2DShape& pi_rShape) const;

    IMAGEPP_EXPORT virtual void      Drop(HGF2DPositionCollection* po_pPoint,
                                         double                   pi_rTolerance) const;

    // Special

    // From HGF2DVector
    IMAGEPP_EXPORT virtual HGF2DPosition    
                                    CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
    IMAGEPP_EXPORT virtual size_t    Intersect(const HGF2DVector& pi_rVector,
                                              HGF2DPositionCollection* po_pCrossPoints) const;
    IMAGEPP_EXPORT virtual size_t    ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                               HGF2DPositionCollection* po_pContiguousnessPoints) const;
    IMAGEPP_EXPORT virtual void      ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                                 const HGF2DPosition& pi_rPoint,
                                                                 HGF2DPosition* pi_pFirstContiguousnessPoint,
                                                                 HGF2DPosition* pi_pSecondContiguousnessPoint) const;
    virtual HGF2DVector*            Clone() const;
    IMAGEPP_EXPORT virtual bool      Crosses(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool      AreContiguous(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool      AreAdjacent(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool      IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                              HGF2DVector::ExtremityProcessing
                                              pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                              double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual bool      AreContiguousAt(const HGF2DVector& pi_rVector,
                                                    const HGF2DPosition& pi_rPoint) const;
    IMAGEPP_EXPORT virtual HGFBearing   
                                    CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                                                     HGF2DVector::ArbitraryDirection
                                                     pi_Direction = HGF2DVector::BETA) const;
    IMAGEPP_EXPORT virtual double    CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                                                 HGF2DVector::ArbitraryDirection
                                                                 pi_Direction = HGF2DVector::BETA) const;
    IMAGEPP_EXPORT virtual void      SetTolerance(double pi_Tolerance);
    IMAGEPP_EXPORT virtual void      SetStrokeTolerance(const HFCPtr<HGFLiteTolerance> & pi_Tolerance);
    IMAGEPP_EXPORT virtual void      SetAutoToleranceActive(bool pi_ActiveAutoTolerance);

    // From HGFGraphicObject
    IMAGEPP_EXPORT virtual HGF2DLiteExtent               
                                    GetExtent() const;
    IMAGEPP_EXPORT virtual void      Move(const HGF2DDisplacement& pi_rDisplacement);
    IMAGEPP_EXPORT virtual void      Scale(double pi_ScaleFactor,
                                          const HGF2DPosition& pi_rScaleOrigin);

    // Debug method
    IMAGEPP_EXPORT virtual void      PrintState(ostream& po_rOutput) const;

    // THIS METHOD IS PUBLIC FOR DEBUG PURPOSES ONLY ... DO NOT CALL!!!!
    HGF2DShape*                     AllocateComplexShapeFromAutoContiguousPolySegment(const HGF2DPolySegment& pi_rPolySegment) const;


    virtual HFCPtr<HGF2DShape>              AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const override;

    
    // Helper functions
    // Although, neither input nor result is directly related to HGF2DPolygonOfSegments
    // It is accessible here. The simple reason is that the polygon of segments
    // is the primary (but not sole) consumer for this function and that all
    // result shapes are know to polygons while they remain unknown to polysegments.
    static HGF2DShape* CreateShapeFromAutoCrossingPolySegment(const HGF2DPolySegment& pi_rAutoCrossingPolySegment);

protected:

private:
#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {

        // Polysegment and polygon of segments must have the same tolerance
        // and tolerance setting
        HASSERT(m_PolySegment.GetTolerance() == GetTolerance());
        HASSERT(m_PolySegment.IsAutoToleranceActive() == IsAutoToleranceActive());

        // Polysegment must close on itself
        HASSERT(m_PolySegment.m_StartPoint == m_PolySegment.m_EndPoint);

        // Polysegment may not cross itself
        HASSERTSUPERDEBUG(!m_PolySegment.AutoCrosses());

        // Polysegment may not be auto contiguous
        HASSERTSUPERDEBUG(!m_PolySegment.IsAutoContiguous());
        }
#endif

    enum DecomposeOperation
        {
        DIFF,
        DIFFFROM,
        UNION,
        INTERSECT
        };

    enum PointUsage
        {
        UNKNOWN,
        ON_POINT,
        USED
        };


    IMAGEPP_EXPORT virtual double    CalculateRawArea() const;

    HGF2DShape::SpatialPosition     CalculateSpatialPositionOfPolygonOfSegments(const HGF2DPolygonOfSegments& pi_rPolygon) const;

    HGF2DShape::SpatialPosition     CalculateSpatialPositionOfNonCrossingPolygonOfSegments(const HGF2DPolygonOfSegments& pi_rPolygon) const;


    bool                            CrossesPolygonOfSegments(const HGF2DPolygonOfSegments& pi_rPolygon) const;

    bool                            InteractsWith(const HGF2DPolygonOfSegments& pi_rPolygon,
                                                  HGF2DPositionCollection* po_pSelfPolyPoints,
                                                  HGF2DPositionCollection* po_pGivenPolyPoints,
                                                  HGF2DPolygonOfSegments::DecomposeOperation pi_Operation,
                                                  bool pi_IgnoreSimpleContiguousness = false,
                                                  bool* po_pContiguousInteraction = NULL) const;


    bool                            InteractsWithSameUnits(const HGF2DPolygonOfSegments& pi_rPolygon,
                                                           HGF2DPositionCollection* po_pSelfPolyPoints,
                                                           HGF2DPositionCollection* po_pGivenPolyPoints,
                                                           HGF2DPolygonOfSegments::DecomposeOperation pi_Operation,
                                                           bool pi_IgnoreSimpleContiguousness = false,
                                                           bool* po_pContiguousInteraction = NULL) const;



    void                            InteractsWithSameUnitsContiguousProcessing(HGF2DLiteSegment& selfSegment,
                                                                               HGF2DLiteSegment& givenSegment,
                                                                               double Tolerance,
                                                                               bool pi_IgnoreSimpleContiguousness,
                                                                               bool* po_pContiguousInteraction,
                                                                               bool* ReturnValue,
                                                                               HGF2DPositionCollection*             po_pGivenPolyPoints,
                                                                               HGF2DPositionCollection*             po_pSelfPolyPoints,
                                                                               HGF2DPositionCollection::iterator&   SelfItr,
                                                                               HGF2DPositionCollection::iterator&   PrevSelfItr,
                                                                               HGF2DPositionCollection::iterator&   GivenItr,
                                                                               HGF2DPositionCollection::iterator&   PrevGivenItr) const;

    HGF2DShape*                     UnifyCrossingPolygon(const HGF2DPolygonOfSegments& pi_rPolygon,
                                                         const HGF2DPositionCollection& pi_rPoly1,
                                                         const HGF2DPositionCollection& pi_rPoly2) const;
    HGF2DShape*                     UnifyPolygon(const HGF2DPolygonOfSegments& pi_rPolygon) const;
    HGF2DShape*                     IntersectCrossingPolygon(const HGF2DPolygonOfSegments& pi_rPolygon,
                                                             const HGF2DPositionCollection& pi_rPoly1,
                                                             const HGF2DPositionCollection& pi_rPoly2) const;
    HGF2DShape*                     IntersectPolygon(const HGF2DPolygonOfSegments& pi_rPolygon) const;
    HGF2DShape*                     DifferentiateCrossingPolygon(const HGF2DPolygonOfSegments& pi_rPolygon,
                                                                 const HGF2DPositionCollection& pi_rPoly1,
                                                                 const HGF2DPositionCollection& pi_rPoly2) const;
    HGF2DShape*                     DifferentiatePolygon(const HGF2DPolygonOfSegments& pi_rPolygon) const;
    HGF2DShape*                     DifferentiateFromCrossingPolygon(const HGF2DPolygonOfSegments& pi_rPolygon,
                                                                     const HGF2DPositionCollection& pi_rPoly1,
                                                                     const HGF2DPositionCollection& pi_rPoly2) const;
    HGF2DShape*                     DifferentiateFromPolygon(const HGF2DPolygonOfSegments& pi_rPolygon) const;

    void                            Decompose(const HGF2DPolygonOfSegments& pi_rPolygon,
                                              const HGF2DPositionCollection& pi_rPoly1,
                                              const HGF2DPositionCollection& pi_rPoly2,
                                              HGF2DPolygonOfSegments::DecomposeOperation pi_Operation,
                                              HGF2DShape::SimpleShapeList& pi_rListOfShapes) const;

    void                            SuperScan(const HGF2DPolygonOfSegments&  pi_rGiven,
                                              const HGF2DPositionCollection& pi_rPoly1,
                                              const HGF2DPositionCollection& pi_rPoly2,
                                              bool                          pi_WantInPtsOfShape1,
                                              bool                          pi_ScanShape1CW,
                                              bool                          pi_WantInPtsOfShape2,
                                              bool                          pi_ScanShape2CW,
                                              HGF2DShape::SimpleShapeList&   pi_rListOfShapes,
                                              bool*                         po_pAllOnPoints) const;

    void                            SuperScan2(const HGF2DPolygonOfSegments&  pi_rGiven,
                                               const HGF2DPositionCollection& pi_rPoly1,
                                               const HGF2DPositionCollection& pi_rPoly2,
                                               bool                          pi_WantInPtsOfShape1,
                                               bool                          pi_ScanShape1CW,
                                               bool                          pi_WantInPtsOfShape2,
                                               bool                          pi_ScanShape2CW,
                                               HGF2DShape::SimpleShapeList&   pi_rListOfShapes,
                                               bool*                         po_pAllOnPoints) const;




    HGF2DShape::SpatialPosition     CalculateSpatialPositionOfPosition(const HGF2DPosition& pi_rPoint, double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    HGF2DShape::SpatialPosition     CalculateSpatialPositionOfPositionSameUnits(const HGF2DPosition& pi_rPoint, double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    HGF2DShape::SpatialPosition     CalculateSpatialPositionOfPositionSegment(const HGF2DPosition& pi_rPoint,
                                                                              const HGF2DPosition& pi_rStartPoint,
                                                                              const HGF2DPosition& pi_rEndPoint,
                                                                              double              pi_Tolerance) const;
    HGF2DShape::SpatialPosition     CalculateSpatialPositionOfPositionSegment2(const HGF2DLiteExtent& pi_rSelfLiteExtent,
                                                                               const HGF2DPosition& pi_rStartPoint,
                                                                               const HGF2DPosition& pi_rEndPoint,
                                                                               double              pi_Tolerance) const;

    void                            Simplify();

    void                            ResetTolerance();

    IMAGEPP_EXPORT virtual HGF2DShape*
                                    Clip(const HGF2DRectangle& pi_rRectangle) const;

    PointUsage*                     InsertAutoContiguousPoints(HGF2DPolySegment& pio_rPolySegment) const;
    void                            InsertAutoFlirtPoints(HGF2DPositionCollection& pio_rPoints) const;

    void                            ChangeShape(const HGF2DPolygonOfSegments* const* apPoly,
                                                const HGF2DPositionCollection**  apPoints,
                                                HGF2DLiteExtent* PolyExtents,
                                                PointUsage** Flags,
                                                bool* PolyIn,
                                                size_t& ShapeIndex,
                                                size_t& TestShapeIndex,
                                                size_t& Index,
                                                size_t& PrevIndex,
                                                HGF2DPosition& PreviousPoint,
                                                HGF2DPosition& CurrentPoint,
                                                double Tolerance) const;

    bool                            ChangeToShape(const HGF2DPolygonOfSegments* const* apPoly,
                                                  const HGF2DPositionCollection**  apPoints,
                                                  HGF2DLiteExtent* PolyExtents,
                                                  PointUsage** Flags,
                                                  bool* PolyIn,
                                                  size_t& ShapeIndex,
                                                  size_t& TestShapeIndex,
                                                  size_t& Index,
                                                  HGF2DPosition& PreviousPoint,
                                                  double Tolerance,
                                                  bool OnAccepted = true) const;

    HGF2DVector*                    AllocateCopyInParallismPreservingRelatedCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    HGF2DVector*                    AllocateCopyInLinearityPreservingRelatedCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    HGF2DVector*                    AllocateCopyInGeneralRelatedCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;


    // Member attribute ... a list of positions
    HGF2DPolySegment m_PolySegment;

    // Acceleration attributes
    HGF2DSimpleShape::RotationDirection   m_RotationDirection;
    bool               m_RotationDirectionUpToDate;
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DPolygonOfSegments.hpp"
