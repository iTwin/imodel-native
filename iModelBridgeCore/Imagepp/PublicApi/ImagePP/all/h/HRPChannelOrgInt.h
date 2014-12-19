//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrgInt.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgInt
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

class HRPChannelOrgInt : public HRPChannelOrg
    {
public:
    HRPChannelOrgInt(unsigned short             pi_NbBits,
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