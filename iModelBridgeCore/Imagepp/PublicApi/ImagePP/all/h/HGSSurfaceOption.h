//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceOption.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSSurfaceOption
//-----------------------------------------------------------------------------
// General class for surfaces option.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"


class HGSSurfaceOption : public HFCShareableObject<HGSSurfaceOption>
    {
    HDECLARE_BASECLASS_ID(1702)

public:

    HGSSurfaceOption(const HGSSurfaceOption& pi_rObj);
    virtual        ~HGSSurfaceOption();

    virtual HGSSurfaceOption*
    Clone() const = 0;
protected:
    HGSSurfaceOption();

private:


    // disabled methods
    HGSSurfaceOption&
    operator=(const HGSSurfaceOption&);
    };

typedef map<HCLASS_ID, HFCPtr<HGSSurfaceOption> > HGSSurfaceOptions;


#include "HGSSurfaceOption.hpp"

