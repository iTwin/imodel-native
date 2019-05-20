//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DComplexLinear
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DLinear.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a complex linear. A complex linear is simply a
    collection of linears which connect by their extremity points. The sum of
    all those linears produces a linear expressed by a complex linear.
    A complex linear contains component linears which can be any type of
    linear including complex linears themselves.

    A complex linear possesses the same behavior as any other type of linears.

    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
class HVE2DComplexLinear : public HVE2DLinear
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DLinearId_Complex)

public:

    typedef list<HVE2DLinear*>   LinearList;
    typedef LinearList::iterator* IteratorHandle;


    // Primary methods
    IMAGEPP_EXPORT                    HVE2DComplexLinear ();
    IMAGEPP_EXPORT                    HVE2DComplexLinear (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT                    HVE2DComplexLinear (const HVE2DComplexLinear&    pi_rObject);
    IMAGEPP_EXPORT virtual            ~HVE2DComplexLinear();

    IMAGEPP_EXPORT HVE2DComplexLinear& operator=(const HVE2DComplexLinear& pi_rObj);

    // Information extraction
    bool              IsEmpty() const;
    const HVE2DComplexLinear::LinearList&    
                       GetLinearList() const;

    // Complex linear building
    IMAGEPPTEST_EXPORT virtual void       InsertLinear(const HVE2DLinear& pi_rLinear);
    IMAGEPP_EXPORT     virtual void       AppendLinear(const HVE2DLinear& pi_rLinear);
    IMAGEPPTEST_EXPORT virtual void       InsertComplexLinear(const HVE2DComplexLinear& pi_rComplexLinear);
    IMAGEPPTEST_EXPORT virtual void       AppendComplexLinear(const HVE2DComplexLinear& pi_rComplexLinear);
    IMAGEPPTEST_EXPORT virtual void       InsertLinearPtrSCS(HVE2DLinear* pi_pLinear);
    IMAGEPPTEST_EXPORT virtual void       AppendLinearPtrSCS(HVE2DLinear* pi_pLinear);


    // Component extraction
    size_t             GetNumberOfLinears() const;
    const HVE2DLinear& GetLinear(size_t pi_Address) const;

    // Miscalenious
    IMAGEPP_EXPORT void        MakeEmpty();
    IMAGEPPTEST_EXPORT void               SplitAtAllIntersectionPoints(const HVE2DVector& pi_rVector);
    IMAGEPPTEST_EXPORT void               SplitAtAllOnPoints(const HGF2DLocationCollection& pi_rPoints);
    IMAGEPPTEST_EXPORT void               SplitAtAllOnPointsSCS(const HGF2DLocationCollection& pi_rPoints);

    // From HVE2DLinear
    bool       IsABasicLinear() const override;
    bool       IsComplex() const override;

    IMAGEPP_EXPORT double    CalculateLength() const override;
    IMAGEPP_EXPORT HGF2DLocation    
                             CalculateRelativePoint(double pi_RelativePos) const override;
    IMAGEPP_EXPORT double    CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const override;
    IMAGEPP_EXPORT double    CalculateRayArea(const HGF2DLocation& pi_rPoint) const override;

    IMAGEPP_EXPORT void      Shorten(double pi_StartRelativePos, double pi_EndRelativePos) override;
    IMAGEPP_EXPORT void      Shorten(const HGF2DLocation& pi_rNewStartPoint,
                                     const HGF2DLocation& pi_rNewEndPoint) override;

    IMAGEPP_EXPORT void      ShortenTo(const HGF2DLocation& pi_rNewEndPoint) override;
    IMAGEPP_EXPORT void      ShortenTo(double pi_EndRelativePosition) override;
    IMAGEPP_EXPORT void      ShortenFrom(const HGF2DLocation& pi_rNewStartPoint) override;
    IMAGEPP_EXPORT void      ShortenFrom(double pi_StartRelativePosition) override;
    IMAGEPP_EXPORT void      Reverse() override;
    IMAGEPP_EXPORT void      AdjustStartPointTo(const HGF2DLocation& pi_rPoint) override;
    IMAGEPP_EXPORT void      AdjustEndPointTo(const HGF2DLocation& pi_rPoint) override;
    IMAGEPP_EXPORT void      Drop(HGF2DLocationCollection* po_pPoint,
                                  double  pi_Tolerance,
                                  EndPointProcessing  pi_EndPointProcessing = INCLUDE_END_POINT) const override;

    IMAGEPP_EXPORT bool      AutoCrosses() const override;

    // From HVE2DVector
    IMAGEPP_EXPORT HGF2DLocation
                             CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const override;
    IMAGEPP_EXPORT size_t    Intersect(const HVE2DVector& pi_rVector,
                                       HGF2DLocationCollection* po_pCrossPoints) const override;
    IMAGEPP_EXPORT size_t    ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                        HGF2DLocationCollection* po_pContiguousnessPoints) const override;
    IMAGEPP_EXPORT void      ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                          const HGF2DLocation& pi_rPoint,
                                                          HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                          HGF2DLocation* pi_pSecondContiguousnessPoint) const override;
    IMAGEPP_EXPORT HVE2DVector*
                             AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const override;
    IMAGEPP_EXPORT bool      Crosses(const HVE2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool      AreContiguous(const HVE2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool      AreAdjacent(const HVE2DVector& pi_rVector) const override;
    IMAGEPP_EXPORT bool      IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                       HVE2DVector::ExtremityProcessing
                                       pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                       double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPP_EXPORT bool      IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                          HVE2DVector::ExtremityProcessing
                                          pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                          double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const override;
    IMAGEPP_EXPORT bool      AreContiguousAt(const HVE2DVector& pi_rVector,
                                             const HGF2DLocation& pi_rPoint) const override;
    IMAGEPP_EXPORT HGFBearing    
                             CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                              HVE2DVector::ArbitraryDirection
                                              pi_Direction = HVE2DVector::BETA) const override;
    IMAGEPP_EXPORT double    CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                          HVE2DVector::ArbitraryDirection
                                                          pi_Direction = HVE2DVector::BETA) const override;
    bool             IsNull() const override;
    IMAGEPP_EXPORT void      SetTolerance(double pi_Tolerance) override;
    IMAGEPP_EXPORT void      SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance) override;

    // From HGFGraphicObject
    IMAGEPP_EXPORT HGF2DExtent
                             GetExtent() const override;

    IMAGEPP_EXPORT void      Move(const HGF2DDisplacement& pi_rDisplacement) override;
    IMAGEPP_EXPORT void      Scale(double pi_ScaleFactor,
                                   const HGF2DLocation& pi_rScaleOrigin) override;

    IMAGEPP_EXPORT bool      AreContiguousAtAndGet(const HVE2DVector& pi_rVector,
                                                   const HGF2DLocation& pi_rPoint,
                                                   HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                   HGF2DLocation* pi_pSecondContiguousnessPoint) const override;



    // From HPMPersistentObject
    HPMPersistentObject*
                             Clone() const override;


    // Debugging
    IMAGEPP_EXPORT void      PrintState(ostream& po_rOutput) const override;

protected:

    IMAGEPP_EXPORT void      SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) override;

    // Member attribute ... list of linears
    LinearList      m_LinearList;

    // Acceleration attributes
    mutable HGF2DExtent     m_Extent;
    mutable bool            m_ExtentUpToDate;
    mutable double          m_Length;
    mutable bool            m_LengthUpToDate;

private:
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DComplexLinear.hpp"
