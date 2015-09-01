//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DSegment.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DSegment
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DBasicLinear.h"
#include "HGF2DLine.h"
#include "HGFBearing.h"
#include "HGF2DLocation.h"

BEGIN_IMAGEPP_NAMESPACE
class HVE2DComplexLinear;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a segment. This is a quite simple class made
    of two locations. The segment expressed a graphic object which is a
    line beginning from one of the points and finishing at the other point.
    The order of points is maintained, that is the segment is oriented from
    the first point to the second. The class implements methods to perform
    some mathematical operations upon segments and location (HGF2DLocation).
    Such operations include finding a crossing point between segments, or
    a line (HGF2DLine), and ways to obtains the closest point on segment
    from a specified location.
    -----------------------------------------------------------------------------
*/
class HVE2DSegment : public HVE2DBasicLinear
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DSegmentId)

public:

    enum CrossState
        {
        CROSS_FOUND,
        PARALLEL,
        NO_CROSS
        };

    // Primary methods
    /** -----------------------------------------------------------------------------
        These are the constructors and copy constructor of the HVE2DSegment
        class. They permit to instantiate a new segment. The first is
        the default constructor.The second constructor creates
        a default segment with both points set to (0,0) in the given
        coordinate system. The third one specifies immediately
        the two points of the segment. The coordinate system assigned to the
        segment is then the coordinate system of the start point.
        The fourth is equivalent to the third, except that the coordinates
        are specified by raw objects, and the coordinate system used for
        the interpretation is provided. The fifth constructor enables the
        user to create a segment by specification of a start point and
        a displacement to the end point. The sixth constructor is the
        copy constructor..

        @param pi_rpCoordSys Constant reference to a smart pointer to
                             coordinate system that will be used
                             in the interpretation of the segment.

        @param pi_rStartPoint Reference to an HGF2DLocation object containing
                              the definition of the start point of the segment.
                              From this point is also copied the smart pointer
                              to the coordinate system which will also be used
                              in the interpretation of the segment.

        @param pi_rEndPoint Reference to an HGF2DPosition object containing the
                            definition of the end point of the segment.

        @param pi_rStartPoint Reference to an HGF2DPosition object containing
                              the definition of the start point of the segment.

        @param pi_rEndPoint Reference to an HGF2DPosition object containing the
                            definition of the end point of the segment.

        @param pi_rDisplacement Constant reference to the displacement from
                                specified start point to end point.


        @param pi_rObject Constant reference to a HVE2DSegment to duplicate.


        Example:
        @code
        HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
        HVE2DSegment             MySeg1(pMyWorld);
        HGF2DLocation            MyFirstPoint(10, 10, pMyWorld);
        HGF2DLocation            MySecondPoint(15, 16, pMyWorld);
        HVE2DSegment             MySeg2(MyFirstPoint, MySecondPoint);
        HVE2DSegment             MySeg3(MySeg1);
        @end

        @see HGF2DCoordSys
        @see HGF2DLocation
        @see HGF2DPosition
        @see HGF2DDisplacement
        -----------------------------------------------------------------------------
    */
    HVE2DSegment();
    HVE2DSegment(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DSegment(const HGF2DLocation& pi_rStartPoint,
                 const HGF2DLocation& pi_rEndPoint);
    HVE2DSegment(const HGF2DPosition& pi_rStartPoint,
                 const HGF2DPosition& pi_rEndPoint,
                 const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DSegment(const HGF2DLocation& pi_rStartPoint,
                 const HGF2DDisplacement& pi_rDisplacement);
    HVE2DSegment(const HVE2DSegment&    pi_rObject);
    virtual            ~HVE2DSegment();

    HVE2DSegment&      operator=(const HVE2DSegment& pi_rObj);

    // Setting
    void               SetStartPoint(const HGF2DLocation& pi_rNewStartPoint);
    void               SetEndPoint(const HGF2DLocation& pi_rNewEndPoint);
    void               SetRawStartPoint(double pi_X, double pi_Y);
    void               SetRawEndPoint(double pi_X, double pi_Y);


    // Miscalenous
    HGF2DLine          CalculateLine() const;
    CrossState         IntersectLine(const HGF2DLine& pi_rLine,
                                     HGF2DLocation*   po_pPoint,
                                     double pi_Tolerance = HVE_USE_INTERNAL_EPSILON)const;
    CrossState         IntersectSegment(const HVE2DSegment& pi_rSegment,
                                        HGF2DLocation* po_pPoint)const;
    IMAGEPP_EXPORT CrossState  IntersectSegmentSCS(const HVE2DSegment& pi_rSegment,
                                           HGF2DLocation* po_pPoint)const;

    CrossState         IntersectSegmentExtremityIncludedSCS(const HVE2DSegment& pi_rSegment,
                                                            HGF2DLocation* po_pPoint,
                                                            bool* po_pIntersectsAtExtremity = NULL)const;

    bool               IsParallelTo(const HVE2DSegment& pi_rSegment) const;
    bool               IsParallelTo(const HGF2DLine& pi_rLine) const;

    // Geometry
    void               Rotate(double  pi_Angle,
                              const HGF2DLocation& pi_rOrigin);

    // From HVE2DBasicLinear
    HVE2DBasicLinearTypeId
                       GetBasicLinearType() const;

    // From HVE2DLinear
    IMAGEPP_EXPORT virtual double   CalculateLength() const;
    virtual HGF2DLocation   CalculateRelativePoint(double pi_RelativePos) const;
    virtual double          CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const;
    virtual double          CalculateRayArea(const HGF2DLocation& pi_rPoint) const;
    virtual void            Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
    virtual void            Shorten(const HGF2DLocation& pi_rNewStartPoint,
                                    const HGF2DLocation& pi_rNewEndPoint);
    virtual void            ShortenTo(const HGF2DLocation& pi_rNewEndPoint);
    virtual void            ShortenTo(double pi_EndRelativePosition);
    virtual void            ShortenFrom(const HGF2DLocation& pi_rNewStartPoint);
    virtual void            ShortenFrom(double pi_StartRelativePosition);
    virtual bool            AutoCrosses() const;
    virtual void            AdjustStartPointTo(const HGF2DLocation& pi_rPoint);
    virtual void            AdjustEndPointTo(const HGF2DLocation& pi_rPoint);

    virtual void            Drop(HGF2DLocationCollection* po_pPoint,
                                 double                   pi_Tolerance,
                                 EndPointProcessing pi_EndPointProcessing = INCLUDE_END_POINT) const;


    // From HVE2DVector
    IMAGEPP_EXPORT virtual HGF2DLocation
                            CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    IMAGEPP_EXPORT virtual size_t   Intersect(const HVE2DVector& pi_rVector,
                                      HGF2DLocationCollection* po_pCrossPoints) const;
    IMAGEPP_EXPORT virtual size_t   ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                       HGF2DLocationCollection* po_pContiguousnessPoints) const;
    IMAGEPP_EXPORT virtual void     ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                         const HGF2DLocation& pi_rPoint,
                                                         HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                         HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    IMAGEPP_EXPORT virtual HVE2DVector*
                            AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;

    IMAGEPP_EXPORT virtual bool     Crosses(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool     AreContiguous(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool     AreAdjacent(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool     IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                      HVE2DVector::ExtremityProcessing
                                      pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                      double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual bool     IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                         HVE2DVector::ExtremityProcessing
                                         pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                         double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual bool     AreContiguousAt(const HVE2DVector& pi_rVector,
                                            const HGF2DLocation& pi_rPoint) const;
    virtual HGFBearing      CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                             HVE2DVector::ArbitraryDirection
                                             pi_Direction = HVE2DVector::BETA) const;
    virtual double          CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                         HVE2DVector::ArbitraryDirection
                                                         pi_Direction = HVE2DVector::BETA) const;
    virtual bool            IsNull() const;

    // From HGFGraphicObject
    virtual HGF2DExtent     GetExtent() const;

    IMAGEPP_EXPORT virtual bool     AreContiguousAtAndGet(const HVE2DVector& pi_rVector,
                                                  const HGF2DLocation& pi_rPoint,
                                                  HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                  HGF2DLocation* pi_pSecondContiguousnessPoint) const;

    // From HPMPersistentObject
    virtual HPMPersistentObject*
                            Clone() const;


    IMAGEPP_EXPORT virtual void     PrintState(ostream& po_rOutput) const;

protected:

private:


    HVE2DVector* AllocateCopyInComplexCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    void         AppendItselfInCoordSys(HVE2DComplexLinear& pio_rResultComplex,
                                        const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    bool         AreSegmentsAdjacent(const HVE2DSegment& pi_rSegment) const;
    bool         AreSegmentsAdjacentSCS(const HVE2DSegment& pi_rSegment) const;
    bool         AreSegmentsFlirting(const HVE2DSegment& pi_rSegment) const;
    bool         AreSegmentsFlirtingSCS(const HVE2DSegment& pi_rSegment) const;
    bool         AreSegmentsContiguous(const HVE2DSegment& pi_rSegment) const;
    bool         AreSegmentsContiguousSCS(const HVE2DSegment& pi_rSegment) const;
    bool         AreSegmentsTouching(const HVE2DSegment& pi_rSegment) const;
    bool         AreSegmentsTouchingSCS(const HVE2DSegment& pi_rSegment) const;
    bool         AreSegmentsCrossing(const HVE2DSegment& pi_rSegment) const;
    bool         AreSegmentsCrossingSCS(const HVE2DSegment& pi_rSegment) const;
    size_t       ObtainContiguousnessPointsWithSegment(const HVE2DSegment& pi_rSegment,
                                                       HGF2DLocationCollection* po_pContiguousnessPoints) const;

    size_t       ObtainContiguousnessPointsWithSegmentSCS(const HVE2DSegment& pi_rSegment,
                                                          HGF2DLocationCollection* po_pContiguousnessPoints) const;

    bool        IsPointOnLineOnSegment(const HGF2DLocation& pi_rTestPoint,
                                        double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    bool        IsPointOnLineOnSegmentSCS(const HGF2DLocation& pi_rTestPoint,
                                           double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;

    bool        AreContiguousAtAndGetWithSegment(const HVE2DSegment& pi_rSegment,
                                                  HGF2DLocation* po_pFirstPoint,
                                                  HGF2DLocation* po_pSecondPoint)const;
    bool        AreContiguousAtAndGetWithSegmentSCS(const HVE2DSegment& pi_rSegment,
                                                     HGF2DLocation* po_pFirstPoint,
                                                     HGF2DLocation* po_pSecondPoint) const;

    void        ResetTolerance();
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DSegment.hpp"
