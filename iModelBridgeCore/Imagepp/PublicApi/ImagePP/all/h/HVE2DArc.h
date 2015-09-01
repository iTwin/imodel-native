//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DArc.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DArc
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DBasicLinear.h"
#include "HGFBearing.h"
#include "HGF2DLocation.h"

BEGIN_IMAGEPP_NAMESPACE
class HVE2DComplexLinear;
class HVE2DSegment;
class HGF2DLine;
class HVE2DCircle;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a basic linear representing an arc of circle
    Like any linear, an arc is constituted of a start point, an end point
    and a path into space linking these two points. The nature of the arc
    specifies that the path between the two points must be located on a circle
    upon which the two points are located. The exact curvature of the circle and
    thus the arc is specified by additional parameters such as a third point
    belonging to the arc or a radius and a direction of rotation and so on. Many
    different properties may be imposed to specify the arc.
    -----------------------------------------------------------------------------
*/
class HVE2DArc : public HVE2DBasicLinear
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DArcId)

public:

    // Primary methods
    HVE2DArc();
    HVE2DArc(const HGF2DLocation& pi_rStartPoint,
             const HGF2DLocation& pi_rMiddlePoint,
             const HGF2DLocation& pi_rEndPoint);
    HVE2DArc(const HGF2DLocation& pi_rCenter,
             const HGFBearing&    pi_rStartBearing,
             double               pi_Sweep,
             double               pi_rRadius);
    HVE2DArc(const HGF2DLocation& pi_rCenter,
             const HGFBearing&    pi_rStartBearing,
             const HGFBearing&    pi_rEndBearing,
             double               pi_rRadius,
             HGFAngle::AngleDirection pi_Direction);
    HVE2DArc(const HVE2DCircle&   pi_rCircle,
             const HGFBearing&    pi_rStartBearing,
             double               pi_Sweep);
    HVE2DArc(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DArc(const HVE2DArc&      pi_rObject);
    virtual            ~HVE2DArc();

    HVE2DArc&          operator=(const HVE2DArc& pi_rObj);

    // Information extraction
    double                  CalculateSweep() const;
    HGFBearing              CalculateStartBearing() const;
    HGFBearing              CalculateEndBearing() const;
    const HGF2DLocation&    GetCenter() const;
    double                  CalculateRadius() const;

    // Setting
    void               SetByPoints(const HGF2DLocation& pi_rStartPoint,
                                   const HGF2DLocation& pi_rMiddlePoint,
                                   const HGF2DLocation& pi_rEndPoint);

    void               SetByCenterAndSweep(const HGF2DLocation& pi_rCenter,
                                           const HGFBearing&    pi_rStartBearing,
                                           double               pi_Sweep,
                                           double               pi_Radius);
    void               SetByCenterAndBearings(const HGF2DLocation& pi_rCenter,
                                              const HGFBearing&    pi_rStartBearing,
                                              const HGFBearing&    pi_rEndBearing,
                                              double               pi_Radius);
    void               SetByStartAndSweep(const HGF2DLocation& pi_rCenter,
                                          const HGF2DLocation& pi_rStartPoint,
                                          double               pi_Sweep);
    void               SetByCircle(const HVE2DCircle&   pi_rCircle,
                                   const HGFBearing&    pi_rStartBearing,
                                   double               pi_Sweep);

    // Miscalenous
    HVE2DCircle        CalculateCircle() const;

    int32_t           IntersectArc(const HVE2DArc& pi_rArc,
                                    HGF2DLocation* po_pFirstPoint,
                                    HGF2DLocation* po_pSecondPoint) const;
    int32_t           IntersectLine(const HGF2DLine& pi_rLine,
                                     HGF2DLocation* po_pFirstPoint,
                                     HGF2DLocation* po_pSecondPoint) const;
    int32_t           IntersectSegment(const HVE2DSegment& pi_rSegment,
                                        HGF2DLocation* po_pFirstPoint,
                                        HGF2DLocation* po_pSecondPoint) const;

    void               Rotate(double               pi_Angle,
                              const HGF2DLocation& pi_rOrigin);


    // From HVE2DBasicLinear
    HVE2DBasicLinearTypeId    GetBasicLinearType() const;

    // From HVE2DLinear
    virtual double            CalculateLength() const;
    virtual HGF2DLocation     CalculateRelativePoint(double pi_RelativePos) const;
    virtual double            CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const;
    virtual double            CalculateRayArea(const HGF2DLocation& pi_rPoint) const;
    virtual void              Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
    virtual void              Shorten(const HGF2DLocation& pi_rNewStartPoint,
                                      const HGF2DLocation& pi_rNewEndPoint);
    virtual void              ShortenTo(const HGF2DLocation& pi_rNewEndPoint);
    virtual void              ShortenTo(double pi_EndRelativePosition);
    virtual void              ShortenFrom(const HGF2DLocation& pi_rNewStartPoint);
    virtual void              ShortenFrom(double pi_StartRelativePosition);
    virtual bool              AutoCrosses() const;
    virtual void              AdjustStartPointTo(const HGF2DLocation& pi_rPoint);
    virtual void              AdjustEndPointTo(const HGF2DLocation& pi_rPoint);

    virtual void              Drop(HGF2DLocationCollection* po_pPoint,
                                   double                   pi_rTolerance,
                                   EndPointProcessing       pi_EndPointProcessing = INCLUDE_END_POINT) const;



    // From HVE2DVector
    virtual HGF2DLocation     CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    virtual size_t            Intersect(const HVE2DVector& pi_rVector,
                                        HGF2DLocationCollection* po_pCrossPoints) const;
    virtual size_t            ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                         HGF2DLocationCollection* po_pContiguousnessPoints) const;
    virtual void              ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                           const HGF2DLocation& pi_rPoint,
                                                           HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                           HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    virtual HPMPersistentObject*
                              Clone() const;
    virtual HVE2DVector*      AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    virtual bool              Crosses(const HVE2DVector& pi_rVector) const;
    virtual bool              AreContiguous(const HVE2DVector& pi_rVector) const;
    virtual bool              AreAdjacent(const HVE2DVector& pi_rVector) const;
    virtual bool              IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                        HVE2DVector::ExtremityProcessing
                                        pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                        double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool              IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                           HVE2DVector::ExtremityProcessing
                                           pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                           double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool              AreContiguousAt(const HVE2DVector& pi_rVector,
                                              const HGF2DLocation& pi_rPoint) const;
    virtual HGFBearing        CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                               HVE2DVector::ArbitraryDirection
                                               pi_Direction = HVE2DVector::BETA) const;
    virtual double            CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                           HVE2DVector::ArbitraryDirection
                                                           pi_Direction = HVE2DVector::BETA) const;
    virtual bool              IsNull() const;

    // From HGFGraphicObject
    virtual HGF2DExtent       GetExtent() const;

    virtual void              Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void              Scale(double pi_ScaleFactor,
                                    const HGF2DLocation& pi_rScaleOrigin);

    // Debugging
    virtual void              PrintState(ostream& po_rOutput) const;


