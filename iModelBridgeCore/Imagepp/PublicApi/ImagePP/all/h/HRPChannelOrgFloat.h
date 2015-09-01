//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrgFloat.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgFloat
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgFloat : public HRPChannelOrg
    {
public:
    HRPChannelOrgFloat(unsigned short             pi_NbBits,
                       HRPChannelType::ChannelRole pi_ChannelRole,
                       const double*             pi_pNoDataValue = 0)
        : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(pi_ChannelRole,
                                  HRPChannelType::FLOAT_CH,
                                  pi_NbBits,
                                  0,
                                  pi_pNoDataValue));
        };

    HRPChannelOrgFloat(const HRPChannelOrgFloat& pi_rObj) : HRPChannelOrg(pi_rObj)
        {
        };

    virtual ~HRPChannelOrgFloat() { };
    };
END_IMAGEPP_NAMESPACE
