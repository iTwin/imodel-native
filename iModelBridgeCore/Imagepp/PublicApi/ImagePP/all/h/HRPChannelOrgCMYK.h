//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrgCMYK.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// Class HRPChannelOrgCMYK
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

class HRPChannelOrgCMYK : public HRPChannelOrg
    {
public:
    HRPChannelOrgCMYK() : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::CYAN,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::MAGENTA,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::YELLOW,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::BLACK,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        };

    HRPChannelOrgCMYK(const HRPChannelOrgCMYK& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgCMYK() { };
    };
