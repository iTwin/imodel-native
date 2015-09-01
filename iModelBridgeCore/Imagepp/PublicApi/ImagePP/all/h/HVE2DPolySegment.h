//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DPolySegment.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// Class : HVE2DPolySegment
//-----------------------------------------------------------------------------
// This class implements chained multi-segments basic linear
//-----------------------------------------------------------------------------
#pragma once

#include "HVE2DBasicLinear.h"
#include "HGFBearing.h"
#include "HGF2DLocation.h"
#include "HGF2DPosition.h"
#include "HGF2DLiteSegment.h"

BEGIN_IMAGEPP_NAMESPACE
class HVE2DSegment;
class HGF2DCoordSys;
class HVE2DPolygonOfSegments;
class HGF2DLine;


class HVE2DPolySegment : public HVE2DBasicLinear
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DPolySegmentId)


public:

    // Primary methods
    HVE2DPolySegment();
    HVE2DPolySegment(const HGF2DLocation& pi_rStartPoint,
                     const HGF2DLocation& pi_rEndPoint);
    HVE2DPolySegment(const HGF2DLocationCollection& pi_rListOfPoints);
    IMAGEPP_EXPORT HVE2DPolySegment(const HGF2DPositionCollection& pi_rListOfPoints,
                     const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DPolySegment(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT                    HVE2DPolySegment(size_t  pi_BufferLength,
                                               double pi_aBuffer[],
                                               const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DPolySegment(const HVE2DPolySegment&    pi_rObject);
    IMAGEPP_EXPORT virtual            ~HVE2DPolySegment();

    HVE2DPolySegment&  operator=(const HVE2DPolySegment& pi_rObj);

    // Setting and extracting
    IMAGEPP_EXPORT void               AppendPoint(const HGF2DLocation& pi_rNewPoint);
    IMAGEPP_EXPORT void               AppendPosition(const HGF2DPosition& pi_rNewPoint);
    HGF2DLocation      GetPoint(size_t pi_Index) const;
    const HGF2DPosition&
    GetPosition(size_t pi_Index) const;
    size_t             GetSize() const;
    void               RemovePoint(size_t pi_Index);

    // Parallel Copy
    HVE2DPolySegment*  AllocateParallelCopy(double                             pi_Offset,
                                            HVE2DVector::ArbitraryDirection    pi_DirectionToRight = HVE2DVector::BETA,
                                            const HGF2DLine*                   pi_pFirstPointAlignment = 0,
                                            const HGF2DLine*                   pi_pLastPointAlignment = 0) const;

    HVE2DSegment       GetClosestSegment(const HGF2DLocation& pi_rLocation) const;

    // Remove autocontiguousness points
    IMAGEPP_EXPORT void        RemoveAutoContiguousNeedles(bool pi_ClosedProcessing = false);

    // Split into non-autocrossing polysegments.

    bool               SplitIntoNonAutoCrossing(list<HFCPtr<HVE2DPolySegment> >* pio_pListOfResultPolySegments,
                                                bool pi_ProcessClosed = false) const;

// HChk AR : Should be moved to HGF2DLinear
    void               SortPointsAccordingToRelativePosition(HGF2DLocationCollection* pio_pListOfPointsOnLinear) const;

    // Geometry
    IMAGEPP_EXPORT virtual void       Rotate(double               pi_Angle,
                                     const HGF2DLocation& pi_rOrigin);

    IMAGEPP_EXPORT bool               IsAutoContiguous() const;

    // From HVE2DBasicLinear
    IMAGEPP_EXPORT HVE2DBasicLinearTypeId    
                              GetBasicLinearType() const;

    // From HVE2DLinear
    IMAGEPP_EXPORT virtual double     CalculateLength() const;
    IMAGEPP_EXPORT virtual HGF2DLocation    
                              CalculateRelativePoint(double pi_RelativePos) const;
    IMAGEPP_EXPORT virtual double     CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const;
    IMAGEPP_EXPORT virtual double     CalculateRayArea(const HGF2DLocation& pi_rPoint) const;
    IMAGEPP_EXPORT virtual void       Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
    IMAGEPP_EXPORT virtual void       Shorten(const HGF2DLocation& pi_rNewStartPoint,
                                      const HGF2DLocation& pi_rNewEndPoint);
    IMAGEPP_EXPORT virtual void       ShortenTo(const HGF2DLocation& pi_rNewEndPoint);
    IMAGEPP_EXPORT virtual void       ShortenTo(double pi_EndRelativePosition);
    IMAGEPP_EXPORT virtual void       ShortenFrom(const HGF2DLocation& pi_rNewStartPoint);
    IMAGEPP_EXPORT virtual void       ShortenFrom(double pi_StartRelativePosition);
    IMAGEPP_EXPORT virtual bool       AutoCrosses() const;
    IMAGEPP_EXPORT virtual size_t     AutoIntersect(HGF2DLocationCollection* po_pPoints) const;
    IMAGEPP_EXPORT virtual void       AdjustStartPointTo(const HGF2DLocation& pi_rPoint);
    IMAGEPP_EXPORT virtual void       AdjustEndPointTo(const HGF2DLocation& pi_rPoint);

    IMAGEPP_EXPORT virtual void       Drop(HGF2DLocationCollection* po_pPoint,
                                   double                   pi_Tolerance,
                                   EndPointProcessing       pi_EndPointProcessing = INCLUDE_END_POINT) const;
    IMAGEPP_EXPORT virtual void       Reverse();


    // From HVE2DVector
    IMAGEPP_EXPORT virtual HGF2DLocation
                              CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    IMAGEPP_EXPORT virtual size_t     Intersect(const HVE2DVector& pi_rVector,
                                        HGF2DLocationCollection* po_pCrossPoints) const;
    IMAGEPP_EXPORT virtual size_t     ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                         HGF2DLocationCollection* po_pContiguousnessPoints) const;
    IMAGEPP_EXPORT virtual void       ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                           const HGF2DLocation& pi_rPoint,
                                                           HGF2DLocation* po_pFirstContiguousnessPoint,
                                                           HGF2DLocation* po_pSecondContiguousnessPoint) const;
    IMAGEPP_EXPORT virtual HVE2DVector*
                              AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;

    IMAGEPP_EXPORT virtual bool       Crosses(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool       AreContiguous(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool       AreAdjacent(const HVE2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual bool       IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                        HVE2DVector::ExtremityProcessing
                                        pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                        double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual bool       IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                           HVE2DVector::ExtremityProcessing
                                           pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                           double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    IMAGEPP_EXPORT virtual bool       AreContiguousAt(const HVE2DVector& pi_rVector,
                                              const HGF2DLocation& pi_rPoint) const;
    IMAGEPP_EXPORT virtual HGFBearing CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                               HVE2DVector::ArbitraryDirection
                                               pi_Direction = HVE2DVector::BETA) const;
    virtual double            CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                           HVE2DVector::ArbitraryDirection
                                                           pi_Direction = HVE2DVector::BETA) const;
    virtual bool              IsNull() const;

    // From HGFGraphicObject
    IMAGEPP_EXPORT virtual HGF2DExtent
                              GetExtent() const;

    IMAGEPP_EXPORT virtual void       Move(const HGF2DDisplacement& pi_rDisplacement);
    IMAGEPP_EXPORT virtual void       Scale(double pi_ScaleFactor,
                                    const HGF2DLocation& pi_rScaleOrigin);

    // From HPMPersistentObject
    IMAGEPP_EXPORT virtual HPMPersistentObject*
                              Clone() const;


    IMAGEPP_EXPORT virtual void       PrintState(ostream& po_rOutput) const;

protected:

    IMAGEPP_EXPORT virtual void       SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);


private:
    friend class HVE2DPolygonOfSegments;

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        HASSERT(m_Points.size() == 0 || (m_StartPoint.GetPosition() == m_Points[0] && m_EndPoint.GetPosition() == m_Points.back()));
        }
