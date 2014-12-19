//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DPolySegment.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// Class : HGF2DPolySegment
//-----------------------------------------------------------------------------
// This class implements chained multi-segments basic linear
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DBasicLinear.h"
#include "HGF2DPosition.h"
#include "HGF2DLiteSegment.h"

class HGF2DSegment;
class HGF2DPolygonOfSegments;
class HGF2DLiteLine;


class HGF2DPolySegment : public HGF2DBasicLinear
    {

    HDECLARE_CLASS_ID(8315, HGF2DBasicLinear)

public:

    // Primary methods
    HGF2DPolySegment();
    HGF2DPolySegment(const HGF2DPosition& pi_rStartPoint,
                     const HGF2DPosition& pi_rEndPoint);

    _HDLLg                    HGF2DPolySegment(const HGF2DPositionCollection& pi_rListOfPoints);

    _HDLLg                    HGF2DPolySegment(size_t  pi_BufferLength,
                                               double pi_aBuffer[]);

    HGF2DPolySegment(const HGF2DPolySegment&    pi_rObject);
    _HDLLg virtual            ~HGF2DPolySegment();

    HGF2DPolySegment&  operator=(const HGF2DPolySegment& pi_rObj);

    // Setting and extracting
    void               AppendPoint(const HGF2DPosition& pi_rNewPoint);
    HGF2DPosition      GetPoint(size_t pi_Index) const;
    size_t             GetSize() const;
    void               RemovePoint(size_t pi_Index);

    // Parallel Copy
    HGF2DPolySegment*  AllocateParallelCopy(double pi_rOffset,
                                          HGF2DVector::ArbitraryDirection
                                          pi_DirectionToRight = HGF2DVector::BETA,
                                          const HGF2DLiteLine* pi_pFirstPointAlignment = 0,
                                          const HGF2DLiteLine* pi_pLastPointAlignment = 0) const;

    HGF2DSegment       GetClosestSegment(const HGF2DPosition& pi_rLocation) const;

    // Remove autocontiguousness points
    _HDLLg void                RemoveAutoContiguousNeedles(bool pi_ClosedProcessing = false);

    // Split into non-autocrossing polysegments.

    bool               SplitIntoNonAutoCrossing(list<HFCPtr<HGF2DPolySegment> >* pio_pListOfResultPolySegments,
                                                bool pi_ProcessClosed = false) const;

// HChk AR : Should be moved to HGF2DLinear
    void               SortPointsAccordingToRelativePosition(HGF2DPositionCollection* pio_pListOfPointsOnLinear) const;

    // Geometry
    _HDLLg virtual void       Rotate(double pi_Angle,
                                     const HGF2DPosition& pi_rOrigin);

    _HDLLg bool              IsAutoContiguous() const;

    // From HGF2DBasicLinear
    _HDLLg HGF2DBasicLinearTypeId  GetBasicLinearType() const;

    // From HGF2DLinear
    _HDLLg virtual double        CalculateLength() const;
    _HDLLg virtual HGF2DPosition CalculateRelativePoint(double pi_RelativePos) const;
    _HDLLg virtual double        CalculateRelativePosition(const HGF2DPosition& pi_rPointOnLinear) const;
    _HDLLg virtual double        CalculateRayArea(const HGF2DPosition& pi_rPoint) const;
    _HDLLg virtual void          Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
    _HDLLg virtual void          Shorten(const HGF2DPosition& pi_rNewStartPoint,
                                         const HGF2DPosition& pi_rNewEndPoint);
    _HDLLg virtual void          ShortenTo(const HGF2DPosition& pi_rNewEndPoint);
    _HDLLg virtual void          ShortenTo(double pi_EndRelativePosition);
    _HDLLg virtual void          ShortenFrom(const HGF2DPosition& pi_rNewStartPoint);
    _HDLLg virtual void          ShortenFrom(double pi_StartRelativePosition);
    _HDLLg virtual bool          AutoCrosses() const;
    _HDLLg virtual size_t        AutoIntersect(HGF2DPositionCollection* po_pPoints) const;
    _HDLLg virtual void          AdjustStartPointTo(const HGF2DPosition& pi_rPoint);
    _HDLLg virtual void          AdjustEndPointTo(const HGF2DPosition& pi_rPoint);
    _HDLLg virtual void          Drop(HGF2DPositionCollection* po_pPoint,
                                      double                   pi_rTolerance,
                                      EndPointProcessing       pi_EndPointProcessing = INCLUDE_END_POINT) const;
    _HDLLg virtual void          Reverse();


    // From HGF2DVector
    _HDLLg virtual HGF2DPosition  CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
    _HDLLg virtual size_t     Intersect(const HGF2DVector& pi_rVector,
                                        HGF2DPositionCollection* po_pCrossPoints) const;
    _HDLLg virtual size_t     ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                         HGF2DPositionCollection* po_pContiguousnessPoints) const;
    _HDLLg virtual void       ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                           const HGF2DPosition& pi_rPoint,
                                                           HGF2DPosition* po_pFirstContiguousnessPoint,
                                                           HGF2DPosition* po_pSecondContiguousnessPoint) const;

    _HDLLg virtual bool      Crosses(const HGF2DVector& pi_rVector) const;
    _HDLLg virtual bool      AreContiguous(const HGF2DVector& pi_rVector) const;
    _HDLLg virtual bool      AreAdjacent(const HGF2DVector& pi_rVector) const;
    _HDLLg virtual bool      IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                        HGF2DVector::ExtremityProcessing
                                        pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                        double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    _HDLLg virtual bool      AreContiguousAt(const HGF2DVector& pi_rVector,
                                              const HGF2DPosition& pi_rPoint) const;
    _HDLLg virtual HGFBearing   CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                                                   HGF2DVector::ArbitraryDirection
                                                   pi_Direction = HGF2DVector::BETA) const;
    virtual double    CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                                                   HGF2DVector::ArbitraryDirection
                                                                   pi_Direction = HGF2DVector::BETA) const;
    virtual bool      IsNull() const;

    // From HGFGraphicObject
    _HDLLg virtual HGF2DLiteExtent     GetExtent() const;
    _HDLLg virtual void       Move(const HGF2DDisplacement& pi_rDisplacement);
    _HDLLg virtual void       Scale(double pi_ScaleFactor,
                                    const HGF2DPosition& pi_rScaleOrigin);

    // From HPMPersistentObject
    _HDLLg virtual HGF2DVector*     Clone() const;

    _HDLLg virtual void       PrintState(ostream& po_rOutput) const;

