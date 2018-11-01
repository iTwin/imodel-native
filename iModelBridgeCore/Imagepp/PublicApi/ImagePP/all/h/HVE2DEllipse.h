//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DEllipse.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DEllipse
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DPolygon.h"
#include "HVE2DRectangle.h"
#include "HVE2DSimpleShape.h"

BEGIN_IMAGEPP_NAMESPACE
class HGF2DLine;
class HVE2DSegment;

class HVE2DEllipse : public HVE2DSimpleShape
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT, HVE2DEllipseId)

public:

    // Primary methods
                                 HVE2DEllipse();
                                 HVE2DEllipse(const HVE2DEllipse& pi_rObj);

    IMAGEPP_EXPORT                       HVE2DEllipse(const HVE2DRectangle& pi_rRect);

    virtual                      ~HVE2DEllipse();

    HVE2DEllipse&                operator=(const HVE2DEllipse& pi_rObj);

    // From HVE2DSimpleShape
    IMAGEPP_EXPORT HVE2DComplexLinear   GetLinear() const override;
    IMAGEPP_EXPORT HVE2DComplexLinear   GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const override;
    IMAGEPP_EXPORT HVE2DComplexLinear*  AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const override;

    // From HVE2DShape
    bool                 IsEmpty     () const override;
    HVE2DShapeTypeId     GetShapeType() const override;
    double               CalculateArea() const override;
    double               CalculatePerimeter() const override;
    bool                 IsPointIn(const HGF2DLocation& pi_rPoint, double Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    void                 MakeEmpty() override;

    IMAGEPP_EXPORT HGF2DShape*   GetLightShape() const override;

    // From HVE2DVector
    HGF2DLocation        CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const override;
    size_t               Intersect(const HVE2DVector& pi_rVector,
                                           HGF2DLocationCollection* po_pCrossPoints) const override;
    size_t               ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                            HGF2DLocationCollection* po_pContiguousnessPoints) const override;
    void                 ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                              const HGF2DLocation& pi_rPoint,
                                                              HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                              HGF2DLocation* pi_pSecondContiguousnessPoint) const override;
    IMAGEPP_EXPORT HVE2DVector*  AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const override;
    bool                 Crosses(const HVE2DVector& pi_rVector) const override;
    bool                 AreContiguous(const HVE2DVector& pi_rVector) const override;
    bool                 AreAdjacent(const HVE2DVector& pi_rVector) const override;
    bool                 IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                           HVE2DVector::ExtremityProcessing
                                           pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                           double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    bool                 IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                              HVE2DVector::ExtremityProcessing
                                              pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                              double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    bool                 AreContiguousAt(const HVE2DVector& pi_rVector,
                                                 const HGF2DLocation& pi_rPoint) const override;
    HGFBearing           CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                                  HVE2DVector::ArbitraryDirection
                                                  pi_Direction = HVE2DVector::BETA) const override;
    double               CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                              HVE2DVector::ArbitraryDirection
                                                              pi_Direction = HVE2DVector::BETA) const override;

    // From HGFGraphicObject
    HGF2DExtent          GetExtent() const override;


    void                 Move(const HGF2DDisplacement& pi_rDisplacement) override;
    void                 Scale(double pi_ScaleFactor,
                                       const HGF2DLocation& pi_rScaleOrigin) override;

    // From HPMPersistentObject
    HPMPersistentObject* Clone() const override;

private:

    double       ComputeX(double pi_X,
                           int16_t pi_SignFactor = 1) const;

    double       ComputeY(double pi_X,
                           int16_t pi_SignFactor = 1) const;
    IMAGEPP_EXPORT void  ResetTolerance();

    bool          m_IsVertical;
    double        m_SemiMajorAxis;
    double        m_SemiMinorAxis;
    HGF2DLocation m_Center;
    HGF2DLocation m_F1;
    HGF2DLocation m_F2;
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DEllipse.hpp"
