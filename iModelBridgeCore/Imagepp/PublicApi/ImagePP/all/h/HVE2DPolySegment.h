//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
#include "HGF2DPolySegment.h"

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
    HVE2DPolySegment(HGF2DPolySegment* peerPolySegment, const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);


    IMAGEPP_EXPORT                    HVE2DPolySegment(size_t  pi_BufferLength,
                                               double pi_aBuffer[],
                                               const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DPolySegment(const HVE2DPolySegment&    pi_rObject);
    IMAGEPP_EXPORT virtual            ~HVE2DPolySegment();

    HVE2DPolySegment&  operator=(const HVE2DPolySegment& pi_rObj);

    // Setting and extracting
    IMAGEPP_EXPORT void               AppendPoint(const HGF2DLocation& pi_rNewPoint);
    IMAGEPP_EXPORT void               AppendPosition(const HGF2DPosition& pi_rNewPoint);
    HGF2DLocation       GetPoint(size_t pi_Index) const;
    HGF2DPosition       GetPosition(size_t pi_Index) const;
    size_t              GetSize() const;
    IMAGEPPTEST_EXPORT void                RemovePoint(size_t pi_Index);

    void                Simplify(bool processAsClosed);

    // Parallel Copy
    IMAGEPPTEST_EXPORT HVE2DPolySegment*  AllocateParallelCopy(double                             pi_Offset,
                                            HVE2DVector::ArbitraryDirection    pi_DirectionToRight = HVE2DVector::BETA,
                                            const HGF2DLine*                   pi_pFirstPointAlignment = 0,
                                            const HGF2DLine*                   pi_pLastPointAlignment = 0) const;

    IMAGEPPTEST_EXPORT HVE2DSegment       GetClosestSegment(const HGF2DLocation& pi_rLocation) const;

    // Remove autocontiguousness points
    IMAGEPP_EXPORT void        RemoveAutoContiguousNeedles(bool pi_ClosedProcessing = false);

    // Split into non-autocrossing polysegments.

    bool               SplitIntoNonAutoCrossing(list<HFCPtr<HVE2DPolySegment> >* pio_pListOfResultPolySegments,
                                                bool pi_ProcessClosed = false) const;

// HChk AR : Should be moved to HGF2DLinear
    IMAGEPPTEST_EXPORT void               SortPointsAccordingToRelativePosition(HGF2DLocationCollection* pio_pListOfPointsOnLinear) const;

    // Geometry
    IMAGEPP_EXPORT virtual void       Rotate(double               pi_Angle,
                                     const HGF2DLocation& pi_rOrigin);

    IMAGEPP_EXPORT bool               IsAutoContiguous() const;

    // From HVE2DBasicLinear
    IMAGEPP_EXPORT HVE2DBasicLinearTypeId    
                              GetBasicLinearType() const override;

    // From HVE2DLinear
    IMAGEPP_EXPORT double     CalculateLength() const override;
    IMAGEPP_EXPORT HGF2DLocation    
                              CalculateRelativePoint(double pi_RelativePos) const override;
    IMAGEPP_EXPORT double     CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const override;
    IMAGEPP_EXPORT double     CalculateRayArea(const HGF2DLocation& pi_rPoint) const override;
    IMAGEPP_EXPORT void       Shorten(double pi_StartRelativePos, double pi_EndRelativePos) override;
    IMAGEPP_EXPORT void       Shorten(const HGF2DLocation& pi_rNewStartPoint,
                                      const HGF2DLocation& pi_rNewEndPoint) override;
    IMAGEPP_EXPORT void       ShortenTo(const HGF2DLocation& pi_rNewEndPoint) override;
    IMAGEPP_EXPORT void       ShortenTo(double pi_EndRelativePosition) override;
    IMAGEPP_EXPORT void       ShortenFrom(const HGF2DLocation& pi_rNewStartPoint) override;
    IMAGEPP_EXPORT void       ShortenFrom(double pi_StartRelativePosition) override;
    IMAGEPP_EXPORT bool       AutoCrosses() const override;
    IMAGEPP_EXPORT virtual size_t     AutoIntersect(HGF2DLocationCollection* po_pPoints) const;
    IMAGEPP_EXPORT void       AdjustStartPointTo(const HGF2DLocation& pi_rPoint) override;
    IMAGEPP_EXPORT void       AdjustEndPointTo(const HGF2DLocation& pi_rPoint) override;

    IMAGEPP_EXPORT void       Drop(HGF2DLocationCollection* po_pPoint,
                                   double                   pi_Tolerance,
                                   EndPointProcessing       pi_EndPointProcessing = INCLUDE_END_POINT) const override;
    IMAGEPP_EXPORT void       Reverse() override;


    // From HVE2DVector
    IMAGEPP_EXPORT HGF2DLocation
                              CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const override;
    IMAGEPP_EXPORT size_t     Intersect(const HVE2DVector& pi_rVector,
                                        HGF2DLocationCollection* po_pCrossPoints) const override;
    IMAGEPP_EXPORT size_t     ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                         HGF2DLocationCollection* po_pContiguousnessPoints) const override;
    IMAGEPP_EXPORT void       ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                           const HGF2DLocation& pi_rPoint,
                                                           HGF2DLocation* po_pFirstContiguousnessPoint,
                                                           HGF2DLocation* po_pSecondContiguousnessPoint) const override;
    IMAGEPP_EXPORT HVE2DVector*
                              AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const override;

    IMAGEPP_EXPORT bool       Crosses(const HVE2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool       AreContiguous(const HVE2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool       AreAdjacent(const HVE2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool       IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                        HVE2DVector::ExtremityProcessing
                                        pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                        double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPP_EXPORT bool       IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                           HVE2DVector::ExtremityProcessing
                                           pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                           double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPP_EXPORT bool       AreContiguousAt(const HVE2DVector& pi_rVector,
                                              const HGF2DLocation& pi_rPoint) const override;
    IMAGEPP_EXPORT HGFBearing CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                               HVE2DVector::ArbitraryDirection
                                               pi_Direction = HVE2DVector::BETA) const override;
    double            CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                           HVE2DVector::ArbitraryDirection
                                                           pi_Direction = HVE2DVector::BETA) const override;
    bool              IsNull() const override;

    // From HGFGraphicObject
    IMAGEPP_EXPORT HGF2DExtent
                              GetExtent() const override;

    IMAGEPP_EXPORT void       Move(const HGF2DDisplacement& pi_rDisplacement) override;
    IMAGEPP_EXPORT void       Scale(double pi_ScaleFactor,
                                    const HGF2DLocation& pi_rScaleOrigin) override;


    IMAGEPP_EXPORT void Scale(double               pi_ScaleFactorX,
                              double               pi_ScaleFactorY,
                              const HGF2DLocation& pi_rScaleOrigin);

    // From HPMPersistentObject
    IMAGEPP_EXPORT HPMPersistentObject*
                              Clone() const override;


    IMAGEPP_EXPORT void       PrintState(ostream& po_rOutput) const override;

protected:

    IMAGEPP_EXPORT void       SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys) override;


    HGF2DPolySegment&  GetPolySegmentPeer() const
        {
        return (*static_cast<HGF2DPolySegment*>(&(GetPeer())));
        }
                        
    void CreatePolySegmentPeer(const HGF2DPositionCollection& m_Points)
        {
        m_Peer = new HGF2DPolySegment(m_Points);
        }
        
    void CreateEmptyPolySegmentPeer()
        {
        m_Peer = new HGF2DPolySegment();
        }
        
    void CreateCopyPolySegmentPeer(const HGF2DPolySegment& polySegment)
        {
        m_Peer = new HGF2DPolySegment(polySegment);
        }        
                
    virtual void CreatePeer() const override
        {
        HASSERT(!"The polysegment peer should have been created at construction");
        }

