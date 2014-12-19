//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DComplexLinear.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
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
class HVE2DComplexLinear : public HVE2DLinear
    {

    HPM_DECLARE_CLASS_DLL(_HDLLg,  1102)

public:

    typedef list<HVE2DLinear*>   LinearList;
    typedef LinearList::iterator* IteratorHandle;


    // Primary methods
    _HDLLg                    HVE2DComplexLinear ();
    _HDLLg                    HVE2DComplexLinear (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    _HDLLg                    HVE2DComplexLinear (const HVE2DComplexLinear&    pi_rObject);
    _HDLLg virtual            ~HVE2DComplexLinear();

    HVE2DComplexLinear&
    operator=(const HVE2DComplexLinear& pi_rObj);

    // Information extraction
    bool              IsEmpty() const;
    const HVE2DComplexLinear::LinearList&    
                       GetLinearList() const;

    // Complex linear building
    virtual void       InsertLinear(const HVE2DLinear& pi_rLinear);
    virtual void       AppendLinear(const HVE2DLinear& pi_rLinear);
    virtual void       InsertComplexLinear(const HVE2DComplexLinear& pi_rComplexLinear);
    virtual void       AppendComplexLinear(const HVE2DComplexLinear& pi_rComplexLinear);
    virtual void       InsertLinearPtrSCS(HVE2DLinear* pi_pLinear);
    virtual void       AppendLinearPtrSCS(HVE2DLinear* pi_pLinear);


    // Component extraction
    size_t             GetNumberOfLinears() const;
    const HVE2DLinear& GetLinear(size_t pi_Address) const;

    // Miscalenious
    _HDLLg void        MakeEmpty();
    void               SplitAtAllIntersectionPoints(const HVE2DVector& pi_rVector);
    void               SplitAtAllOnPoints(const HGF2DLocationCollection& pi_rPoints);
    void               SplitAtAllOnPointsSCS(const HGF2DLocationCollection& pi_rPoints);

    // From HVE2DLinear
    virtual bool       IsABasicLinear() const;
    virtual bool       IsComplex() const;

    _HDLLg virtual double    CalculateLength() const;
    _HDLLg virtual HGF2DLocation    
                             CalculateRelativePoint(double pi_RelativePos) const;
    _HDLLg virtual double    CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const;
    _HDLLg virtual double    CalculateRayArea(const HGF2DLocation& pi_rPoint) const;

    _HDLLg virtual void      Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
    _HDLLg virtual void      Shorten(const HGF2DLocation& pi_rNewStartPoint,
                                     const HGF2DLocation& pi_rNewEndPoint);

    _HDLLg virtual void      ShortenTo(const HGF2DLocation& pi_rNewEndPoint);
    _HDLLg virtual void      ShortenTo(double pi_EndRelativePosition);
    _HDLLg virtual void      ShortenFrom(const HGF2DLocation& pi_rNewStartPoint);
    _HDLLg virtual void      ShortenFrom(double pi_StartRelativePosition);
    _HDLLg virtual void      Reverse();
    _HDLLg virtual void      AdjustStartPointTo(const HGF2DLocation& pi_rPoint);
    _HDLLg virtual void      AdjustEndPointTo(const HGF2DLocation& pi_rPoint);
    _HDLLg virtual void      Drop(HGF2DLocationCollection* po_pPoint,
                                  double  pi_Tolerance,
                                  EndPointProcessing  pi_EndPointProcessing = INCLUDE_END_POINT) const;

    _HDLLg virtual bool      AutoCrosses() const;

    // From HVE2DVector
    _HDLLg virtual HGF2DLocation
                             CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    _HDLLg virtual size_t    Intersect(const HVE2DVector& pi_rVector,
                                       HGF2DLocationCollection* po_pCrossPoints) const;
    _HDLLg virtual size_t    ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                        HGF2DLocationCollection* po_pContiguousnessPoints) const;
    _HDLLg virtual void      ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                          const HGF2DLocation& pi_rPoint,
                                                          HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                          HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    _HDLLg virtual HVE2DVector*
                             AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    _HDLLg virtual bool      Crosses(const HVE2DVector& pi_rVector) const;
    _HDLLg virtual bool      AreContiguous(const HVE2DVector& pi_rVector) const;
    _HDLLg virtual bool      AreAdjacent(const HVE2DVector& pi_rVector) const;
    _HDLLg virtual bool      IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                       HVE2DVector::ExtremityProcessing
                                       pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                       double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    _HDLLg virtual bool      IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                          HVE2DVector::ExtremityProcessing
                                          pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                          double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    _HDLLg virtual bool      AreContiguousAt(const HVE2DVector& pi_rVector,
                                             const HGF2DLocation& pi_rPoint) const;
    _HDLLg virtual HGFBearing    
                             CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                              HVE2DVector::ArbitraryDirection
                                              pi_Direction = HVE2DVector::BETA) const;
    _HDLLg virtual double    CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                          HVE2DVector::ArbitraryDirection
                                                          pi_Direction = HVE2DVector::BETA) const;
    virtual bool             IsNull() const;
    _HDLLg virtual void      SetTolerance(double pi_Tolerance);
    _HDLLg virtual void      SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance);

    // From HGFGraphicObject
    _HDLLg virtual HGF2DExtent
                             GetExtent() const;

    _HDLLg virtual void      Move(const HGF2DDisplacement& pi_rDisplacement);
    _HDLLg virtual void      Scale(double pi_ScaleFactor,
                                   const HGF2DLocation& pi_rScaleOrigin);

    _HDLLg virtual bool      AreContiguousAtAndGet(const HVE2DVector& pi_rVector,
                                                   const HGF2DLocation& pi_rPoint,
                                                   HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                   HGF2DLocation* pi_pSecondContiguousnessPoint) const;



    // From HPMPersistentObject
    virtual HPMPersistentObject*
                             Clone() const;


    // Debugging
    _HDLLg virtual void      PrintState(ostream& po_rOutput) const;

protected:

    _HDLLg virtual void      SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

    // Member attribute ... list of linears
    LinearList      m_LinearList;

    // Acceleration attributes
    mutable HGF2DExtent     m_Extent;
    mutable bool            m_ExtentUpToDate;
    mutable double          m_Length;
    mutable bool            m_LengthUpToDate;

private:
    };


#include "HVE2DComplexLinear.hpp"
