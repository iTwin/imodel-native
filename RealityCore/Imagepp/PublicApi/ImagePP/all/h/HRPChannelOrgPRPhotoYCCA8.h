//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRPChannelOrgPRPhotoYCCA8
//-----------------------------------------------------------------------------
#pragma once
#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgPRPhotoYCCA8 : public HRPChannelOrg
    {
public:
    HRPChannelOrgPRPhotoYCCA8() : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::PREMULTIPLIED_Y,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::PREMULTIPLIED_CHROMA1,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::PREMULTIPLIED_CHROMA2,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::ALPHA,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        };

    HRPChannelOrgPRPhotoYCCA8(const HRPChannelOrgPRPhotoYCCA8& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgPRPhotoYCCA8() { };
    };
END_IMAGEPP_NAMESPACE