private:
    friend class HVE2DPolygonOfSegments;

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        HASSERT(m_Peer != nullptr);

        HASSERT(m_Peer->GetTolerance() == GetTolerance());

        HASSERT(m_Peer->IsAutoToleranceActive() == IsAutoToleranceActive());

        HASSERT(GetPoints().size() == 0 || (GetStartPoint().GetPosition() == GetPoints()[0] && GetEndPoint().GetPosition() == GetPoints().back()));

        HASSERT(GetPoints().size() == GetPolySegmentPeer().GetSize());

        HASSERT(GetPolySegmentPeer().GetSize() == 0 || (GetStartPoint().GetPosition() == GetPolySegmentPeer().GetStartPoint() && GetEndPoint().GetPosition() == GetPolySegmentPeer().GetEndPoint()));

        HASSERT(!m_VolatilePeer);

        }
#endif

    // Method provided to friend
	
	// &&AR Not sure we need those
    const HGF2DPositionCollection& GetPoints() const
        {
        return GetPolySegmentPeer().GetPoints();
        }

    HGF2DPositionCollection& GetPoints()
        {
        return GetPolySegmentPeer().GetPoints();
        }

    // List of points

    // Acceleration attributes
    mutable bool        m_ExtentUpToDate;
    mutable HGF2DExtent m_Extent;

    // Private methods
    IMAGEPP_EXPORT void               Reserve(size_t pi_PointsToPreAllocate);
    IMAGEPP_EXPORT void               MakeEmpty();
    IMAGEPPTEST_EXPORT void                      ResetTolerance();
    bool IsContiguousToPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const;
    bool IsContiguousToSegmentSCS(const HVE2DSegment& pi_rSegment) const;
    bool IsContiguousToPolySegment(const HVE2DPolySegment& pi_rPolySegment) const;
    bool IsContiguousToSegment(const HVE2DSegment& pi_rSegment) const;
    bool IsContiguousToPolySegmentAtSCS(const HVE2DPolySegment& pi_rPolySegment,
                                         const HGF2DLocation&    pi_rPoint) const;
    bool IsContiguousToSegmentAtSCS(const HVE2DSegment&  pi_rSegment,
                                     const HGF2DLocation& pi_rPoint) const;
    bool IsContiguousToPolySegmentAt(const HVE2DPolySegment& pi_rPolySegment,
                                      const HGF2DLocation&    pi_rPoint) const;
    bool IsContiguousToSegmentAt(const HVE2DSegment&  pi_rSegment,
                                  const HGF2DLocation& pi_rPoint) const;
    bool CrossesPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const;
    bool CrossesSegmentSCS(const HVE2DSegment& pi_rSegment) const;
    bool CrossesPolySegment(const HVE2DPolySegment& pi_rPolySegment) const;
    bool CrossesSegment(const HVE2DSegment& pi_rSegment) const;
    bool IsAdjacentToPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const;
    bool IsAdjacentToSegmentSCS(const HVE2DSegment& pi_rSegment) const;
    bool IsAdjacentToPolySegment(const HVE2DPolySegment& pi_rPolySegment) const;
    bool IsAdjacentToSegment(const HVE2DSegment& pi_rSegment) const;
    size_t IntersectSegmentSCS(const HVE2DSegment& pi_rSegment,
                               HGF2DLocationCollection* po_pCrossPoints) const;
    size_t IntersectPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment,
                                   HGF2DLocationCollection* po_pCrossPoints) const;
    size_t IntersectSegment(const HVE2DSegment& pi_rSegment,
                            HGF2DLocationCollection* po_pCrossPoints) const;
    size_t IntersectPolySegment(const HVE2DPolySegment& pi_rPolySegment,
                                HGF2DLocationCollection* po_pCrossPoints) const;
    size_t ObtainContiguousnessPointsWithSegmentSCS(const HVE2DSegment&  pi_rSegment,
                                                    HGF2DLocationCollection* po_pContiguousnessPoints) const;
    size_t ObtainContiguousnessPointsWithPolySegmentSCS(const HVE2DPolySegment&  pi_rPolySegment,
                                                        HGF2DLocationCollection* po_pContiguousnessPoints) const;
    size_t ObtainContiguousnessPointsWithPolySegment(const HVE2DPolySegment&  pi_rPolySegment,
                                                     HGF2DLocationCollection* po_pContiguousnessPoints) const;
    size_t ObtainContiguousnessPointsWithSegment(const HVE2DSegment&  pi_rSegment,
                                                 HGF2DLocationCollection* po_pContiguousnessPoints) const;
    void ObtainContiguousnessPointsWithSegmentAtSCS(const HVE2DSegment&  pi_rSegment,
                                                    const HGF2DLocation& pi_rPoint,
                                                    HGF2DLocation* po_pFirstContiguousnessPoint,
                                                    HGF2DLocation* po_pSecondContiguousnessPoint) const;
    void ObtainContiguousnessPointsWithPolySegmentAtSCS(const HVE2DPolySegment&  pi_rPolySegment,
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
