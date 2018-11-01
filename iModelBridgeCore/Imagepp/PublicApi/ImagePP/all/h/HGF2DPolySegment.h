//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DPolySegment.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

BEGIN_IMAGEPP_NAMESPACE
class HGF2DSegment;
class HGF2DPolygonOfSegments;
class HGF2DLiteLine;


class HGF2DPolySegment : public HGF2DBasicLinear
    {

// &&AR Remove friendship!
    friend class HVE2DPolySegment;
    friend class HVE2DPolygonOfSegments;

    HDECLARE_CLASS_ID(HGF2DPolySegmentId, HGF2DBasicLinear)

public:

    // Primary methods
    HGF2DPolySegment();
    HGF2DPolySegment(const HGF2DPosition& pi_rStartPoint,
                     const HGF2DPosition& pi_rEndPoint);

    IMAGEPP_EXPORT                    HGF2DPolySegment(const HGF2DPositionCollection& pi_rListOfPoints);

    IMAGEPP_EXPORT                    HGF2DPolySegment(size_t  pi_BufferLength,
                                               double pi_aBuffer[]);

    HGF2DPolySegment(const HGF2DPolySegment&    pi_rObject);
    IMAGEPP_EXPORT virtual            ~HGF2DPolySegment();

    HGF2DPolySegment&  operator=(const HGF2DPolySegment& pi_rObj);

    // Setting and extracting
    IMAGEPP_EXPORT void               AppendPoint(const HGF2DPosition& pi_rNewPoint);
    HGF2DPosition       GetPoint(size_t pi_Index) const;
    size_t              GetSize() const;
    IMAGEPPTEST_EXPORT void                RemovePoint(size_t pi_Index);
    void                Simplify(bool processAsClosed);


    // Parallel Copy
    IMAGEPPTEST_EXPORT HGF2DPolySegment*  AllocateParallelCopy(double pi_rOffset,
                                          HGF2DVector::ArbitraryDirection
                                          pi_DirectionToRight = HGF2DVector::BETA,
                                          const HGF2DLiteLine* pi_pFirstPointAlignment = 0,
                                          const HGF2DLiteLine* pi_pLastPointAlignment = 0) const;

    IMAGEPPTEST_EXPORT HGF2DSegment       GetClosestSegment(const HGF2DPosition& pi_rLocation) const;

    // Remove autocontiguousness points
    IMAGEPP_EXPORT void                RemoveAutoContiguousNeedles(bool pi_ClosedProcessing = false);

    // Split into non-autocrossing polysegments.

    bool               SplitIntoNonAutoCrossing(list<HFCPtr<HGF2DPolySegment> >* pio_pListOfResultPolySegments,
                                                bool pi_ProcessClosed = false) const;

// HChk &&AR : Should be moved to HGF2DLinear
    IMAGEPPTEST_EXPORT void               SortPointsAccordingToRelativePosition(HGF2DPositionCollection* pio_pListOfPointsOnLinear) const;

    // Geometry
    IMAGEPP_EXPORT virtual void       Rotate(double pi_Angle,
                                     const HGF2DPosition& pi_rOrigin);

    IMAGEPP_EXPORT bool              IsAutoContiguous() const;

    // From HGF2DBasicLinear
    IMAGEPP_EXPORT HGF2DBasicLinearTypeId  GetBasicLinearType() const override;

    // From HGF2DLinear
    IMAGEPP_EXPORT double        CalculateLength() const override;
    IMAGEPP_EXPORT HGF2DPosition CalculateRelativePoint(double pi_RelativePos) const override;
    IMAGEPP_EXPORT double        CalculateRelativePosition(const HGF2DPosition& pi_rPointOnLinear) const override;
    IMAGEPP_EXPORT double        CalculateRayArea(const HGF2DPosition& pi_rPoint) const override;
    IMAGEPP_EXPORT void          Shorten(double pi_StartRelativePos, double pi_EndRelativePos) override;
    IMAGEPP_EXPORT void          Shorten(const HGF2DPosition& pi_rNewStartPoint,
                                         const HGF2DPosition& pi_rNewEndPoint) override;
    IMAGEPP_EXPORT void          ShortenTo(const HGF2DPosition& pi_rNewEndPoint) override;
    IMAGEPP_EXPORT void          ShortenTo(double pi_EndRelativePosition) override;
    IMAGEPP_EXPORT void          ShortenFrom(const HGF2DPosition& pi_rNewStartPoint) override;
    IMAGEPP_EXPORT void          ShortenFrom(double pi_StartRelativePosition) override;
    IMAGEPP_EXPORT bool          AutoCrosses() const override;
    IMAGEPP_EXPORT virtual size_t        AutoIntersect(HGF2DPositionCollection* po_pPoints) const;
    IMAGEPP_EXPORT void          AdjustStartPointTo(const HGF2DPosition& pi_rPoint) override;
    IMAGEPP_EXPORT void          AdjustEndPointTo(const HGF2DPosition& pi_rPoint) override;
    IMAGEPP_EXPORT void          Drop(HGF2DPositionCollection* po_pPoint,
                                              double                   pi_rTolerance,
                                              EndPointProcessing       pi_EndPointProcessing = INCLUDE_END_POINT) const override;
    IMAGEPP_EXPORT void          Reverse() override;


    // From HGF2DVector
    IMAGEPP_EXPORT HGF2DPosition  CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const override;
    IMAGEPP_EXPORT size_t       Intersect(const HGF2DVector& pi_rVector,
                                                  HGF2DPositionCollection* po_pCrossPoints) const override;
    IMAGEPP_EXPORT size_t       ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                                   HGF2DPositionCollection* po_pContiguousnessPoints) const override;
    IMAGEPP_EXPORT void         ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                                     const HGF2DPosition& pi_rPoint,
                                                                     HGF2DPosition* po_pFirstContiguousnessPoint,
                                                                     HGF2DPosition* po_pSecondContiguousnessPoint) const override;

    IMAGEPP_EXPORT bool         Crosses(const HGF2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool         AreContiguous(const HGF2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool         AreAdjacent(const HGF2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool         IsPointOn(const HGF2DPosition& pi_rTestPoint,
                                                  HGF2DVector::ExtremityProcessing
                                                  pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                                  double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const override;
    IMAGEPP_EXPORT bool         AreContiguousAt(const HGF2DVector& pi_rVector,
                                                        const HGF2DPosition& pi_rPoint) const override;
    IMAGEPP_EXPORT HGFBearing   CalculateBearing(const HGF2DPosition& pi_rPositionPoint,
                                                         HGF2DVector::ArbitraryDirection
                                                         pi_Direction = HGF2DVector::BETA) const override;
    IMAGEPP_EXPORT double       CalculateAngularAcceleration(const HGF2DPosition& pi_rPositionPoint,
                                                                     HGF2DVector::ArbitraryDirection
                                                                      pi_Direction = HGF2DVector::BETA) const override;
    bool      IsNull() const override;

    // From HGFGraphicObject
    IMAGEPP_EXPORT HGF2DLiteExtent     GetExtent() const override;
    IMAGEPP_EXPORT void         Move(const HGF2DDisplacement& pi_rDisplacement) override;
    IMAGEPP_EXPORT void         Scale(double pi_ScaleFactor,
                                              const HGF2DPosition& pi_rScaleOrigin) override;

    IMAGEPP_EXPORT void Scale(double               pi_ScaleFactorX,
                              double               pi_ScaleFactorY,
                              const HGF2DPosition& pi_rScaleOrigin);

    // From HPMPersistentObject
    IMAGEPP_EXPORT HGF2DVector*     Clone() const override;

    IMAGEPP_EXPORT void         PrintState(ostream& po_rOutput) const override;

    IMAGEPP_EXPORT virtual HFCPtr<HGF2DPolySegment>    AllocPolySegmentTransformDirect(const HGF2DTransfoModel& pi_rModel) const;


// &&AR Not sure we want to decapsulate this much ... try removing!
    // Method provided to friend
    const HGF2DPositionCollection& GetPoints() const
        {
        return m_Points;
        }

    HGF2DPositionCollection& GetPoints()
        {
        return m_Points;
        }

protected:

    friend class HGF2DPolygonOfSegments;


private:

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        HASSERT(m_Points.size() == 0 || (GetStartPoint() == m_Points[0] && GetEndPoint() == m_Points.back()));
        }
#endif


    HFCPtr<HGF2DPolySegment> AllocPolySegmentTransformDirectNonLinearModel(const HGF2DTransfoModel& pi_rModel) const;

    void TransformAndAppendSegment(const HGF2DLiteSegment& pi_rSegment, const HGF2DTransfoModel& pi_rModel);




    // List of points
    HGF2DPositionCollection m_Points;

    // Acceleration attributes
    mutable bool       m_ExtentUpToDate;
    mutable HGF2DLiteExtent m_Extent;

    // Private methods
    IMAGEPP_EXPORT void               Reserve(size_t pi_PointsToPreAllocate);
    IMAGEPP_EXPORT void               MakeEmpty();
    IMAGEPPTEST_EXPORT void               ResetTolerance();
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

END_IMAGEPP_NAMESPACE

#include "HGF2DPolySegment.hpp"
