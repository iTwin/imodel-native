//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRPChannelOrgGrayWhite
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgGrayWhite : public HRPChannelOrg
    {
public:
    HRPChannelOrgGrayWhite(uint16_t pi_BitsGray,
                           HRPChannelType::ChannelRole pi_RoleChannel4,
                           HRPChannelType::DataType    pi_DataTypeChannel4,
                           uint16_t             pi_BitsChannel4)
        : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::GRAYWHITE,
                                  HRPChannelType::INT_CH,
                                  pi_BitsGray,0));

        if(pi_BitsChannel4)
            AddChannel(HRPChannelType(pi_RoleChannel4,
                                      pi_DataTypeChannel4,
                                      pi_BitsChannel4,0));
        };

    HRPChannelOrgGrayWhite(const HRPChannelOrgGrayWhite& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgGrayWhite() { };
    };
END_IMAGEPP_NAMESPACE