protected:


private:
    friend class HGF2DPolygonOfSegments;

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        HASSERT(m_Points.size() == 0 || (m_StartPoint == m_Points[0] && m_EndPoint == m_Points.back()));
        }
#endif


    // List of points
    HGF2DPositionCollection m_Points;

    // Acceleration attributes
    mutable bool       m_ExtentUpToDate;
    mutable HGF2DLiteExtent m_Extent;

    // Private methods
    _HDLLg void               Reserve(size_t pi_PointsToPreAllocate);
    _HDLLg void               MakeEmpty();
    void               ResetTolerance();
    bool IsContiguousToPolySegment(const HGF2DPolySegment& pi_rPolySegment) const;
    bool IsContiguousToSegment(const HGF2DSegment& pi_rSegment) const;
    bool IsContiguousToPolySegmentAt(const HGF2DPolySegment& pi_rPolySegment,
                                      const HGF2DPosition&    pi_rPoint) const;
    bool IsContiguousToSegmentAt(const HGF2DSegment&  pi_rSegment,
                                  const HGF2DPosition& pi_rPoint) const;
    bool CrossesPolySegment(const HGF2DPolySegment& pi_rPolySegment) const;
    bool CrossesSegment(const HGF2DSegment& pi_rSegment) const;
    bool IsAdjacentToPolySegment(const HGF2DPolySegment& pi_rPolySegment) const;
    bool IsAdjacentToSegment(const HGF2DSegment& pi_rSegment) const;
    size_t IntersectSegment(const HGF2DSegment& pi_rSegment,
                            HGF2DPositionCollection* po_pCrossPoints) const;
    size_t IntersectPolySegment(const HGF2DPolySegment& pi_rPolySegment,
                                HGF2DPositionCollection* po_pCrossPoints) const;
    size_t ObtainContiguousnessPointsWithPolySegment(const HGF2DPolySegment&  pi_rPolySegment,
                                                     HGF2DPositionCollection* po_pContiguousnessPoints) const;
    size_t ObtainContiguousnessPointsWithSegment(const HGF2DSegment&  pi_rSegment,
                                                 HGF2DPositionCollection* po_pContiguousnessPoints) const;
    void ObtainContiguousnessPointsWithPolySegmentAt(const HGF2DPolySegment&  pi_rPolySegment,
                                                     const HGF2DPosition& pi_rPoint,
                                                     HGF2DPosition* po_pFirstContiguousnessPoint,
                                                     HGF2DPosition* po_pSecondContiguousnessPoint) const;
    void ObtainContiguousnessPointsWithSegmentAt(const HGF2DSegment&  pi_rSegment,
                                                 const HGF2DPosition& pi_rPoint,
                                                 HGF2DPosition* po_pFirstContiguousnessPoints,
                                                 HGF2DPosition* po_pSecondContiguousnessPoints) const;
    bool        IntersectsAtAnySplitPointWithLiteSegment(const HGF2DLiteSegment& pi_rSegment) const;
    bool        IntersectsAtAnySplitPointWithPolySegment(const HGF2DPolySegment& pi_rPolySegment) const;

    bool        IntersectsAtSplitPointWithPolySegment(const HGF2DPolySegment& pi_rVector,
                                                       const HGF2DPosition&    pi_rTestPoint,
                                                       const HGF2DPosition&    pi_rNextEndPoint,
                                                       bool             pi_ProcessNext) const;

    void RecomposeClosedPolySegments(list<HFCPtr<HGF2DPolySegment> >* pio_pListOfResultPolySegments) const;

    };


#include "HGF2DPolySegment.hpp"
