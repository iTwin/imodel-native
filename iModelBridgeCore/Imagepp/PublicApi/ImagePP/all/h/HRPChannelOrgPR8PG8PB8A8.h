//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrgPR8PG8PB8A8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgPR8PG8PB8A8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgPR8PG8PB8A8 : public HRPChannelOrg
    {
public:
    HRPChannelOrgPR8PG8PB8A8() : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::PREMULTIPLIED_RED,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::PREMULTIPLIED_GREEN,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::PREMULTIPLIED_BLUE,
                                  HRPChannelType::INT_CH,
                                  8, 0));

        AddChannel(HRPChannelType(HRPChannelType::ALPHA,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        };

    HRPChannelOrgPR8PG8PB8A8(const HRPChannelOrgPR8PG8PB8A8& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgPR8PG8PB8A8() { };
    };
END_IMAGEPP_NAMESPACE
