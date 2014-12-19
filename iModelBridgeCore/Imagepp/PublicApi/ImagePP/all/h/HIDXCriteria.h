//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXCriteria.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXCriteria
//-----------------------------------------------------------------------------
// Selection criteria
//-----------------------------------------------------------------------------

#pragma once



class HIDXCriteria
    {
    HDECLARE_BASECLASS_ID(3300)

public:


    HIDXCriteria();


    virtual        ~HIDXCriteria();


private:

    // Copy ctor and assignment are disabled
    HIDXCriteria(const HIDXCriteria& pi_rObj);
    HIDXCriteria& operator=(const HIDXCriteria& pi_rObj);

    };


#include "HIDXCriteria.hpp"


