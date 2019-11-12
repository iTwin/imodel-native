//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HMDMetaData
    {
    HDECLARE_BASECLASS_ID (HMDMetaDataId_Base);

public :

    IMAGEPP_EXPORT HMDMetaData();
    IMAGEPP_EXPORT virtual ~HMDMetaData();

private :

    HMDMetaData(const HMDMetaData& pi_rObj);
    HMDMetaData& operator=(const HMDMetaData& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
