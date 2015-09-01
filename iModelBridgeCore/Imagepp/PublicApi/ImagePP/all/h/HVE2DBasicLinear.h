//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DBasicLinear.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DBasicLinear
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DLinear.h"


BEGIN_IMAGEPP_NAMESPACE
typedef uint32_t HVE2DBasicLinearTypeId;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This abstract class encapsulates all attributes common to basic linear
    elements. Basic linear elements are linear elements which are composed of
    a single component. This basic linear is therefore based on a unique
    mathematical definition valid over all the length of the basic linear.
    Basic linear are also named "vertices".
    -----------------------------------------------------------------------------
*/
class HNOVTABLEINIT HVE2DBasicLinear : public HVE2DLinear
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DLinearId_Basic)

public:

    // Primary methods
    HVE2DBasicLinear();
    HVE2DBasicLinear(const HGF2DLocation& pi_rStartPoint,
                     const HGF2DLocation& pi_rEndPoint);
    HVE2DBasicLinear(const HGF2DPosition& pi_rStartPoint,
                     const HGF2DPosition& pi_rEndPoint,
                     const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DBasicLinear(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DBasicLinear(const HVE2DBasicLinear&    pi_rObject);
    virtual            ~HVE2DBasicLinear();

    HVE2DBasicLinear&  operator=(const HVE2DBasicLinear& pi_rObj);

    /** -----------------------------------------------------------------------------
        This pure virtual method returns an id uniquely identifying the basic linear
        type.
        -----------------------------------------------------------------------------
    */
    virtual HVE2DBasicLinearTypeId
    GetBasicLinearType() const = 0;

    // From HVE2DLinear
    virtual bool      IsABasicLinear() const;
    virtual bool      IsComplex() const;

    // Debug method
    void              PrintState(ostream& po_rOutput) const;

protected:

private:

    };
END_IMAGEPP_NAMESPACE


#include "HVE2DBasicLinear.hpp"
