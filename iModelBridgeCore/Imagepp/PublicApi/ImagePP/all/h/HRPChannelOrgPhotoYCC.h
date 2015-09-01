//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrgPhotoYCC.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// Class HRPChannelOrgPhotoYCC
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPChannelOrgPhotoYCC : public HRPChannelOrg
    {
public:
    HRPChannelOrgPhotoYCC() : HRPChannelOrg()
        {
        AddChannel(HRPChannelType(HRPChannelType::Y,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::CHROMA1,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        AddChannel(HRPChannelType(HRPChannelType::CHROMA2,
                                  HRPChannelType::INT_CH,
                                  8, 0));
        };

    HRPChannelOrgPhotoYCC(const HRPChannelOrgPhotoYCC& pi_rObj) : HRPChannelOrg(pi_rObj) { };
    virtual ~HRPChannelOrgPhotoYCC() { };
    };
END_IMAGEPP_NAMESPACE
