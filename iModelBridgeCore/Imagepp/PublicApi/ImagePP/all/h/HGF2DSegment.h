//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DSegment.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
                        HGF2DSegment();
                        HGF2DSegment(const HGF2DPosition& pi_rStartPoint,
                                     const HGF2DPosition& pi_rEndPoint);
                        HGF2DSegment(const HGF2DPosition& pi_rStartPoint,
                                     const HGF2DDisplacement& pi_rDisplacement);
                        HGF2DSegment(const HGF2DSegment&    pi_rObject);
    virtual             ~HGF2DSegment();

    HGF2DSegment&       operator=(const HGF2DSegment& pi_rObj);

    // Setting
    void                SetStartPoint(const HGF2DPosition& pi_rNewStartPoint);
    void                SetEndPoint(const HGF2DPosition& pi_rNewEndPoint);
    void                SetRawStartPoint(double pi_X, double pi_Y);
    void                SetRawEndPoint(double pi_X, double pi_Y);


    // Miscalenous
    HGF2DLiteLine       CalculateLine() const;
    CrossState          IntersectLine(const HGF2DLiteLine& pi_rLine,
                                      HGF2DPosition*   po_pPoint,
                                      double pi_Tolerance = HGF_USE_INTERNAL_EPSILON)const;
    CrossState          IntersectSegment(const HGF2DSegment& pi_rSegment,
                                         HGF2DPosition* po_pPoint)const;
    IMAGEPP_EXPORT CrossState  
                        IntersectSegmentSCS(const HGF2DSegment& pi_rSegment,
                                            HGF2DPosition* po_pPoint)const;

    CrossState          IntersectSegmentExtremityIncluded(const HGF2DSegment& pi_rSegment,
                                                          HGF2DPosition* po_pPoint,
                                                          bool* po_pIntersectsAtExtremity = NULL)const;

    bool                IsParallelTo(const HGF2DSegment& pi_rSegment) const;
    bool                IsParallelTo(const HGF2DLiteLine& pi_rLine) const;

    // Geometry
    void                Rotate(double pi_Angle,
                               const HGF2DPosition& pi_rOrigin);

    // From HGF2DBasicLinear
    HGF2DBasicLinearTypeId
                        GetBasicLinearType() const;

    // From HGF2DLinear
    IMAGEPP_EXPORT virtual double    
                        CalculateLength() const;
    virtual HGF2DPosition    
                        CalculateRelativePoint(double pi_RelativePos) const;
    virtual double      CalculateRelativePosition(const HGF2DPosition& pi_rPointOnLinear) const;
    virtual double      CalculateRayArea(const HGF2DPosition& pi_rPoint) const;
    virtual void        Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
    virtual void        Shorten(const HGF2DPosition& pi_rNewStartPoint,
                                const HGF2DPosition& pi_rNewEndPoint);
    virtual void        ShortenTo(const HGF2DPosition& pi_rNewEndPoint);
    virtual void        ShortenTo(double pi_EndRelativePosition);
    virtual void        ShortenFrom(const HGF2DPosition& pi_rNewStartPoint);
    virtual void        ShortenFrom(double pi_StartRelativePosition);
    virtual bool        AutoCrosses() const;
    virtual void        AdjustStartPointTo(const HGF2DPosition& pi_rPoint);
    virtual void        AdjustEndPointTo(const HGF2DPosition& pi_rPoint);

    virtual void        Drop(HGF2DPositionCollection* po_pPoint,
                             double                   pi_Tolerance,
                             EndPointProcessing pi_EndPointProcessing = INCLUDE_END_POINT) const;


    // From HGF2DVector
    IMAGEPP_EXPORT virtual HGF2DPosition    
                        CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
    IMAGEPP_EXPORT virtual size_t    
                        Intersect(const HGF2DVector& pi_rVector,
                                  HGF2DPositionCollection* po_pCrossPoints) const;
    IMAGEPP_EXPORT virtual size_t    
                        ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                   HGF2DPositionCollection* po_pContiguousnessPoints) const;
    IMAGEPP_EXPORT virtual void      
                        ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                     const HGF2DPosition& pi_rPoint,
                                                     HGF2DPosition* pi_pFirstContiguousnessPoint,
                                                     HGF2DPosition* pi_pSecondContiguousnessPoint) const;

    IMAGEPP_EXPORT virtual bool     
                        Crosses(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool     
                        AreContiguous(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool     
                        AreAdjacent(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool     
                        IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                  HGF2DVector::ExtremityProcessing
                                  pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                  double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual bool     
                        AreContiguousAt(const HGF2DVector& pi_rVector,
                                        const HGF2DPosition& pi_rPoint) const;
    virtual HGFBearing  CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                                         HGF2DVector::ArbitraryDirection
                                         pi_Direction = HGF2DVector::BETA) const;
    virtual double      CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                                     HGF2DVector::ArbitraryDirection
                                                     pi_Direction = HGF2DVector::BETA) const;
    virtual bool        IsNull() const;

    // From HGF2DVector
    virtual HGF2DLiteExtent    
                        GetExtent() const;

    IMAGEPP_EXPORT virtual bool    
                        AreContiguousAtAndGet(const HGF2DVector& pi_rVector,
                                              const HGF2DPosition& pi_rPoint,
                                              HGF2DPosition* pi_pFirstContiguousnessPoint,
                                              HGF2DPosition* pi_pSecondContiguousnessPoint) const;

    virtual HGF2DVector*     
                        Clone() const;


    IMAGEPP_EXPORT virtual void
                        PrintState(ostream& po_rOutput) const;

protected:

private:


    bool                AreSegmentsAdjacent(const HGF2DSegment& pi_rSegment) const;
    bool                AreSegmentsFlirting(const HGF2DSegment& pi_rSegment) const;
    bool                AreSegmentsContiguous(const HGF2DSegment& pi_rSegment) const;
    bool                AreSegmentsTouching(const HGF2DSegment& pi_rSegment) const;
    bool                AreSegmentsCrossing(const HGF2DSegment& pi_rSegment) const;
    size_t              ObtainContiguousnessPointsWithSegment(const HGF2DSegment& pi_rSegment,
                                                              HGF2DPositionCollection* po_pContiguousnessPoints) const;

    bool                IsPointOnLineOnSegment(const HGF2DPosition& pi_rTestPoint,
                                               double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    bool                AreContiguousAtAndGetWithSegment(const HGF2DSegment& pi_rSegment,
                                                         HGF2DPosition* po_pFirstPoint,
                                                         HGF2DPosition* po_pSecondPoint)const;

    void                ResetTolerance();
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DSegment.hpp"
