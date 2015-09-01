//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DVectorGroup.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    virtual HGF2DLocation    CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    virtual HGFBearing  CalculateBearing(const HGF2DLocation& pi_rPoint,
                                         HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
    virtual double    CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                 HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
    virtual size_t      Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
    virtual size_t      ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                   HGF2DLocationCollection* po_pContiguousnessPoints) const;
    virtual void        ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                     const HGF2DLocation& pi_rPoint,
                                                     HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                     HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    virtual HPMPersistentObject*
    Clone() const;
    virtual HVE2DVector*
    AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;

    // Vector property determination
    virtual bool       Crosses(const HVE2DVector& pi_rVector) const;
    virtual bool       AreContiguous(const HVE2DVector& pi_rVector) const;
    virtual bool       AreAdjacent(const HVE2DVector& pi_rVector) const;
    virtual bool       IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                  HVE2DVector::ExtremityProcessing pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                  double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool       IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                     HVE2DVector::ExtremityProcessing pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                     double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
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
    virtual void        SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance);

    // Temporary
    virtual void        Rotate(double               pi_Angle,
                               const HGF2DLocation& pi_rOrigin)
        {};

    virtual void        PrintState(ostream& po_rOutput) const;

    // Inherited from graphic object
    virtual HGF2DExtent
    GetExtent() const;

    virtual void       Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void       Scale(double pi_ScaleFactor,
                             const HGF2DLocation& pi_rScaleOrigin);

protected:

    virtual void       SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

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
