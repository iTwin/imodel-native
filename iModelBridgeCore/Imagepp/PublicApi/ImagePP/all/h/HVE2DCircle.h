//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DCircle.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DCircle
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DSimpleShape.h"

BEGIN_IMAGEPP_NAMESPACE
class HVE2DSegment;
class HGF2DLine;

class HVE2DCircle : public HVE2DSimpleShape
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DCircleId)


public:

    // Primary methods
    HVE2DCircle();
    HVE2DCircle(const HGF2DLocation& pi_rCenter,
                double               pi_rRadius);
    HVE2DCircle(const HGF2DLocation& pi_rFirstPoint,
                const HGF2DLocation& pi_rSecondPoint,
                const HGF2DLocation& pi_rThirdPoint);
    HVE2DCircle(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DCircle(const HVE2DCircle&    pi_rObject);
    virtual            ~HVE2DCircle();

    HVE2DCircle&      operator=(const HVE2DCircle& pi_rObj);

    // Compare operators
    bool              operator== (const HVE2DCircle& pi_rObject) const;
    bool              operator!= (const HVE2DCircle& pi_rObject) const;

    // Compare operators
    bool              IsEqualTo(const HVE2DCircle& pi_rObject) const;
    bool              IsEqualTo(const HVE2DCircle& pi_rObject, double pi_Epsilon) const;
    bool              IsEqualToAutoEpsilon(const HVE2DCircle& pi_rObject) const;

    // Miscalenious
    double            GetRadius() const;
    void              SetRadius(double pi_rNewRadius);
    const HGF2DLocation&
    GetCenter() const;
    void               SetCenter(const HGF2DLocation& pi_rNewCenter);
    void               SetByPerimeterPoints(const HGF2DLocation& pi_rFirstPoint,
                                            const HGF2DLocation& pi_rSecondPoint,
                                            const HGF2DLocation& pi_rThirdPoint);
    void               Set(const HGF2DLocation& pi_rCenter,
                           double               pi_rRadius);

    // Mathematical operations
    uint32_t           IntersectLine(const HGF2DLine& pi_rLine,
                                     HGF2DLocation* pi_pFirstCrossPoint,
                                     HGF2DLocation* pi_pSecondCrossPoint) const;

    uint32_t           IntersectSegment(const HVE2DSegment& pi_rSegment,
                                        HGF2DLocation* pi_pFirstCrossPoint,
                                        HGF2DLocation* pi_pSecondCrossPoint) const;

    uint32_t           IntersectCircle(const HVE2DCircle& pi_rCircle,
                                       HGF2DLocation* pi_pFirstCrossPoint,
                                       HGF2DLocation* pi_pSecondCrossPoint) const;
    double              CalculateShortestDistance(const HGF2DLocation& pi_rPoint) const;

    void                ChangeCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

    bool                IsAdjacentToCircle(const HVE2DCircle& pi_rCircle) const;
    bool                IsAdjacentToLine(const HGF2DLine& pi_rLine) const;
    void                ObtainCircleAdjacencyPoint(const HVE2DCircle& pi_rCircle,
                                                   HGF2DLocation* po_pAdjacencePoint) const;
    void                ObtainLineAdjacencyPoint(const HGF2DLine& pi_rLine,
                                                 HGF2DLocation* po_pAdjacencePoint) const;

    // From HVE2DSimpleShape
    virtual HVE2DComplexLinear    GetLinear() const;
    virtual HVE2DComplexLinear    GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
    virtual HVE2DComplexLinear*   AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;

    // From HVE2DShape
    virtual bool                  IsEmpty     () const;
    virtual HVE2DShapeTypeId      GetShapeType() const;
    virtual double                CalculateArea() const;
    virtual double                CalculatePerimeter() const;
    virtual bool                  IsPointIn(const HGF2DLocation& pi_rPoint, double Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual void                  MakeEmpty();

    // From HVE2DVector
    virtual HGF2DLocation         CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    virtual size_t                Intersect(const HVE2DVector& pi_rVector,
                                            HGF2DLocationCollection* po_pCrossPoints) const;
    virtual size_t                ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                             HGF2DLocationCollection* po_pContiguousnessPoints) const;
    virtual void                  ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                               const HGF2DLocation& pi_rPoint,
                                                               HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                               HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    virtual HVE2DVector*          AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    virtual bool                  Crosses(const HVE2DVector& pi_rVector) const;
    virtual bool                  AreContiguous(const HVE2DVector& pi_rVector) const;
    virtual bool                  AreAdjacent(const HVE2DVector& pi_rVector) const;
    virtual bool                  IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                            HVE2DVector::ExtremityProcessing
                                            pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                            double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool                  IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                               HVE2DVector::ExtremityProcessing
                                               pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                               double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool                  AreContiguousAt(const HVE2DVector& pi_rVector,
                                                  const HGF2DLocation& pi_rPoint) const;
    virtual HGFBearing            CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                                   HVE2DVector::ArbitraryDirection
                                                   pi_Direction = HVE2DVector::BETA) const;
    virtual double                CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                               HVE2DVector::ArbitraryDirection
                                                               pi_Direction = HVE2DVector::BETA) const;

    // From HGFGraphicObject
    virtual HGF2DExtent           GetExtent() const;

    virtual void                  Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void                  Scale(double pi_ScaleFactor,
                                        const HGF2DLocation& pi_rScaleOrigin);

    // From HPMPersistentObject
    virtual HPMPersistentObject*  Clone() const;

    // Debugging
    virtual void                  PrintState(ostream& po_rOutput) const;


protected:

    virtual void                  SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

private:


#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        // The center must always be in same coordinate system as circle
        HASSERT(m_Center.GetCoordSys() == GetCoordSys());

        // The radius may not be negative
        HASSERT(m_Radius >= 0.0);
        }
#endif

    void               ResetTolerance();

    HGF2DLocation   m_Center;
    double          m_Radius;
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DCircle.hpp"
