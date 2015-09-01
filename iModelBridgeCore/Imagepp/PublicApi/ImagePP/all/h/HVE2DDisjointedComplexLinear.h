//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DDisjointedComplexLinear.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DDisjointedComplexLinear
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DComplexLinear.h"

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
class HVE2DDisjointedComplexLinear : public HVE2DComplexLinear
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT, HVE2DLinearId_DisjointedComplex)

public:
    DEFINE_T_SUPER(HVE2DComplexLinear)

    // Primary methods
    IMAGEPP_EXPORT                    HVE2DDisjointedComplexLinear ();
    IMAGEPP_EXPORT                    HVE2DDisjointedComplexLinear (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT                    HVE2DDisjointedComplexLinear (const HVE2DDisjointedComplexLinear&    pi_rObject);
    IMAGEPP_EXPORT virtual            ~HVE2DDisjointedComplexLinear();

    HVE2DDisjointedComplexLinear&
                              operator=(const HVE2DDisjointedComplexLinear& pi_rObj);

    // Complex linear building
    virtual void              InsertLinear(const HVE2DLinear& pi_rLinear);
    virtual void              AppendLinear(const HVE2DLinear& pi_rLinear);
    virtual void              InsertComplexLinear(const HVE2DDisjointedComplexLinear& pi_rDisComplexLinear);
    virtual void              InsertComplexLinear(const HVE2DComplexLinear& pi_rComplexLinear) {
        T_Super::InsertComplexLinear(pi_rComplexLinear);
        }

    virtual void              AppendComplexLinear(const HVE2DComplexLinear& pi_rComplexLinear) {
        T_Super::AppendComplexLinear(pi_rComplexLinear);
        }
    virtual void              AppendComplexLinear(const HVE2DDisjointedComplexLinear& pi_rDisComplexLinear);
    virtual void              InsertLinearPtrSCS(HVE2DLinear* pi_pLinear);
    virtual void              AppendLinearPtrSCS(HVE2DLinear* pi_pLinear);

    virtual void              Drop(HGF2DLocationCollection* po_pPoint,
                                   double                   pi_rTolerance,
                                   EndPointProcessing       pi_EndPointProcessing = INCLUDE_END_POINT) const;

    // From HPMPersistentObject
    virtual HPMPersistentObject* Clone() const;
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DDisjointedComplexLinear.hpp"
