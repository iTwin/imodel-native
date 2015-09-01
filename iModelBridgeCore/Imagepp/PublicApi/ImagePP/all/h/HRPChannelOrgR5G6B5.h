//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrgR5G6B5.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgR5G6B5
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgR5G6B5 : public HRPChannelOrg
    {
public:
    HRPChannelOrgR5G6B5() : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::RED,
                                  HRPChannelType::INT_CH,
                                  5, 0));
        AddChannel(HRPChannelType(HRPChannelType::GREEN,
                                  HRPChannelType::INT_CH,
                                  6, 0));
        AddChannel(HRPChannelType(HRPChannelType::BLUE,
                                  HRPChannelType::INT_CH,
                                  5, 0));
        };

    HRPChannelOrgR5G6B5(const HRPChannelOrgR5G6B5& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgR5G6B5() { };
    };
END_IMAGEPP_NAMESPACE
