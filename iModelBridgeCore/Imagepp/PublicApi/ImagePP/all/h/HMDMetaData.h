//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDMetaData.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

class HMDMetaData
    {
    HDECLARE_BASECLASS_ID (7004);

public :

    _HDLLu HMDMetaData();
    _HDLLu virtual ~HMDMetaData();

private :

    HMDMetaData(const HMDMetaData& pi_rObj);
    HMDMetaData& operator=(const HMDMetaData& pi_rObj);
    };

