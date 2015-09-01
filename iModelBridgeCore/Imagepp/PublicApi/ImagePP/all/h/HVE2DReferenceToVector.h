//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DReferenceToVector.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAReferenceToRaster
//-----------------------------------------------------------------------------
// This class describes a reference to another raster object.
//-----------------------------------------------------------------------------
#pragma once

#include "HVE2DVector.h"

BEGIN_IMAGEPP_NAMESPACE
class HVE2DVector;

class HVE2DReferenceToVector : public HVE2DVector
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DReferenceToVectorId)

public:

    // Primary methods
    HVE2DReferenceToVector();

    HVE2DReferenceToVector(const HVE2DVector*   pi_pSource);
    HVE2DReferenceToVector(const HVE2DVector*   pi_pSource,
                           const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);
    HVE2DReferenceToVector(const HVE2DReferenceToVector& pi_rObj);
    virtual ~HVE2DReferenceToVector();
    HVE2DReferenceToVector& operator=(const HVE2DReferenceToVector& pi_rObj);

    // Added methods
    const HVE2DVector*  GetSource() const;


    // Overiden from HGFGraphicObject
    virtual HGF2DExtent
    GetExtent() const;

    virtual void        Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void        Rotate(double               pi_Angle,
                               const HGF2DLocation& pi_rOrigin);
    virtual void        Scale(double pi_ScaleFactorX,
                              double pi_ScaleFactorY,
                              const HGF2DLocation& pi_rOrigin);

    // Overriden from HVE2DVector
    virtual HGF2DLocation    CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    virtual HGFBearing       CalculateBearing(const HGF2DLocation& pi_rPoint,
                                              HVE2DVector::ArbitraryDirection
                                              pi_Direction = HVE2DVector::BETA) const;
    virtual double           CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                                          HVE2DVector::ArbitraryDirection
                                                          pi_Direction = HVE2DVector::BETA) const;
    virtual size_t           Intersect(const HVE2DVector& pi_rVector,
                                       HGF2DLocationCollection* po_pCrossPoints) const;
    virtual size_t           ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                        HGF2DLocationCollection* po_pContiguousnessPoints) const;
    virtual void             ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                          const HGF2DLocation& pi_rPoint,
                                                          HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                          HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    virtual HVE2DVector*     AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;

    // Vector property determination
    virtual bool       Crosses(const HVE2DVector& pi_rVector) const;
    virtual bool       AreContiguous(const HVE2DVector& pi_rVector) const;
    virtual bool       AreAdjacent(const HVE2DVector& pi_rVector) const;
    virtual bool       IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                 HVE2DVector::ExtremityProcessing
                                 pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                 double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool       IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                    HVE2DVector::ExtremityProcessing
                                    pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                    double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool       IsConnectedBy(const HVE2DLinear& pi_rLinear) const;
    virtual bool       AreContiguousAt(const HVE2DVector& pi_rVector,
                                       const HGF2DLocation& pi_rPoint) const;

    virtual bool       IsAtAnExtremity(const HGF2DLocation& pi_rLocation,
                                        double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool       IsNull() const;

    // Classification
    virtual HVE2DVectorTypeId
                        GetMainVectorType() const;

    // Tolerance application
    virtual void        SetAutoToleranceActive(bool pi_ActiveAutoTolerance);
    virtual void        SetTolerance(double pi_Tolerance);

    virtual bool        AreContiguousAtAndGet(const HVE2DVector& pi_rVector,
                                              const HGF2DLocation& pi_rPoint,
                                              HGF2DLocation* pi_pFirstContiguousnessPoint,
                                              HGF2DLocation* pi_pSecondContiguousnessPoint) const;


    // Debug function
    virtual void        PrintState(ostream& po_rOutput) const;

    // Message handlers
    bool                NotifyGeometryChanged (HMGMessage& pi_rMessage);

    HPMPersistentObject*
                        Clone () const;



protected:

    virtual void        SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

private:

    // Source raster object
    const HVE2DVector* m_pSource;

    // Tell if the reference has a geometry different of the source.
    // i.e. different coordinate system and shape.
    bool               m_CoordSysChanged;

    // Acceleration attributes
    bool               m_ExtentUpToDate;
    HGF2DExtent        m_Extent;
    };
END_IMAGEPP_NAMESPACE

#include "HVE2DReferenceToVector.hpp"
