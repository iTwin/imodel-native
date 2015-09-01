//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFColorSet.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFRGBSet
//---------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE

class HNOVTABLEINIT HGFColorSet : public HFCShareableObject<HGFColorSet>
    {
    HDECLARE_BASECLASS_ID(HGFColorSetId_Base)

public:

    IMAGEPP_EXPORT                 HGFColorSet();
    IMAGEPP_EXPORT virtual         ~HGFColorSet();

    virtual bool   IsIn(Byte pi_R, Byte pi_G, Byte pi_B) const = 0;

protected:

private:

    // Disabled

    HGFColorSet(const HGFColorSet&);
    HGFColorSet& operator=(const HGFColorSet&);
    };

END_IMAGEPP_NAMESPACE