//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDMetaData.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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