//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrgBGR.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgBGR
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgBGR : public HRPChannelOrg
    {
public:
    HRPChannelOrgBGR(unsigned short pi_BitsBlue,
                     unsigned short pi_BitsGreen,
                     unsigned short pi_BitsRed,
                     unsigned short pi_BitsExtra,
                     HRPChannelType::ChannelRole pi_RoleChannel5,
                     HRPChannelType::DataType    pi_DataTypeChannel5,
                     unsigned short             pi_BitsChannel5,
                     bool                       pi_IsExtraBitsAlpha = true)
        : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::BLUE,
                                  HRPChannelType::INT_CH,
                                  pi_BitsBlue,0));
        AddChannel(HRPChannelType(HRPChannelType::GREEN,
                                  HRPChannelType::INT_CH,
                                  pi_BitsGreen,0));
        AddChannel(HRPChannelType(HRPChannelType::RED,
                                  HRPChannelType::INT_CH,
                                  pi_BitsRed,0));

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

    HRPChannelOrgBGR(const HRPChannelOrgBGR& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgBGR() { };
    };
END_IMAGEPP_NAMESPACE
