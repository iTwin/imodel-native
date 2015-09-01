//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DEllipse.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    IMAGEPP_EXPORT virtual HVE2DComplexLinear   GetLinear() const;
    IMAGEPP_EXPORT virtual HVE2DComplexLinear   GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
    IMAGEPP_EXPORT virtual HVE2DComplexLinear*  AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;

    // From HVE2DShape
    virtual bool                 IsEmpty     () const;
    virtual HVE2DShapeTypeId     GetShapeType() const;
    virtual double               CalculateArea() const;
    virtual double               CalculatePerimeter() const;
    virtual bool                 IsPointIn(const HGF2DLocation& pi_rPoint, double Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual void                 MakeEmpty();

    IMAGEPP_EXPORT virtual HGF2DShape*   GetLightShape() const;

    // From HVE2DVector
    virtual HGF2DLocation        CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    virtual size_t               Intersect(const HVE2DVector& pi_rVector,
                                           HGF2DLocationCollection* po_pCrossPoints) const;
    virtual size_t               ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                            HGF2DLocationCollection* po_pContiguousnessPoints) const;
    virtual void                 ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                              const HGF2DLocation& pi_rPoint,
                                                              HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                              HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    IMAGEPP_EXPORT virtual HVE2DVector*  AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    virtual bool                 Crosses(const HVE2DVector& pi_rVector) const;
    virtual bool                 AreContiguous(const HVE2DVector& pi_rVector) const;
    virtual bool                 AreAdjacent(const HVE2DVector& pi_rVector) const;
    virtual bool                 IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                           HVE2DVector::ExtremityProcessing
                                           pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                           double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool                 IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                              HVE2DVector::ExtremityProcessing
                                              pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                              double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool                 AreContiguousAt(const HVE2DVector& pi_rVector,
                                                 const HGF2DLocation& pi_rPoint) const;
    virtual HGFBearing           CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                                  HVE2DVector::ArbitraryDirection
                                                  pi_Direction = HVE2DVector::BETA) const;
    virtual double               CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                              HVE2DVector::ArbitraryDirection
                                                              pi_Direction = HVE2DVector::BETA) const;

    // From HGFGraphicObject
    virtual HGF2DExtent          GetExtent() const;


    virtual void                 Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void                 Scale(double pi_ScaleFactor,
                                       const HGF2DLocation& pi_rScaleOrigin);

    // From HPMPersistentObject
    virtual HPMPersistentObject* Clone() const;

private:

    double       ComputeX(double pi_X,
                           short pi_SignFactor = 1) const;

    double       ComputeY(double pi_X,
                           short pi_SignFactor = 1) const;
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
