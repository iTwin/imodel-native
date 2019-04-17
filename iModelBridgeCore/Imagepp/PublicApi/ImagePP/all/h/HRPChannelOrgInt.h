//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgInt
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgInt : public HRPChannelOrg
    {
public:
    HRPChannelOrgInt(uint16_t             pi_NbBits,
                     HRPChannelType::ChannelRole pi_ChannelRole,
                     const double*             pi_pNoDataValue = 0)
        : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(pi_ChannelRole,
                                  HRPChannelType::SINT_CH,
                                  pi_NbBits,
                                  0,
                                  pi_pNoDataValue));
        };

    HRPChannelOrgInt(const HRPChannelOrgInt& pi_rObj)
        : HRPChannelOrg(pi_rObj)
        {
        };

    virtual ~HRPChannelOrgInt() { };
    };
END_IMAGEPP_NAMESPACE
