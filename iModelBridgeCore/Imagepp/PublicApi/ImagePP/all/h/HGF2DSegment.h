//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DSegment
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DBasicLinear.h"
#include "HGF2DLiteLine.h"

BEGIN_IMAGEPP_NAMESPACE
class HGFComplexLinear;


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
    a line (HGF2DLiteLine), and ways to obtains the closest point on segment
    from a specified location.
    -----------------------------------------------------------------------------
*/
class HGF2DSegment : public HGF2DBasicLinear
    {

//    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HGF2DSegmentId)
    HDECLARE_CLASS_ID(HGF2DSegmentId, HGF2DBasicLinear)

public:

    enum CrossState
        {
        CROSS_FOUND,
        PARALLEL,
        NO_CROSS
        };

    // Primary methods
    /** -----------------------------------------------------------------------------
        These are the constructors and copy constructor of the HGF2DSegment
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


        @param pi_rObject Constant reference to a HGF2DSegment to duplicate.


        Example:
        @code
        HGF2DSegment        MySeg1();
        HGF2DLocation       MyFirstPoint(10, 10));
        HGF2DLocation       MySecondPoint(15, 16);
        HGF2DSegment        MySeg2(MyFirstPoint, MySecondPoint);
        HGF2DSegment        MySeg3(MySeg1);
        @end

        @see HGF2DPosition
        -----------------------------------------------------------------------------
    */
     IMAGEPPTEST_EXPORT                   HGF2DSegment();
     IMAGEPPTEST_EXPORT                   HGF2DSegment(const HGF2DPosition& pi_rStartPoint,
                                     const HGF2DPosition& pi_rEndPoint);
     IMAGEPPTEST_EXPORT                   HGF2DSegment(const HGF2DPosition& pi_rStartPoint,
                                     const HGF2DDisplacement& pi_rDisplacement);
     IMAGEPPTEST_EXPORT                   HGF2DSegment(const HGF2DSegment&    pi_rObject);
    virtual             ~HGF2DSegment();

    HGF2DSegment&       operator=(const HGF2DSegment& pi_rObj);

    // Setting
    IMAGEPPTEST_EXPORT void                SetStartPoint(const HGF2DPosition& pi_rNewStartPoint);
    IMAGEPPTEST_EXPORT void                SetEndPoint(const HGF2DPosition& pi_rNewEndPoint);
    IMAGEPPTEST_EXPORT void                SetRawStartPoint(double pi_X, double pi_Y);
    IMAGEPPTEST_EXPORT void                SetRawEndPoint(double pi_X, double pi_Y);


    // Miscalenous
    IMAGEPPTEST_EXPORT HGF2DLiteLine       CalculateLine() const;
    IMAGEPPTEST_EXPORT CrossState          IntersectLine(const HGF2DLiteLine& pi_rLine,
                                      HGF2DPosition*   po_pPoint,
                                      double pi_Tolerance = HGF_USE_INTERNAL_EPSILON)const;
    IMAGEPPTEST_EXPORT CrossState          IntersectSegment(const HGF2DSegment& pi_rSegment,
                                         HGF2DPosition* po_pPoint)const;
    IMAGEPP_EXPORT CrossState  
                        IntersectSegmentSCS(const HGF2DSegment& pi_rSegment,
                                            HGF2DPosition* po_pPoint)const;

    IMAGEPPTEST_EXPORT CrossState          IntersectSegmentExtremityIncluded(const HGF2DSegment& pi_rSegment,
                                                          HGF2DPosition* po_pPoint,
                                                          bool* po_pIntersectsAtExtremity = NULL)const;

    IMAGEPPTEST_EXPORT bool                IsParallelTo(const HGF2DSegment& pi_rSegment) const;
    IMAGEPPTEST_EXPORT bool                IsParallelTo(const HGF2DLiteLine& pi_rLine) const;

    // Geometry
    IMAGEPPTEST_EXPORT void                Rotate(double pi_Angle,
                               const HGF2DPosition& pi_rOrigin);

    // From HGF2DBasicLinear
    HGF2DBasicLinearTypeId
                        GetBasicLinearType() const override;

    // From HGF2DLinear
    IMAGEPP_EXPORT double    
                        CalculateLength() const override;
    IMAGEPPTEST_EXPORT HGF2DPosition    
                        CalculateRelativePoint(double pi_RelativePos) const override;
    IMAGEPPTEST_EXPORT double      CalculateRelativePosition(const HGF2DPosition& pi_rPointOnLinear) const override;
    IMAGEPPTEST_EXPORT double      CalculateRayArea(const HGF2DPosition& pi_rPoint) const override;
    IMAGEPPTEST_EXPORT void        Shorten(double pi_StartRelativePos, double pi_EndRelativePos) override;
    IMAGEPPTEST_EXPORT void        Shorten(const HGF2DPosition& pi_rNewStartPoint,
                                const HGF2DPosition& pi_rNewEndPoint) override;
    IMAGEPPTEST_EXPORT void        ShortenTo(const HGF2DPosition& pi_rNewEndPoint) override;
    IMAGEPPTEST_EXPORT void        ShortenTo(double pi_EndRelativePosition) override;
    IMAGEPPTEST_EXPORT void        ShortenFrom(const HGF2DPosition& pi_rNewStartPoint) override;
    IMAGEPPTEST_EXPORT void        ShortenFrom(double pi_StartRelativePosition) override;
    IMAGEPPTEST_EXPORT bool        AutoCrosses() const override;
    IMAGEPPTEST_EXPORT void        AdjustStartPointTo(const HGF2DPosition& pi_rPoint) override;
    IMAGEPPTEST_EXPORT void        AdjustEndPointTo(const HGF2DPosition& pi_rPoint) override;

    IMAGEPPTEST_EXPORT void        Drop(HGF2DPositionCollection* po_pPoint,
                             double                   pi_Tolerance,
                             EndPointProcessing pi_EndPointProcessing = INCLUDE_END_POINT) const override;


    // From HGF2DVector
    IMAGEPP_EXPORT HGF2DPosition    
                        CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const override;
    IMAGEPP_EXPORT size_t    
                        Intersect(const HGF2DVector& pi_rVector,
                                  HGF2DPositionCollection* po_pCrossPoints) const override;
    IMAGEPP_EXPORT size_t    
                        ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                   HGF2DPositionCollection* po_pContiguousnessPoints) const override;
    IMAGEPP_EXPORT void      
                        ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                     const HGF2DPosition& pi_rPoint,
                                                     HGF2DPosition* pi_pFirstContiguousnessPoint,
                                                     HGF2DPosition* pi_pSecondContiguousnessPoint) const override;

    IMAGEPP_EXPORT bool     
                        Crosses(const HGF2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool     
                        AreContiguous(const HGF2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool     
                        AreAdjacent(const HGF2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool     
                        IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                  HGF2DVector::ExtremityProcessing
                                  pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                  double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const override;
    IMAGEPP_EXPORT bool     
                        AreContiguousAt(const HGF2DVector& pi_rVector,
                                        const HGF2DPosition& pi_rPoint) const override;
    IMAGEPPTEST_EXPORT HGFBearing  CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                                         HGF2DVector::ArbitraryDirection
                                         pi_Direction = HGF2DVector::BETA) const override;
    IMAGEPPTEST_EXPORT double      CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                                     HGF2DVector::ArbitraryDirection
                                                     pi_Direction = HGF2DVector::BETA) const override;
    IMAGEPPTEST_EXPORT bool        IsNull() const override;

    // From HGF2DVector
    IMAGEPPTEST_EXPORT HGF2DLiteExtent    
                        GetExtent() const override;

    IMAGEPP_EXPORT bool    
                        AreContiguousAtAndGet(const HGF2DVector& pi_rVector,
                                              const HGF2DPosition& pi_rPoint,
                                              HGF2DPosition* pi_pFirstContiguousnessPoint,
                                              HGF2DPosition* pi_pSecondContiguousnessPoint) const override;

    HGF2DVector*     
                        Clone() const override;


    IMAGEPP_EXPORT void
                        PrintState(ostream& po_rOutput) const override;

    // THIS SHOULD BE PRIVATE BUT MOVED TEMPORARILY HERE TO BE ACCESSED FROM HVE2DSegment
    bool                AreContiguousAtAndGetWithSegment(const HGF2DSegment& pi_rSegment,
                                                         HGF2DPosition* po_pFirstPoint,
                                                         HGF2DPosition* po_pSecondPoint)const;
    size_t              ObtainContiguousnessPointsWithSegment(const HGF2DSegment& pi_rSegment,
                                                              HGF2DPositionCollection* po_pContiguousnessPoints) const;        
    bool                AreSegmentsContiguous(const HGF2DSegment& pi_rSegment) const;

protected:

private:


    bool                AreSegmentsAdjacent(const HGF2DSegment& pi_rSegment) const;
    bool                AreSegmentsFlirting(const HGF2DSegment& pi_rSegment) const;
    bool                AreSegmentsTouching(const HGF2DSegment& pi_rSegment) const;
    bool                AreSegmentsCrossing(const HGF2DSegment& pi_rSegment) const;


    bool                IsPointOnLineOnSegment(const HGF2DPosition& pi_rTestPoint,
                                               double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;


    void                ResetTolerance();
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DSegment.hpp"