protected:

    virtual void              SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

private:

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        // The center must always be in same coordinate system as arc
        HASSERT(m_Center.GetCoordSys() == GetCoordSys());

        // The start and end points must always be in same coordinate system as arc
        HASSERT(m_StartPoint.GetCoordSys() == GetCoordSys());
        HASSERT(m_EndPoint.GetCoordSys() == GetCoordSys());
        }
#endif

    HVE2DVector*       AllocateCopyInComplexCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    void               AppendItselfInCoordSys(HVE2DComplexLinear& pio_rResultComplex,
                                              const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;

    bool               IsPointOnCircleOnArc(const HGF2DLocation& pi_rTestPoint) const;
    bool               AreArcsContiguous(const HVE2DArc& pi_rArc) const;
    bool               AreArcsContiguousAt(const HVE2DArc& pi_rArc,
                                            const HGF2DLocation& pi_rPoint) const;
    bool               AreArcsCrossing(const HVE2DArc& pi_rArc) const;
    bool               AreArcsAdjacent(const HVE2DArc& pi_rArc) const;
    bool               AreArcAndSegmentCrossing(const HVE2DSegment& pi_rSegment) const;
    bool               AreArcAndSegmentAdjacent(const HVE2DSegment& pi_rArc) const;

    size_t             ObtainContiguousnessPointsWithArc(const HVE2DArc& pi_rArc,
                                                         HGF2DLocationCollection* po_pContiguousnessPoints) const;

    void               ObtainContiguousnessPointsAtWithArc(const HVE2DArc& pi_rArc,
                                                           const HGF2DLocation& pi_rPoint,
                                                           HGF2DLocation* po_pFirstContiguousnessPoint,
                                                           HGF2DLocation* po_pSecondContiguousnessPoint) const;



    HGF2DLocation   m_Center;
    HGFAngle::AngleDirection m_RotationDirection;
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DArc.hpp"
