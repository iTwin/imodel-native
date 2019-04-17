//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DVectorGroup
//-----------------------------------------------------------------------------
// Description of a vector
//-----------------------------------------------------------------------------
#pragma once

#include "HVE2DVector.h"

BEGIN_IMAGEPP_NAMESPACE
// Type used for the main vector type
typedef uint32_t HVE2DVectorGroupTypeId;

class HVE2DVectorGroup : public HVE2DVector
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DVectorGroupId)

public:


    // Primary methods
    HVE2DVectorGroup();
    HVE2DVectorGroup(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DVectorGroup(const HVE2DVectorGroup&    pi_rObject);
    virtual             ~HVE2DVectorGroup();
    HVE2DVectorGroup&   operator=(const HVE2DVectorGroup& pi_rObj);

    // Added method
    void                AddVector(HVE2DVector* pi_pVector);

    // Operations
    HGF2DLocation    CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const override;
    HGFBearing  CalculateBearing(const HGF2DLocation& pi_rPoint,
                                         HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const override;
    double    CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                 HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const override;
    size_t      Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const override;
    size_t      ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                   HGF2DLocationCollection* po_pContiguousnessPoints) const override;
    void        ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                     const HGF2DLocation& pi_rPoint,
                                                     HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                     HGF2DLocation* pi_pSecondContiguousnessPoint) const override;
    HPMPersistentObject*
    Clone() const override;
    HVE2DVector*
    AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const override;

    // Vector property determination
    bool       Crosses(const HVE2DVector& pi_rVector) const override;
    bool       AreContiguous(const HVE2DVector& pi_rVector) const override;
    bool       AreAdjacent(const HVE2DVector& pi_rVector) const override;
    bool       IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                  HVE2DVector::ExtremityProcessing pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                  double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    bool       IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                     HVE2DVector::ExtremityProcessing pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                     double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    bool       AreContiguousAt(const HVE2DVector& pi_rVector,
                                        const HGF2DLocation& pi_rPoint) const override;

    bool       IsAtAnExtremity(const HGF2DLocation& pi_rLocation,
                                        double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    bool       IsNull() const override;

    // Classification
    HVE2DVectorTypeId
    GetMainVectorType() const override;

    // Tolerance application
    void        SetAutoToleranceActive(bool pi_ActiveAutoTolerance) override;
    void        SetTolerance(double pi_Tolerance) override;
    void        SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance) override;

    // Temporary
    virtual void        Rotate(double               pi_Angle,
                               const HGF2DLocation& pi_rOrigin)
        {};

    void        PrintState(ostream& po_rOutput) const override;

    // Inherited from graphic object
    HGF2DExtent
    GetExtent() const override;

    void       Move(const HGF2DDisplacement& pi_rDisplacement) override;
    void       Scale(double pi_ScaleFactor,
                             const HGF2DLocation& pi_rScaleOrigin) override;

protected:

    void       SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) override;

private:

    void MakeEmpty();

    // List of vectors in the group
    typedef list<HVE2DVector*, allocator<HVE2DVector*> > VectorList;

    VectorList     m_VectorList;

    // Acceleration attributes
    HGF2DExtent    m_Extent;
    bool          m_ExtentUpToDate;
    };
END_IMAGEPP_NAMESPACE

#include "HVE2DVectorGroup.hpp"
