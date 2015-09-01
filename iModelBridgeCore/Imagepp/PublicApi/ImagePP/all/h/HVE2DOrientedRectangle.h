//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DOrientedRectangle.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DOrientedRectangle
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DSimpleShape.h"

BEGIN_IMAGEPP_NAMESPACE
class HVEShapeRasterLine;

class HVE2DOrientedRectangle : public HVE2DSimpleShape
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DOrientedRectangleId)

public:




    // Primary methods
    HVE2DOrientedRectangle();
    HVE2DOrientedRectangle(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DOrientedRectangle(const HGF2DLocation& pi_rFirstPoint,
                           const HGF2DLocation& pi_rSecondPoint,
                           const HGF2DLocation& pi_rThirdPoint,
                           const HGF2DLocation& pi_rFourthPoint);
    HVE2DOrientedRectangle (const HVE2DRectangle& pi_rRectangle);
    HVE2DOrientedRectangle (const HVE2DOrientedRectangle&    pi_rObject);
    virtual            ~HVE2DOrientedRectangle();

    HVE2DOrientedRectangle&    operator=(const HVE2DOrientedRectangle& pi_rObj);


    // From HVE2DSimpleShape
    virtual HVE2DComplexLinear    GetLinear() const;
    virtual HVE2DComplexLinear    GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
    virtual HVE2DComplexLinear*   AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;

    // From HVE2DShape
    virtual void                  GenerateScanLines(HVEShapeRasterLine& pio_rScanlines,
                                                    double              pi_rYRef,
                                                    double              pi_rYStep,
                                                    bool                pi_rAdd = true) const;

    virtual bool                  IsEmpty     () const;
    virtual HVE2DShapeTypeId      GetShapeType() const;
    virtual HVE2DShape*           DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*           DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*           IntersectShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual HVE2DShape*           UnifyShapeSCS(const HVE2DShape& pi_rShape) const;
    virtual double                CalculateArea() const;
    virtual double                CalculatePerimeter() const;
    virtual bool                  IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual void                  MakeEmpty();
    virtual HVE2DShape::SpatialPosition
                                  CalculateSpatialPositionOfSingleComponentVector(const HVE2DVector& pi_rVector) const;
    virtual HVE2DShape::SpatialPosition
                                  CalculateSpatialPositionOfNonCrossingLinear(const HVE2DLinear& pi_rLinear) const;

    // From HVE2DVector
    virtual HGF2DLocation         CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    virtual size_t                Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
    virtual size_t                ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                             HGF2DLocationCollection* po_pContiguousnessPoints) const;
    virtual void                  ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                               const HGF2DLocation& pi_rPoint,
                                                               HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                               HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    virtual HPMPersistentObject*  Clone() const;
    virtual HVE2DVector*          AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    virtual bool                  Crosses(const HVE2DVector& pi_rVector) const;
    virtual bool                  AreContiguous(const HVE2DVector& pi_rVector) const;
    virtual bool                  AreAdjacent(const HVE2DVector& pi_rVector) const;
    virtual bool                  IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                            HVE2DVector::ExtremityProcessing pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES) const;
    virtual bool                  IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                               HVE2DVector::ExtremityProcessing pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES) const;
    virtual bool                  AreContiguousAt(const HVE2DVector& pi_rVector,
                                                  const HGF2DLocation& pi_rPoint) const;
    virtual HGFBearing            CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                                   HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
    virtual double                CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                               HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;

    // From HGFGraphicObject
    virtual HGF2DExtent           GetExtent() const;
    virtual void                  Move(const HGF2DDisplacement& pi_rDisplacement);

protected:

private:

    // Area oriented operations on rectangle
    virtual HVE2DShape*     DifferentiateRectangleSCS(const HVE2DRectangle& pi_rRectangle) const;
    virtual HVE2DShape*     DifferentiateFromRectangleSCS(const HVE2DRectangle& pi_rRectangle) const;
    virtual HVE2DShape*     IntersectRectangleSCS(const HVE2DRectangle& pi_rRectangle) const;
    virtual HVE2DShape*     UnifyRectangleSCS(const HVE2DRectangle& pi_rRectangle) const;

    virtual HVE2DShape*     DifferentiateOrientedRectangleSCS(const HVE2DOrientedRectangle& pi_rRectangle) const;
    virtual HVE2DShape*     IntersectOrientedRectangleSCS(const HVE2DOrientedRectangle& pi_rRectangle) const;
    virtual HVE2DShape*     UnifyOrientedRectangleSCS(const HVE2DOrientedRectangle& pi_rRectangle) const;


    // Member attribute ... the extremes of the rectangle
    HGF2DLocation   m_FirstCorner;
    HGF2DLocation   m_SecondCorner;
    HGF2DLocation   m_ThirdCorner;
    HGF2DLocation   m_FourthCorner;
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DOrientedRectangle.hpp"
