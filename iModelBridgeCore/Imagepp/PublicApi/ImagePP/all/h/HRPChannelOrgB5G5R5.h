//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrgB5G5R5.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgB5G5R5
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgB5G5R5 : public HRPChannelOrg
    {
public:
    HRPChannelOrgB5G5R5() : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::BLUE,
                                  HRPChannelType::INT_CH,
                                  5, 0));
        AddChannel(HRPChannelType(HRPChannelType::GREEN,
                                  HRPChannelType::INT_CH,
                                  5, 0));
        AddChannel(HRPChannelType(HRPChannelType::RED,
                                  HRPChannelType::INT_CH,
                                  5, 0));
        AddChannel(HRPChannelType(HRPChannelType::UNUSED,
                                  HRPChannelType::VOID_CH,
                                  1, 0));
        };

    HRPChannelOrgB5G5R5(const HRPChannelOrgB5G5R5& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgB5G5R5() { };
    };
END_IMAGEPP_NAMESPACE
