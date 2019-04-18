//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgRGB
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgRGB : public HRPChannelOrg
    {
public:
    HRPChannelOrgRGB(uint16_t pi_BitsRed,
                     uint16_t pi_BitsGreen,
                     uint16_t pi_BitsBlue,
                     uint16_t pi_BitsExtra,
                     HRPChannelType::ChannelRole pi_RoleChannel5,
                     HRPChannelType::DataType    pi_DataTypeChannel5,
                     uint16_t             pi_BitsChannel5,
                     bool                       pi_IsExtraBitsAlpha = true)
        : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::RED,
                                  HRPChannelType::INT_CH,
                                  pi_BitsRed,0));
        AddChannel(HRPChannelType(HRPChannelType::GREEN,
                                  HRPChannelType::INT_CH,
                                  pi_BitsGreen,0));
        AddChannel(HRPChannelType(HRPChannelType::BLUE,
                                  HRPChannelType::INT_CH,
                                  pi_BitsBlue,0));

        if (pi_BitsExtra > 0)
            {
            if (pi_IsExtraBitsAlpha == true)
                {
                AddChannel(HRPChannelType(HRPChannelType::ALPHA,
                                          HRPChannelType::INT_CH,
                                          pi_BitsExtra,0));
                }
            else
                {
                AddChannel(HRPChannelType(HRPChannelType::USER,
                                          HRPChannelType::INT_CH,
                                          pi_BitsExtra,0));
                }
            }

        if(pi_BitsChannel5)
            AddChannel(HRPChannelType(pi_RoleChannel5,
                                      pi_DataTypeChannel5,
                                      pi_BitsChannel5,0));
        };

    HRPChannelOrgRGB(const HRPChannelOrgRGB& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgRGB() { };
    };
END_IMAGEPP_NAMESPACE
