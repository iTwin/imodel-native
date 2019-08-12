//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgGray
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgGray : public HRPChannelOrg
    {
public:
    HRPChannelOrgGray(uint16_t pi_BitsGray,
                      HRPChannelType::ChannelRole pi_RoleChannel4,
                      HRPChannelType::DataType    pi_DataTypeChannel4,
                      uint16_t             pi_BitsChannel4)
        : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::GRAY,
                                  HRPChannelType::INT_CH,
                                  pi_BitsGray,0));

        if(pi_BitsChannel4)
            AddChannel(HRPChannelType(pi_RoleChannel4,
                                      pi_DataTypeChannel4,
                                      pi_BitsChannel4,0));
        };

    HRPChannelOrgGray(const HRPChannelOrgGray& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgGray() { };
    };
END_IMAGEPP_NAMESPACE
