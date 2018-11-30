//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrgARGB.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgARGB
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgARGB : public HRPChannelOrg
    {
public:
    HRPChannelOrgARGB() : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::ALPHA,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::RED,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::GREEN,
                                  HRPChannelType::INT_CH,
                                  8, 0));

        AddChannel(HRPChannelType(HRPChannelType::BLUE,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        };

    HRPChannelOrgARGB(const HRPChannelOrgARGB& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgARGB() { };
    };
END_IMAGEPP_NAMESPACE
