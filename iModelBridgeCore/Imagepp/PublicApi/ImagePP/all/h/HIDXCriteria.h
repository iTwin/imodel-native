//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXCriteria.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXCriteria
//-----------------------------------------------------------------------------
// Selection criteria
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HIDXCriteria
    {
    HDECLARE_BASECLASS_ID(HIDXCriteriaId_Base)

public:


    HIDXCriteria(){};


    virtual        ~HIDXCriteria(){};


private:

    // Copy ctor and assignment are disabled
    HIDXCriteria(const HIDXCriteria& pi_rObj);
    HIDXCriteria& operator=(const HIDXCriteria& pi_rObj);

    };

END_IMAGEPP_NAMESPACE



