//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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

// &&AR Not sure we should keep this one!
    friend class HVE2DPolySegment;

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
    IMAGEPPTEST_EXPORT CrossState         IntersectLine(const HGF2DLine& pi_rLine,
                                     HGF2DLocation*   po_pPoint,
                                     double pi_Tolerance = HVE_USE_INTERNAL_EPSILON)const;
    IMAGEPPTEST_EXPORT CrossState         IntersectSegment(const HVE2DSegment& pi_rSegment,
                                        HGF2DLocation* po_pPoint)const;
    IMAGEPP_EXPORT CrossState  IntersectSegmentSCS(const HVE2DSegment& pi_rSegment,
                                           HGF2DLocation* po_pPoint)const;

    IMAGEPPTEST_EXPORT CrossState         IntersectSegmentExtremityIncludedSCS(const HVE2DSegment& pi_rSegment,
                                                            HGF2DLocation* po_pPoint,
                                                            bool* po_pIntersectsAtExtremity = NULL)const;

    bool               IsParallelTo(const HVE2DSegment& pi_rSegment) const;
    bool               IsParallelTo(const HGF2DLine& pi_rLine) const;

    // Geometry
    IMAGEPPTEST_EXPORT void               Rotate(double  pi_Angle,
                              const HGF2DLocation& pi_rOrigin);

    // From HVE2DBasicLinear
    HVE2DBasicLinearTypeId
                       GetBasicLinearType() const override;

    // From HVE2DLinear
    IMAGEPP_EXPORT double   CalculateLength() const override;
    HGF2DLocation   CalculateRelativePoint(double pi_RelativePos) const override;
    double          CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const override;
    double          CalculateRayArea(const HGF2DLocation& pi_rPoint) const override;
    void            Shorten(double pi_StartRelativePos, double pi_EndRelativePos) override;
    void            Shorten(const HGF2DLocation& pi_rNewStartPoint,
                                    const HGF2DLocation& pi_rNewEndPoint) override;
    void            ShortenTo(const HGF2DLocation& pi_rNewEndPoint) override;
    void            ShortenTo(double pi_EndRelativePosition) override;
    void            ShortenFrom(const HGF2DLocation& pi_rNewStartPoint) override;
    void            ShortenFrom(double pi_StartRelativePosition) override;
    bool            AutoCrosses() const override;
    void            AdjustStartPointTo(const HGF2DLocation& pi_rPoint) override;
    void            AdjustEndPointTo(const HGF2DLocation& pi_rPoint) override;

    void            Drop(HGF2DLocationCollection* po_pPoint,
                                 double                   pi_Tolerance,
                                 EndPointProcessing pi_EndPointProcessing = INCLUDE_END_POINT) const override;


    // From HVE2DVector
    IMAGEPP_EXPORT HGF2DLocation
                            CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const override;
    IMAGEPP_EXPORT size_t   Intersect(const HVE2DVector& pi_rVector,
                                      HGF2DLocationCollection* po_pCrossPoints) const override;
    IMAGEPP_EXPORT size_t   ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                       HGF2DLocationCollection* po_pContiguousnessPoints) const override;
    IMAGEPP_EXPORT void     ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                         const HGF2DLocation& pi_rPoint,
                                                         HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                         HGF2DLocation* pi_pSecondContiguousnessPoint) const override;
    IMAGEPP_EXPORT HVE2DVector*
                            AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const override;

    IMAGEPP_EXPORT bool     Crosses(const HVE2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool     AreContiguous(const HVE2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool     AreAdjacent(const HVE2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool     IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                      HVE2DVector::ExtremityProcessing
                                      pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                      double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPP_EXPORT bool     IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                         HVE2DVector::ExtremityProcessing
                                         pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                         double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPP_EXPORT bool     AreContiguousAt(const HVE2DVector& pi_rVector,
                                            const HGF2DLocation& pi_rPoint) const override;
    HGFBearing      CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                             HVE2DVector::ArbitraryDirection
                                             pi_Direction = HVE2DVector::BETA) const override;
    double          CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                         HVE2DVector::ArbitraryDirection
                                                         pi_Direction = HVE2DVector::BETA) const override;
    bool            IsNull() const override;

    // From HGFGraphicObject
    HGF2DExtent     GetExtent() const override;

    IMAGEPP_EXPORT bool     AreContiguousAtAndGet(const HVE2DVector& pi_rVector,
                                                  const HGF2DLocation& pi_rPoint,
                                                  HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                  HGF2DLocation* pi_pSecondContiguousnessPoint) const override;

    // From HPMPersistentObject
    HPMPersistentObject*
                            Clone() const override;


    IMAGEPP_EXPORT void     PrintState(ostream& po_rOutput) const override;

protected:
    
    HGF2DSegment&  GetSegmentPeer() const
        {
        return (*static_cast<HGF2DSegment*>(&(GetPeer())));
        }
        
    virtual void CreatePeer() const override
        {
        m_Peer = new HGF2DSegment(GetStartPoint().GetPosition(), GetEndPoint().GetPosition());
        m_Peer->SetTolerance(GetTolerance());            
        }

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