#endif


    // List of points
    HGF2DPositionCollection m_Points;

    // Acceleration attributes
    mutable bool        m_ExtentUpToDate;
    mutable HGF2DExtent m_Extent;

    // Private methods
    IMAGEPP_EXPORT void               Reserve(size_t pi_PointsToPreAllocate);
    IMAGEPP_EXPORT void               MakeEmpty();
    void                      ResetTolerance();
    bool IsContiguousToPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const;
    bool IsContiguousToPolySegmentLRCS(const HVE2DPolySegment& pi_rPolySegment) const;
    bool IsContiguousToSegmentSCS(const HVE2DSegment& pi_rSegment) const;
    bool IsContiguousToSegmentLRCS(const HVE2DSegment& pi_rSegment) const;
    bool IsContiguousToPolySegment(const HVE2DPolySegment& pi_rPolySegment) const;
    bool IsContiguousToSegment(const HVE2DSegment& pi_rSegment) const;
    bool IsContiguousToPolySegmentAtSCS(const HVE2DPolySegment& pi_rPolySegment,
                                         const HGF2DLocation&    pi_rPoint) const;
    bool IsContiguousToPolySegmentAtLRCS(const HVE2DPolySegment& pi_rPolySegment,
                                          const HGF2DLocation&    pi_rPoint) const;
    bool IsContiguousToSegmentAtSCS(const HVE2DSegment&  pi_rSegment,
                                     const HGF2DLocation& pi_rPoint) const;
    bool IsContiguousToSegmentAtLRCS(const HVE2DSegment&  pi_rSegment,
                                      const HGF2DLocation& pi_rPoint) const;
    bool IsContiguousToPolySegmentAt(const HVE2DPolySegment& pi_rPolySegment,
                                      const HGF2DLocation&    pi_rPoint) const;
    bool IsContiguousToSegmentAt(const HVE2DSegment&  pi_rSegment,
                                  const HGF2DLocation& pi_rPoint) const;
    bool CrossesPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const;
    bool CrossesPolySegmentDPCS(const HVE2DPolySegment& pi_rPolySegment) const;
    bool CrossesSegmentSCS(const HVE2DSegment& pi_rSegment) const;
    bool CrossesSegmentDPCS(const HVE2DSegment& pi_rSegment) const;
    bool CrossesPolySegment(const HVE2DPolySegment& pi_rPolySegment) const;
    bool CrossesSegment(const HVE2DSegment& pi_rSegment) const;
    bool IsAdjacentToPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const;
    bool IsAdjacentToPolySegmentLRCS(const HVE2DPolySegment& pi_rPolySegment) const;
    bool IsAdjacentToSegmentSCS(const HVE2DSegment& pi_rSegment) const;
    bool IsAdjacentToSegmentLRCS(const HVE2DSegment& pi_rSegment) const;
    bool IsAdjacentToPolySegment(const HVE2DPolySegment& pi_rPolySegment) const;
    bool IsAdjacentToSegment(const HVE2DSegment& pi_rSegment) const;
    size_t IntersectSegmentSCS(const HVE2DSegment& pi_rSegment,
                               HGF2DLocationCollection* po_pCrossPoints) const;
    size_t IntersectSegmentDPCS(const HVE2DSegment& pi_rSegment,
                                HGF2DLocationCollection* po_pCrossPoints) const;
    size_t IntersectPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment,
                                   HGF2DLocationCollection* po_pCrossPoints) const;
    size_t IntersectPolySegmentDPCS(const HVE2DPolySegment& pi_rPolySegment,
                                    HGF2DLocationCollection* po_pCrossPoints) const;
    size_t IntersectSegment(const HVE2DSegment& pi_rSegment,
                            HGF2DLocationCollection* po_pCrossPoints) const;
    size_t IntersectPolySegment(const HVE2DPolySegment& pi_rPolySegment,
                                HGF2DLocationCollection* po_pCrossPoints) const;
    size_t ObtainContiguousnessPointsWithSegmentSCS(const HVE2DSegment&  pi_rSegment,
                                                    HGF2DLocationCollection* po_pContiguousnessPoints) const;
    size_t ObtainContiguousnessPointsWithSegmentLRCS(const HVE2DSegment&  pi_rSegment,
                                                     HGF2DLocationCollection* po_pContiguousnessPoints) const;
    size_t ObtainContiguousnessPointsWithPolySegmentSCS(const HVE2DPolySegment&  pi_rPolySegment,
                                                        HGF2DLocationCollection* po_pContiguousnessPoints) const;
    size_t ObtainContiguousnessPointsWithPolySegmentLRCS(const HVE2DPolySegment&  pi_rPolySegment,
                                                         HGF2DLocationCollection* po_pContiguousnessPoints) const;
    size_t ObtainContiguousnessPointsWithPolySegment(const HVE2DPolySegment&  pi_rPolySegment,
                                                     HGF2DLocationCollection* po_pContiguousnessPoints) const;
    size_t ObtainContiguousnessPointsWithSegment(const HVE2DSegment&  pi_rSegment,
                                                 HGF2DLocationCollection* po_pContiguousnessPoints) const;
    void ObtainContiguousnessPointsWithSegmentAtSCS(const HVE2DSegment&  pi_rSegment,
                                                    const HGF2DLocation& pi_rPoint,
                                                    HGF2DLocation* po_pFirstContiguousnessPoint,
                                                    HGF2DLocation* po_pSecondContiguousnessPoint) const;
    void ObtainContiguousnessPointsWithSegmentAtLRCS(const HVE2DSegment&  pi_rSegment,
                                                     const HGF2DLocation& pi_rPoint,
                                                     HGF2DLocation* po_pFirstContiguousnessPoint,
                                                     HGF2DLocation* po_pSecondContiguousnessPoint) const;
    void ObtainContiguousnessPointsWithPolySegmentAtSCS(const HVE2DPolySegment&  pi_rPolySegment,
                                                        const HGF2DLocation& pi_rPoint,
                                                        HGF2DLocation* po_pFirstContiguousnessPoint,
                                                        HGF2DLocation* po_pSecondContiguousnessPoint) const;
    void ObtainContiguousnessPointsWithPolySegmentAtLRCS(const HVE2DPolySegment&  pi_rPolySegment,
                                                         const HGF2DLocation& pi_rPoint,
                                                         HGF2DLocation* po_pFirstContiguousnessPoint,
                                                         HGF2DLocation* po_pSecondContiguousnessPoint) const;
    void ObtainContiguousnessPointsWithPolySegmentAt(const HVE2DPolySegment&  pi_rPolySegment,
                                                     const HGF2DLocation& pi_rPoint,
                                                     HGF2DLocation* po_pFirstContiguousnessPoint,
                                                     HGF2DLocation* po_pSecondContiguousnessPoint) const;
    void ObtainContiguousnessPointsWithSegmentAt(const HVE2DSegment&  pi_rSegment,
                                                 const HGF2DLocation& pi_rPoint,
                                                 HGF2DLocation* po_pFirstContiguousnessPoints,
                                                 HGF2DLocation* po_pSecondContiguousnessPoints) const;
    void AppendSegmentInCoordSys(const HVE2DSegment& pi_rSegment);
    HVE2DVector* AllocateCopyInComplexCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;

    bool        IntersectsAtAnySplitPointWithLiteSegmentSCS(const HGF2DLiteSegment& pi_rSegment) const;
    bool        IntersectsAtAnySplitPointWithPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const;

    bool        IntersectsAtSplitPointWithPolySegment(const HVE2DPolySegment& pi_rVector,
                                                       const HGF2DLocation&    pi_rTestPoint,
                                                       const HGF2DLocation&    pi_rNextEndPoint,
                                                       bool             pi_ProcessNext) const;

    void RecomposeClosedPolySegments(list<HFCPtr<HVE2DPolySegment> >* pio_pListOfResultPolySegments) const;

    };
END_IMAGEPP_NAMESPACE


#include "HVE2DPolySegment.hpp"
