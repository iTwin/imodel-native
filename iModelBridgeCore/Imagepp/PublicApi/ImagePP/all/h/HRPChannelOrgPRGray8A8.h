//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrgPRGray8A8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgPRGray8A8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgPRGray8A8 : public HRPChannelOrg
    {
public:
    HRPChannelOrgPRGray8A8() : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::PREMULTIPLIED_GRAY,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::ALPHA,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        };

    HRPChannelOrgPRGray8A8(const HRPChannelOrgPRGray8A8& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgPRGray8A8() { };
    };
END_IMAGEPP_NAMESPACE
