//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFColorSet.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFRGBSet
//---------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"

class HNOVTABLEINIT HGFColorSet : public HFCShareableObject<HGFColorSet>
    {
    HDECLARE_BASECLASS_ID(1536)

public:

    _HDLLg                 HGFColorSet();
    _HDLLg virtual         ~HGFColorSet();

    virtual bool   IsIn(Byte pi_R, Byte pi_G, Byte pi_B) const = 0;

protected:

private:

    // Disabled

    HGFColorSet(const HGFColorSet&);
    HGFColorSet& operator=(const HGFColorSet&);
    };

