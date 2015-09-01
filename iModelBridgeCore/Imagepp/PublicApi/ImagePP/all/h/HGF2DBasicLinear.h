//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DBasicLinear.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DBasicLinear
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DLinear.h"
BEGIN_IMAGEPP_NAMESPACE

typedef uint32_t HGF2DBasicLinearTypeId;

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
class HNOVTABLEINIT HGF2DBasicLinear : public HGF2DLinear
    {
    HDECLARE_CLASS_ID(HGF2DLinearId_Basic, HGF2DLinear)

public:

    // Primary methods
    HGF2DBasicLinear();
    HGF2DBasicLinear(const HGF2DPosition& pi_rStartPoint,
                    const HGF2DPosition& pi_rEndPoint);
    HGF2DBasicLinear(const HGF2DBasicLinear&    pi_rObject);
    virtual            ~HGF2DBasicLinear();

    HGF2DBasicLinear&  operator=(const HGF2DBasicLinear& pi_rObj);

    /** -----------------------------------------------------------------------------
        This pure virtual method returns an id uniquely identifying the basic linear
        type.
        -----------------------------------------------------------------------------
    */
    virtual HGF2DBasicLinearTypeId  GetBasicLinearType() const = 0;

    // From HGF2DLinear
    virtual bool                    IsABasicLinear() const;
    virtual bool                    IsComplex() const;

    // Debug method
    void                            PrintState(ostream& po_rOutput) const;

protected:

private:

    };

END_IMAGEPP_NAMESPACE
#include "HGF2DBasicLinear.hpp"
